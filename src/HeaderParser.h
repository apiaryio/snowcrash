//
//  HeaderParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/22/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_HEADERPARSER_H
#define SNOWCRASH_HEADERPARSER_H

#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "ListUtility.h"
#include "RegexMatch.h"
#include "StringUtility.h"

// Headers matching regex
static const std::string HeadersRegex("^[Hh]eaders?[[:space:]]*$");

namespace snowcrash {

    // Internal type alias for Collection of Headers
    typedef Collection<Header>::type HeaderCollection;
    
    // Query header signature a of given block
    inline bool HasHeaderSignature(const BlockIterator& begin,
                                   const BlockIterator& end) {
        
        if (begin->type == ListBlockBeginType || begin->type == ListItemBlockBeginType) {
            
            BlockIterator cur = ListItemNameBlock(begin, end);
            if (cur == end)
                return false;
            
            if (cur->type != ParagraphBlockType &&
                cur->type != ListItemBlockEndType)
                return false;
            
            std::string content = GetFirstLine(cur->content);
            return RegexMatch(content, HeadersRegex);
        }
        
        return false;
    }
    
    //
    // Block Classifier, Headers Context
    //
    template <>
    inline Section ClassifyBlock<HeaderCollection>(const BlockIterator& begin,
                                                   const BlockIterator& end,
                                                   const Section& context) {
        
        if (context == UndefinedSection &&
            HasHeaderSignature(begin, end)) {
            return HeadersSection;
        }
        else if (context == HeadersSection) {
            
            // Section closure
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSection;
            
            // Adjacent list item
            if (begin->type == ListItemBlockBeginType)
                return UndefinedSection;
        }
        
        return (context == HeadersSection) ? context : UndefinedSection;
    }
    
    //
    // Headers Section Parser
    //
    template<>
    struct SectionParser<HeaderCollection> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               HeaderCollection& headers) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            switch (section) {
                case HeadersSection:
                    result = HandleHeadersSectionBlock(cur, bounds, sourceData, blueprint, headers);
                    break;
                    
                case UndefinedSection:
                    result.second = CloseListItemBlock(cur, bounds.second);
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, cur->sourceMap);
                    break;
            }
            
            return result;
        }
        
        static ParseSectionResult HandleHeadersSectionBlock(const BlockIterator& cur,
                                                            const SectionBounds& bounds,
                                                            const SourceData& sourceData,
                                                            const Blueprint& blueprint,
                                                            HeaderCollection& headers) {

            SourceData data;
            SourceDataBlock sourceMap;
            ParseSectionResult result = ParseListPreformattedBlock(HeadersSection,
                                                                   cur,
                                                                   bounds,
                                                                   sourceData,
                                                                   data,
                                                                   sourceMap);
            if (result.first.error.code != Error::OK ||
                sourceData.empty())
                return result;
            
            // Proces raw data
            std::vector<std::string> lines = Split(data, '\n');
            for (std::vector<std::string>::iterator line = lines.begin();
                 line != lines.end();
                 ++line) {
                
                Header header;
                if (HeaderFromLine(*line, header)) {
                    
                    // TODO: check duplicates
                    headers.push_back(header);
                }
                else {
                    // WARN: unable to parse header
                    result.first.warnings.push_back(Warning("unable to parse HTTP header, expected"
                                                            " `<header name> : <header value>`, one header per line",
                                               1,
                                               sourceMap));
                }
            }
            
            return result;
        }
        
        // Parse one line of raw header data
        static inline bool HeaderFromLine(const std::string& line, Header& header) {
            
            std::vector<std::string> rawHeader = SplitOnFirst(line, ':');
            if (rawHeader.size() != 2)
                return false;
            
            header = std::make_pair(rawHeader[0], rawHeader[1]);
            TrimString(header.first);
            TrimString(header.second);
            
            return (!header.first.empty() && !header.second.empty());
        }
    };

    typedef BlockParser<HeaderCollection, SectionParser<HeaderCollection> > HeadersParser;
    
    //
    // Generic HeaderSection parser handler
    //
    template <class T>
    inline ParseSectionResult HandleHeaders(const BlockIterator& begin,
                                            const BlockIterator& end,
                                            const SourceData& sourceData,
                                            const Blueprint& blueprint,
                                            T& t)
    {
        HeaderCollection headers;
        ParseSectionResult result = HeadersParser::Parse(begin, end, sourceData, blueprint, headers);
        if (result.first.error.code != Error::OK)
            return result;
        
        if (headers.empty()) {
            BlockIterator nameBlock = ListItemNameBlock(begin, end);
            result.first.warnings.push_back(Warning("no headers specified",
                                                    0,
                                                    nameBlock->sourceMap));
        }
        else {
            // TODO: Check duplicates
            t.headers.insert(t.headers.end(), headers.begin(), headers.end());
            
        }
        return result;
    }
}

#endif
