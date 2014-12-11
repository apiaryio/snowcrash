//
//  TrimString.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/11/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//
//  Credits:
//  http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

#ifndef SNOWCRAH_STRINGUTILITY_H
#define SNOWCRAH_STRINGUTILITY_H

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <string>
#include <sstream>
#include <vector>
#include "RegexMatch.h"
#include "MarkdownParser.h"

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
    inline bool MatchContainers(const T1& arg1, const T2& arg2, const Predicate& predicate) {
        if (arg1.length() != arg2.length()) {
            return false;
        }
        return std::equal(arg1.begin(), arg1.end(), arg2.begin(), predicate);
    }

    template <typename T>
    struct Equal : std::binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const {
            return MatchContainers(left, right, IsEqual());
        }
    };

    template <typename T>
    struct IEqual : std::binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const {
            return MatchContainers(left, right, IsIEqual());
        }
    };

    /**
     * \brief Retrieve the string enclosed by the given matching escaping characters
     *
     *        Please note that the subject will be stripped of the escaped string
     *        and the characters before it
     *
     * \param subject String that needs to be parsed
     * \param begin Character representing the beginning of the escaped string
     * \param stripEscapeChars If true, strip the escape characters from the result string
     *
     * \return Returns the escaped string
     *
     * \example (begin = 1, subject = "a```b```cd") ----> (return = "```b```", subject = "cd")
     */
    inline std::string RetrieveEscaped(std::string& subject,
                                       size_t begin = 0,
                                       const bool stripEscapeChars = false) {

        size_t levels = 0;
        const char escapeChar = subject[begin];

        // Get the level of the backticks
        while (subject[levels + begin] == escapeChar) {
            levels++;
        }

        std::string borderChars = subject.substr(begin, levels);
        size_t end = subject.substr(levels + begin).find(borderChars);

        if (end == std::string::npos) {
            return "";
        }

        if (stripEscapeChars) {
            begin = begin + levels;
            end = end + begin;
        } else {
            end = end + (2 * levels) + begin;
        }

        std::string escapedString = subject.substr(begin, end - begin);
        subject = subject.substr(end);

        return escapedString;
    }

    /**
     * \brief Strip the enclosing backticks and return the string in the middle.
     *
     *        If there are no matching enclosing bacticks, return the whole string.
     *
     * \param subject String that needs to be stripped of enclosing backticks
     *
     * \return Substring that has been stripped of enclosing backticks
     */
    inline std::string StripBackticks(std::string& subject) {

        // Check if first and last chars are backticks
        if (subject[0] != '`' ||
            subject[subject.length() - 1] != '`') {

            return subject;
        }

        std::string escapedString = RetrieveEscaped(subject, 0, true);

        if (escapedString.empty()) {
            return subject;
        }

        TrimString(escapedString);

        return escapedString;
    }

    /**
     * \brief If the given string is a markdown link, return the string which is being linked
     *
     *        If there is no link, return the whole string
     *
     * \param subject String which is a markdown link
     *
     * \return Substring which is inside the first [] of the markdown link
     */
    inline std::string StripMarkdownLink(const std::string& subject) {

        // Check if markdown link
        if (subject[0] != mdp::MarkdownBeginReference) {
            return subject;
        }

        std::string linkedString = RegexCaptureFirst(subject, mdp::MarkdownLinkRegex);
        TrimString(linkedString);

        return linkedString;
    }
}

#endif
