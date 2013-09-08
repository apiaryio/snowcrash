//
//  HeaderParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/22/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_HEADERPARSER_H
#define SNOWCRASH_HEADERPARSER_H

#include <sstream>
#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "ListUtility.h"
#include "RegexMatch.h"
#include "StringUtility.h"

// Headers matching regex
static const std::string HeadersRegex("^[ \\t]*[Hh]eaders?[ \\t]*$");

namespace snowcrash {

    // Internal type alias for Collection of Headers
    typedef Collection<Header>::type HeaderCollection;
    
    // Query header signature a of given block
    FORCEINLINE bool HasHeaderSignature(const BlockIterator& begin,
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
    
    // Header iterator in its containment group
    typedef Collection<Header>::const_iterator HeaderIterator;
    
    // Finds a header in its containment group by its key (first)
    FORCEINLINE HeaderIterator FindHeader(const HeaderCollection& headers,
                                          const Header& header) {
        
        return std::find_if(headers.begin(),
                            headers.end(),
                            std::bind2nd(MatchFirsts<Header>(), header));
    }
    
    
    //
    // Block Classifier, Headers Context
    //
    template <>
    FORCEINLINE Section ClassifyBlock<HeaderCollection>(const BlockIterator& begin,
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
                                               BlueprintParserCore& parser,
                                               HeaderCollection& headers) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            switch (section) {
                case HeadersSection:
                    result = HandleHeadersSectionBlock(cur, bounds, parser, headers);
                    break;
                    
                case UndefinedSection:
                    result.second = CloseListItemBlock(cur, bounds.second);
                    break;
                    
                default:
                    result.first.error = UnexpectedBlockError(*cur);
                    break;
            }
            
            return result;
        }
        
        static ParseSectionResult HandleHeadersSectionBlock(const BlockIterator& cur,
                                                            const SectionBounds& bounds,
                                                            BlueprintParserCore& parser,
                                                            HeaderCollection& headers) {

            SourceData data;
            SourceDataBlock sourceMap;
            ParseSectionResult result = ParseListPreformattedBlock(HeadersSection,
                                                                   cur,
                                                                   bounds,
                                                                   parser,
                                                                   data,
                                                                   sourceMap);
            if (result.first.error.code != Error::OK ||
                parser.sourceData.empty())
                return result;
            
            // Proces raw data
            std::vector<std::string> lines = Split(data, '\n');
            for (std::vector<std::string>::iterator line = lines.begin();
                 line != lines.end();
                 ++line) {
                
                Header header;
                if (KeyValueFromLine(*line, header)) {
                    
                    if (FindHeader(headers, header) != headers.end()) {
                        // WARN: duplicate header on this level
                        std::stringstream ss;
                        ss << "duplicate definition of '" << header.first << "' header";
                        result.first.warnings.push_back(Warning(ss.str(),
                                                                DuplicateWarning,
                                                                sourceMap));
                        
                    }
                        
                    headers.push_back(header);
                }
                else {
                    // WARN: unable to parse header
                    result.first.warnings.push_back(Warning("unable to parse HTTP header, expected"
                                                            " '<header name> : <header value>', one header per line",
                                               FormattingWarning,
                                               sourceMap));
                }
            }
            
            return result;
        }
    };

    typedef BlockParser<HeaderCollection, SectionParser<HeaderCollection> > HeadersParser;
    
    //
    // Generic HeaderSection parser handler
    //
    template <class T>
    ParseSectionResult HandleHeaders(const BlockIterator& begin,
                                     const BlockIterator& end,
                                     BlueprintParserCore& parser,
                                     T& t)
    {
        size_t headerCount = t.headers.size();
        ParseSectionResult result = HeadersParser::Parse(begin, end, parser, t.headers);
        if (result.first.error.code != Error::OK)
            return result;
        
        if (t.headers.size() == headerCount) {
            BlockIterator nameBlock = ListItemNameBlock(begin, end);
            result.first.warnings.push_back(Warning("no headers specified",
                                                    FormattingWarning,
                                                    nameBlock->sourceMap));
        }
        return result;
    }
    
    // Checks T's headers for occurence of R's headers, warns if a match is found.
    template <class T, class R>
    void CheckHeaderDuplicates(const T& left,
                               const R& right,
                               const SourceDataBlock& rightSourceMap,
                               Result& result) {
        
        for (HeaderIterator it = right.headers.begin(); it != right.headers.end(); ++it) {
            if (FindHeader(left.headers, *it) != left.headers.end()) {
                // WARN: overshadowing header definition
                std::stringstream ss;
                ss << "overshadowing previous '" << it->first << "' header definition";
                result.warnings.push_back(Warning(ss.str(),
                                                  RedefinitionWarning,
                                                  rightSourceMap));
            }
        }
    }
}

#endif
