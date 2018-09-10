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

#ifndef __LOGSHIP_LOGGER_HPP__
#define __LOGSHIP_LOGGER_HPP__

#include <stdio.h>

#include <mesos/slave/container_logger.hpp>
#include <mesos/slave/containerizer.hpp>

#include <stout/bytes.hpp>
#include <stout/flags.hpp>
#include <stout/option.hpp>

#include <stout/os/constants.hpp>
#include <stout/os/exists.hpp>
#include <stout/os/pagesize.hpp>
#include <stout/os/shell.hpp>

#include "lib_logship.hpp"

#ifndef MODULE_NAME
#define MODULE_NAME org_apache_mesos_LogShipContainerLogger
#endif

namespace mesos
{
namespace internal
{
namespace logger
{

// Forward declaration.
class LogShipContainerLoggerProcess;

// These flags are loaded twice: once when the `ContainerLogger` module
// is created and each time before launching executors. The flags loaded
// at module creation act as global default values, whereas flags loaded
// prior to executors can override the global values.
struct OverridableModuleFlags : public virtual flags::FlagsBase
{
    OverridableModuleFlags()
    {
        add(&OverridableModuleFlags::max_stdout_size,
            "max_stdout_size",
            "Maximum size, in bytes, of a single stdout log file.\n"
            "Defaults to 10 MB.  Must be at least 1 (memory) page.",
            Megabytes(10),
            &OverridableModuleFlags::validateSize);

        add(&OverridableModuleFlags::logrotate_stdout_options,
            "logrotate_stdout_options",
            "Additional config options to pass into 'logrotate' for stdout.\n"
            "This string will be inserted into a 'logrotate' configuration file.\n"
            "i.e.\n"
            "  /path/to/stdout {\n"
            "    <logrotate_stdout_options>\n"
            "    size <max_stdout_size>\n"
            "  }\n"
            "NOTE: The 'size' option will be overridden by this module.");

        add(&OverridableModuleFlags::max_stderr_size,
            "max_stderr_size",
            "Maximum size, in bytes, of a single stderr log file.\n"
            "Defaults to 10 MB.  Must be at least 1 (memory) page.",
            Megabytes(10),
            &OverridableModuleFlags::validateSize);

        add(&OverridableModuleFlags::logrotate_stderr_options,
            "logrotate_stderr_options",
            "Additional config options to pass into 'logrotate' for stderr.\n"
            "This string will be inserted into a 'logrotate' configuration file.\n"
            "i.e.\n"
            "  /path/to/stderr {\n"
            "    <logrotate_stderr_options>\n"
            "    size <max_stderr_size>\n"
            "  }\n"
            "NOTE: The 'size' option will be overridden by this module.");
    }

    static Option<Error> validateSize(const Bytes &value)
    {
        if (value.bytes() < os::pagesize())
        {
            return Error(
                "Expected --max_stdout_size and --max_stderr_size of "
                "at least " +
                stringify(os::pagesize()) + " bytes");
        }

        return None();
    }

    Bytes max_stdout_size;
    Option<std::string> logrotate_stdout_options;

    Bytes max_stderr_size;
    Option<std::string> logrotate_stderr_options;
};

struct ModuleFlags : public virtual OverridableModuleFlags
{
    ModuleFlags()
    {
        add(&ModuleFlags::environment_variable_prefix,
            "environment_variable_prefix",
            "Prefix for environment variables meant to modify the behavior of\n"
            "the logrotate logger for the specific executor being launched.\n"
            "The logger will look for four prefixed environment variables in the\n"
            "'ExecutorInfo's 'CommandInfo's 'Environment':\n"
            "  * MAX_STDOUT_SIZE\n"
            "  * LOGROTATE_STDOUT_OPTIONS\n"
            "  * MAX_STDERR_SIZE\n"
            "  * LOGROTATE_STDERR_OPTIONS\n"
            "If present, these variables will overwrite the global values set\n"
            "via module parameters.",
            "CONTAINER_LOGGER_");

        add(&ModuleFlags::logrotate_path,
            "logrotate_path",
            "If specified, the logrotate container logger will use the specified\n"
            "'logrotate' instead of the system's 'logrotate'.",
            "logrotate",
            [](const std::string &value) -> Option<Error> {
                // Check if `logrotate` exists via the help command.
                // TODO(josephw): Consider a more comprehensive check.
                Try<std::string> helpCommand =
                    os::shell(value + " --help > " + os::DEV_NULL);

                if (helpCommand.isError())
                {
                    return Error(
                        "Failed to check logrotate: " + helpCommand.error());
                }

                return None();
            });

        add(&ModuleFlags::libprocess_num_worker_threads,
            "libprocess_num_worker_threads",
            "Number of Libprocess worker threads.\n"
            "Defaults to 8.  Must be at least 1.",
            8u,
            [](const size_t &value) -> Option<Error> {
                if (value < 1u)
                {
                    return Error(
                        "Expected --libprocess_num_worker_threads of at least 1");
                }

                return None();
            });

        // custom flags
        add(&ModuleFlags::companion_binary_path,
            "companion_binary_path",
            "path of companion binary.",
            path::join("/usr/local/bin/", mesos::internal::logger::companion::EXEC_NAME).c_str(),
            [](const std::string &executablePath) -> Option<Error> {
                if (!os::exists(executablePath))
                {
                    return Error("Cannot find: " + executablePath);
                }
                return None();
            });

        add(&ModuleFlags::log_ship_base_directory,
            "log_ship_base_directory",
            "The directory to which logs are shipped after max_size is reached");

        add(&ModuleFlags::time_stamp_format,
            "time_stamp_format",
            "Timestamp foramtter pattern used.Default value: %d-%b-%Y_%H.%M.%S",
            "%d-%b-%Y_%H.%M.%S");

        add(&ModuleFlags::log_path_template,
            "log_path_template",
            "TODO help");

        add(&ModuleFlags::exclude_executorid_regex,
            "exclude_executorid_regex",
            "TODO help");
    }

    std::string environment_variable_prefix;
    std::string logrotate_path;

    // custom flags
    std::string companion_binary_path;
    std::string log_path_template;
    std::string time_stamp_format;
    std::string log_ship_base_directory;
    std::string exclude_executorid_regex;

    size_t libprocess_num_worker_threads;
};

// The `LogShipContainerLogger` is a container logger that utilizes the
// `logrotate` utility to strictly constrain total size of a container's
// stdout and stderr log files.  All `logrotate` configuration options
// (besides `size`, which this module uses) are supported.  See `ModuleFlags` above.
class LogShipContainerLogger : public mesos::slave::ContainerLogger
{
  public:
    LogShipContainerLogger(const ModuleFlags &_flags);

    virtual ~LogShipContainerLogger();

    // This is a noop.  The logrotate container logger has nothing to initialize.
    virtual Try<Nothing> initialize();

    virtual process::Future<mesos::slave::ContainerIO> prepare(
        const ExecutorInfo &executorInfo,
        const std::string &sandboxDirectory,
        const Option<std::string> &user);

  protected:
    ModuleFlags flags;
    process::Owned<LogShipContainerLoggerProcess> process;
};

} // namespace logger
} // namespace internal
} // namespace mesos

#endif // __LOGSHIP_LOGGER_HPP__
