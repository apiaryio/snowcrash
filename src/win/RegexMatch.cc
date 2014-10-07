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

using namespace std;

#if _MSC_VER == 1500
using namespace std::tr1;
#endif
//
// A C++09 implementation
//

bool snowcrash::RegexMatch(const string& target, const string& expression)
{
    if (target.empty() || expression.empty())
        return false;

    try {
        regex pattern(expression, regex_constants::extended);
        return regex_search(target, pattern);
    }
    catch (const regex_error&) {
    }
    catch (...) {
    }

    return false;
}

string snowcrash::RegexCaptureFirst(const string& target, const string& expression)
{
    CaptureGroups groups;
    if (!RegexCapture(target, expression, groups) ||
        groups.size() < 2)
        return string();

    return groups[1];
}

bool snowcrash::RegexCapture(const string& target, const string& expression, CaptureGroups& captureGroups, size_t groupSize)
{
    if (target.empty() || expression.empty())
        return false;

    captureGroups.clear();

    try {

        regex pattern(expression, regex_constants::extended);
        match_results<string::const_iterator> result;
        if (!regex_search(target, result, pattern))
            return false;

        for (match_results<string::const_iterator>::const_iterator it = result.begin();
             it != result.end();
             ++it) {

            captureGroups.push_back(*it);
        }

        return true;
    }
    catch (const regex_error&) {
    }
    catch (...) {
    }

    return false;
}
