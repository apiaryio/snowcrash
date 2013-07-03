//
//  RegexMatch.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <regex.h>
#include <cstring>
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
    CaptureGroups groups;
    if (!RegexCapture(target, expression, groups) ||
        groups.size() < 2)
        return std::string();
    
    return groups[1];
}

bool snowcrash::RegexCapture(const std::string& target, const std::string& expression, CaptureGroups& captureGroups, size_t groupSize)
{
    if (target.empty() || expression.empty())
        return false;
    
    captureGroups.clear();
    
    try {
        regex_t regex;
        int reti = ::regcomp(&regex, expression.c_str(), REG_EXTENDED);
        if (reti)
            return false;
        
        regmatch_t *pmatch = ::new regmatch_t[groupSize];
        ::memset(pmatch, 0, sizeof(regmatch_t) * groupSize);
        
        reti = ::regexec(&regex, target.c_str(), groupSize, pmatch, 0);
        if (!reti) {
            ::regfree(&regex);
            
            for (size_t i = 0; i < groupSize; ++i) {
                if (pmatch[i].rm_so == -1 || pmatch[i].rm_eo == -1)
                    captureGroups.push_back(std::string());
                else
                    captureGroups.push_back(std::string(target, pmatch[i].rm_so, pmatch[i].rm_eo - pmatch[i].rm_so));
            }
            
            delete [] pmatch;
            return true;
        }
        else {
            ::regfree(&regex);
            delete [] pmatch;        
            return false;
        }
    }
    catch (...) {
    }
    
    return false;    
}


