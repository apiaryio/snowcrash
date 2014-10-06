//
//  RegexMatch.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_REGEXMATCH_H
#define SNOWCRASH_REGEXMATCH_H

#include <string>
#include <vector>

namespace snowcrash {

    // Perform snowcrash-specific regex evaluation
    // returns true if target string matches given expression, false otherwise
    bool RegexMatch(const std::string& target, const std::string& expression);

    // Performs posix-regex and returns first captured group (excluding whole target)
    std::string RegexCaptureFirst(const std::string& target, const std::string& expression);

    // Array of capture groups
    typedef std::vector<std::string> CaptureGroups;

    // Performs posix-regex
    // returns true if target string matches given expression, false otherwise
    bool RegexCapture(const std::string& target, const std::string& expression, CaptureGroups& captureGroups, size_t groupSize = 8);
}

#endif
