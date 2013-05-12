//
//  TrimString.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/11/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//
//  Credits:
//  http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

#ifndef SNOWCRAH_TRIMSTRING_H
#define SNOWCRAH_TRIMSTRING_H

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <string>

namespace snowcrash {

    // Trim string from start
    static inline std::string& TrimStringStart(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
    }
    
    // Trim string from end
    static inline std::string& TrimStringEnd(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
    }
    
    // Trim both ends of string
    static inline std::string& TrimString(std::string &s) {
        return TrimStringStart(TrimStringEnd(s));
    }
    
    // Retrieve first line of given string
    static inline std::string GetFirstLine(const std::string& source) {
        std::string::size_type pos = source.find("\n");
        if (pos == std::string::npos)
            return source;
        else
            return source.substr(0, pos);
    }
}

#endif
