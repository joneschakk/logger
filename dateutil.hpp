#include <ctime>
#include <string>

using namespace std;

string strftime_now(string format)
{
    char formatted_time[256];
    time_t currentTime = time(0);
    struct tm *temp_time = localtime(&currentTime);
    strftime(formatted_time, 256, format.c_str(), temp_time);
    return string(formatted_time);
}
