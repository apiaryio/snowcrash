//
//  TrimString.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/11/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
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
#include <sstream>
#include <vector>

namespace snowcrash {

    // Check a character not to be an space of any kind
    inline bool isSpace(const std::string::value_type i){
        if(i == ' ' || i == '\t' || i == '\n' || i == '\v' || i == '\f' || i == '\r')
            return true;
        return false;
    }

    // Trim string from start
    inline std::string& TrimStringStart(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun(isSpace))));
        return s;
    }

    // Trim string from end
    inline std::string& TrimStringEnd(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun(isSpace))).base(), s.end());
        return s;
    }

    // Trim both ends of string
    inline std::string& TrimString(std::string &s) {
        return TrimStringStart(TrimStringEnd(s));
    }

    // Split string by delim
    inline std::vector<std::string>& Split(const std::string& s, char delim, std::vector<std::string>& elems) {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }

    // Split string by delim
    inline std::vector<std::string> Split(const std::string& s, char delim) {
        std::vector<std::string> elems;
        Split(s, delim, elems);
        return elems;
    }

    // Split string on the first occurrence of delim
    inline std::vector<std::string> SplitOnFirst(const std::string& s, char delim) {
        std::string::size_type pos = s.find(delim);
        std::vector<std::string> elems;
        if (pos == std::string::npos) {
            elems.push_back(s);
        }
        else {
            elems.push_back(s.substr(0, pos));
            elems.push_back(s.substr(pos + 1, std::string::npos));
        }
        return elems;
    }

    // Make sure last two characters are newlines
    inline std::string& TwoNewLines(std::string& s) {

        if (s[s.length() - 1] != '\n') {
            s += "\n";
        }

        if (s[s.length() - 2] != '\n') {
            s += "\n";
        }

        return s;
    }

    /**
     *  \brief  Replace all occurrences of a string.
     *  \param  s       A string to search in.
     *  \param  find    A string to look for.
     *  \param  replace A string to replace with.
     *  \return A copy of %s with all occurrences of %find replaced by %replace.
     */
    inline std::string ReplaceString(const std::string& s,
                                     const std::string& find,
                                     const std::string& replace) {
        size_t pos = 0;
        std::string target(s);
        while ((pos = target.find(find, pos)) != std::string::npos) {
            target.replace(pos, find.length(), replace);
            pos += replace.length();
        }
        return target;
    }

    /**
     *  \brief  Extract the first line from a string.
     *
     *  \param  s   Subject of the extraction
     *  \param  r   Remaining content aftert the extraction
     *  \return First line from the subject string
     */
    inline std::string GetFirstLine(const std::string& s, std::string& r){
        std::vector<std::string> elem = SplitOnFirst(s, '\n');
        if (elem.empty())
            return std::string();
        if (elem.size() > 1)
            r = elem.back();
        return elem.front();
    }


    /**
     *  \brief  compare equality  - allow compare diferent types
     *
     *  \return true if args era equal
     */
    struct IsEqual {
        template<typename T1, typename T2>
            bool operator()(const T1& a1, const T2& a2) const {
                return a1 == a2;
            }
    };

    /**
     *  \brief  compare equality - character are compare case insensitive
     *
     *  \return true if args era equal
     */
    struct IsIEqual {
        template<typename T1, typename T2>
            bool operator()(const T1& a1, const T2& a2) const {
                return std::tolower(a1) == std::tolower(a2);
            }
    };

    /**
     *  \brief  compare containers equality 
     *
     *  \requirements 
     *    - both containers must support methods ::length(), ::begin(), ::end()
     *    - both containers must be iterable
     *    - both containers must contain comparable types
     *
     *  \param arg1, arg2 - containers to compare
     *  \param predicate - testing equality
     *
     *  \return true if containers contains same content
     */

    template <typename T1, typename T2, typename Predicate>
    inline bool ContainersMatch(const T1& arg1, const T2& arg2, const Predicate& predicate) {
        if (arg1.length() != arg2.length()) {
            return false;
        }
        return std::equal(arg1.begin(), arg1.end(), arg2.begin(), predicate);
    }
    
    template <class T>
    struct Equal : std::binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const {
            return ContainersMatch(left,right,IsEqual());
        }
    };

    template <class T>
    struct IEqual : std::binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const {
            return ContainersMatch(left,right,IsIEqual());
        }
    };

}

#endif
