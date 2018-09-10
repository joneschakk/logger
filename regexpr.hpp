#include <iostream>
#include <cstdlib>

#include "3rdparty/slre/slre.h"
#include "3rdparty/slre/slre.c"

using namespace std;

bool regex_matches(string word, string rx)
{
    return slre_match(
               rx.c_str(),
               word.c_str(),
               word.length(),
               NULL,
               0,
               0) != SLRE_NO_MATCH;
}