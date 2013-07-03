//
//  RegexMatch.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 7/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <regex>
#include <cstring>
#include "RegexMatch.h"

//
// A C++11 implementation
//

bool snowcrash::RegexMatch(const std::string& target, const std::string& expression)
{
    if (target.empty() || expression.empty())
        return false;
    
    try {
        std::regex pattern(expression, std::regex_constants::extended);
        return std::regex_search(target, pattern);
    }
    catch (const std::regex_error&) {
    }
    catch (...) {
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
        
        std::regex pattern(expression, std::regex_constants::extended);
        std::match_results<std::string::const_iterator> result;
        if (!std::regex_search(target, result, pattern))
            return false;
    
        for (std::match_results<std::string::const_iterator>::const_iterator it = result.begin();
             it != result.end();
             ++it) {

            captureGroups.push_back(*it);
        }

        return true;
    }
    catch (const std::regex_error&) {
    }
    catch (...) {
    }
    
    return false;
}
