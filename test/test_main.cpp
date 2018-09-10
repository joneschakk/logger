
#include "../placeholder.hpp"
#include "../dateutil.hpp"
#include "../regexpr.hpp"
#include <string>
#include <stout/path.hpp>
#include <stout/os/constants.hpp>
#include <stout/os/ls.hpp>

using namespace std;

void _main();

int main()
{
    _main();
    return 0;
}

void _main()
{

#ifdef NO_SKIP_COMPILE

    // test regex
    string rx = "^hell.*";
    string word = "helloworld";
    cout << slre_match("b.d", "abcd", 4, NULL, 0, 0) << endl;
    cout << SLRE_NO_MATCH << endl;
    bool an = regex_matches(word, rx);
    std::cout << "Hello, Wandbox! " << an << std::endl;

    // test strftime_now
    cout << "now " << strftime_now("%d-%b-%Y_%H.%M.%S");

    // test replace_placeholders
    map<string, string> lookups;
    string templ = "foo:{foo}/fruit:{fruit}/num:{num}/";
    lookups["foo"] = "bar";
    lookups["fruit"] = "mango";
    lookups["num"] = "12345";
    cout << "rendered: " << replace_placeholders(templ, &lookups) << endl;

    string logDirName = "/tmp";
    string logfileBaseName = "stderr";
    int file_count = 0;

    Try<std::list<std::string>> entries = os::ls(logDirName);
    if (entries.isError())
    {
        cout << ("Failed to list files in dir: " + entries.error()) << endl;
    }

    entries.get().sort();
    std::list<string> filenames = entries.get();

    foreach (const std::string &filename, filenames)
    {
        if (filename.substr(0, logfileBaseName.length()) == logfileBaseName)
        {
            string extension = Path(filename).extension().getOrElse("");

            cout << filename << " >> " << logfileBaseName + "." + stringify(++file_count) + extension << endl;
        }
    }
#endif
}