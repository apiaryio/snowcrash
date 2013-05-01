//
//  RegexMatch.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/2/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <regex.h>
#include "RegexMatch.h"

// FIXME: Migrate to C++11.
// Naive implementation of regex matching using POSIX regex
bool snowcrash::RegexMatch(const std::string& target, const std::string& expression)
{
    if (target.empty() || expression.empty())
        return false;

    regex_t regex;
    int reti = ::regcomp(&regex, expression.c_str(), REG_EXTENDED);
    if (reti) {
        // Unable to compile regex
        return false;
    }
    
    // Execute regular expression
    reti = ::regexec(&regex, target.c_str(), 0, NULL, 0);
    if (!reti) {
        ::regfree(&regex);
        return true;
    }
    else if (reti == REG_NOMATCH) {
        ::regfree(&regex);
        return false;
    }
    else {
        char msgbuf[1024];
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        ::regfree(&regex);
        return false;
    }
    
    return false;
}
