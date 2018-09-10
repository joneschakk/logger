// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <unistd.h>

#include <new>

#include <functional>
#include <string>

#include <process/defer.hpp>
#include <process/dispatch.hpp>
#include <process/id.hpp>
#include <process/io.hpp>
#include <process/process.hpp>

#include <stout/bytes.hpp>
#include <stout/error.hpp>
#include <stout/exit.hpp>
#include <stout/nothing.hpp>
#include <stout/path.hpp>
#include <stout/stringify.hpp>
#include <stout/try.hpp>

#include <stout/os/pagesize.hpp>
#include <stout/os/shell.hpp>
#include <stout/os/su.hpp>
#include <stout/os/write.hpp>
#include <stout/os/ls.hpp>
#include <stout/path.hpp>

#include "companion.hpp"

using namespace process;
using namespace mesos::internal::logger::companion;

class LogrotateLoggerProcess : public Process<LogrotateLoggerProcess>
{
  public:
    LogrotateLoggerProcess(const CmdFlags &_flags)
        : ProcessBase(process::ID::generate("logrotate-logger")),
          flags(_flags),
          leading(None()),
          bytesWritten(0),
          fileCount(0)
    {
        // Prepare a buffer for reading from the `incoming` pipe.
        length = os::pagesize();
        buffer = new char[length];

        Path _path = Path(flags.log_filename.get());
        logfileBaseName = _path.basename();
        logFIleDirName = _path.dirname();
        logrotateConfFileName = path::join(logFIleDirName,
                                           "." + logfileBaseName + LOGROTATE_CONF_FILE_SUFFIX);
        logrorateStateFileName = path::join(logFIleDirName,
                                            "." + logfileBaseName + LOGROTATE_STATE_FILE_SUFFIX);
    }

    virtual ~LogrotateLoggerProcess()
    {
        if (buffer != nullptr)
        {
            delete[] buffer;
            buffer = nullptr;
        }

        if (leading.isSome())
        {
            os::close(leading.get());
        }
    }

    // Prepares and starts the loop which reads from stdin, writes to the
    // leading log file, and manages total log size.
    Future<Nothing> run()
    {
        // Populate the `logrotate` configuration file.
        // See `CmdFlags::logrotate_options` for the format.
        //
        // NOTE: We specify a size of `--max_size - length` because `logrotate`
        // has slightly different size semantics.  `logrotate` will rotate when the
        // max size is *exceeded*.  We rotate to keep files *under* the max size.
        const std::string config =
            "\"" + flags.log_filename.get() + "\" {\n" +
            flags.logrotate_options.getOrElse("") + "\n" +
            "size " + stringify(flags.max_size.bytes() - length) + "\n" +
            "}";

        Try<Nothing> result = os::write(logrotateConfFileName, config);

        if (result.isError())
        {
            return Failure("Failed to write configuration file: " + result.error());
        }

        // NOTE: This is a prerequisuite for `io::read`.
        Try<Nothing> nonblock = os::nonblock(STDIN_FILENO);
        if (nonblock.isError())
        {
            return Failure("Failed to set nonblocking pipe: " + nonblock.error());
        }

        // NOTE: This does not block.
        loop();

        return promise.future();
    }

    // Reads from stdin and writes to the leading log file.
    void loop()
    {
        io::read(STDIN_FILENO, buffer, length)
            .then(defer(self(), [&](size_t readSize) -> Future<Nothing> {
                // Check if EOF has been reached on the input stream.
                // This indicates that the container (whose logs are being
                // piped to this process) has exited.
                if (readSize <= 0)
                {
                    LOG(INFO) << " *** [:companion] EOF on executor logstream."
                                 "ship log (copy=true) and exit companion process";
                    rotate(true);
                    promise.set(Nothing());
                    return Nothing();
                }

                // Do log rotation (if necessary) and write the bytes to the
                // leading log file.
                Try<Nothing> result = write(readSize);
                if (result.isError())
                {
                    promise.fail("Failed to write: " + result.error());
                    return Nothing();
                }

                // Use `dispatch` to limit the size of the call stack.
                dispatch(self(), &LogrotateLoggerProcess::loop);

                return Nothing();
            }));
    }

    // Writes the buffer from stdin to the leading log file.
    // When the number of written bytes exceeds `--max_size`, the leading
    // log file is rotated.  When the number of log files exceed `--max_files`,
    // the oldest log file is deleted.
    Try<Nothing> write(size_t readSize)
    {
        // Rotate the log file if it will grow beyond the `--max_size`.
        if (bytesWritten + readSize > flags.max_size.bytes())
        {
            rotate();
        }

        // If the leading log file is not open, open it.
        // NOTE: We open the file in append-mode as `logrotate` may sometimes fail.
        if (leading.isNone())
        {
            Try<int> open = os::open(
                flags.log_filename.get(),
                O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

            if (open.isError())
            {
                return Error(
                    "Failed to open '" + flags.log_filename.get() +
                    "': " + open.error());
            }

            leading = open.get();
        }

        // Write from stdin to `leading`.
        // NOTE: We do not exit on error here since we are prioritizing
        // clearing the STDIN pipe (which would otherwise potentially block
        // the container on write) over log fidelity.
        Try<Nothing> result =
            os::write(leading.get(), std::string(buffer, readSize));

        if (result.isError())
        {
            std::cerr << "Failed to write: " << result.error() << std::endl;
        }

        bytesWritten += readSize;

        return Nothing();
    }

    // Calls `logrotate` on the leading log file and resets the `bytesWritten`.
    void rotate(bool copy=false)
    {
        if (leading.isSome())
        {
            os::close(leading.get());
            leading = None();
        }

        // Call `logrotate` to move around the files.
        // NOTE: If `logrotate` fails for whatever reason, we will ignore
        // the error and continue logging.  In case the leading log file
        // is not renamed, we will continue appending to the existing
        // leading log file.
        os::shell(
            flags.logrotate_path +
            " --state \"" + logrorateStateFileName + "\" \"" +
            logrotateConfFileName + "\"");

        if (!flags.target_path.empty())
            ship(copy);
        // Reset the number of bytes written.
        bytesWritten = 0;
    }

    void ship(bool copy = false)
    {
        Try<std::list<std::string>> entries = os::ls(logFIleDirName);
        if (entries.isError())
        {
            std::cout << ("Failed to list files in dir: " + entries.error()) << std::endl;
        }

        entries.get().sort();
        std::list<std::string> filenames = entries.get();

        foreach (const std::string &filename, filenames)
        {
            if (filename.substr(0, logfileBaseName.length()) == logfileBaseName)
            {
                std::string file_ext = Path(filename).extension().getOrElse("");
		if(file_ext==".1")   //to avoid ambigous naming, if compress not enabled
		       file_ext="";
                
		std::string src_path = path::join(logFIleDirName, filename);
                std::string target_path = flags.target_path + "." + stringify(++fileCount) + file_ext;
                LOG(INFO) <<file_ext<< "*** [:companion] moving " <<src_path << " to " << target_path;
                if (copy)
                    {
			os::shell("cp " + src_path + " " + target_path);
			LOG(INFO) <<"********cp";
		   }		
                else
                    os::shell("mv " + src_path + " " + target_path);
            }
        }
    }

  private:
    CmdFlags flags;

    // For reading from stdin.
    char *buffer;
    size_t length;

    // For writing and rotating the leading log file.
    Option<int> leading;
    size_t bytesWritten;

    // Used to capture when log rotation has completed because the
    // underlying process/input has terminated.
    Promise<Nothing> promise;

    int fileCount;
    std::string logfileBaseName;
    std::string logFIleDirName;
    std::string logrotateConfFileName;
    std::string logrorateStateFileName;
};

int main(int argc, char **argv)
{
    CmdFlags flags;

    // Load and validate flags from the environment and command line.
    Try<flags::Warnings> load = flags.load(None(), &argc, &argv);
    //std::cerr<<flags.target_path<<"\n\n\n\n";

    if (load.isError())
    {
        EXIT(EXIT_FAILURE) << flags.usage(load.error());
    }

    // Log any flag warnings.
    foreach (const flags::Warning &warning, load->warnings)
    {
        LOG(WARNING) << warning.message;
    }

    // Make sure this process is running in its own session.
    // This ensures that, if the parent process (presumably the Mesos agent)
    // terminates, this logger process will continue to run.
    if (::setsid() == -1)
    {
        EXIT(EXIT_FAILURE)
            << ErrnoError("Failed to put child in a new session").message;
    }

    // If the `--user` flag is set, change the UID of this process to that user.
    if (flags.user.isSome())
    {
        Try<Nothing> result = os::su(flags.user.get());

        if (result.isError())
        {
            EXIT(EXIT_FAILURE)
                << ErrnoError("Failed to switch user for logrotate process").message;
        }
    }

    // Asynchronously control the flow and size of logs.
    LogrotateLoggerProcess process(flags);
    spawn(&process);

    // Wait for the logging process to finish.
    Future<Nothing> status = dispatch(process, &LogrotateLoggerProcess::run);
    status.await();

    terminate(process);
    wait(process);

    return status.isReady() ? EXIT_SUCCESS : EXIT_FAILURE;
}
