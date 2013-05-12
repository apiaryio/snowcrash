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
    int reti = ::regcomp(&regex, expression.c_str(), REG_EXTENDED | REG_NOSUB);
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

std::string snowcrash::RegexCaptureFirst(const std::string& target, const std::string& expression)
{
    if (target.empty() || expression.empty())
        return false;
    
    regex_t regex;
    int reti = ::regcomp(&regex, expression.c_str(), REG_EXTENDED);
    if (reti)
        return std::string();
    
    regmatch_t pmatch[2];
    ::memset(pmatch, 0, sizeof(pmatch));
    reti = ::regexec(&regex, target.c_str(), 2, pmatch, 0);
    if (!reti) {
        ::regfree(&regex);
        
        if (pmatch[1].rm_so == -1 || pmatch[1].rm_eo == -1)
            return std::string(); // no group
        
        return std::string(target, pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
    }
    else {
        ::regfree(&regex);
        return std::string();
    }
    
    return false;
}