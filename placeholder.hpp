#include <iostream>
#include <map>

using namespace std;

string replace_placeholders(string template_string,
                     map<string, string> *lookups)
{
    map<string, string>::iterator itr;
    for (itr = (*lookups).begin(); itr != (*lookups).end(); ++itr)
    {
        string::size_type pos = 0;
        string placeholder = "{" + itr->first + "}";
        while ((pos = template_string.find(placeholder, pos)) != string::npos)
        {
            template_string.replace(pos, placeholder.size(), itr->second);
            pos++;
        }
    }
    return template_string;
}