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
#include "RegexMatch.h"
#include "StringUtility.h"
#include "ListBlockUtility.h"

namespace snowcrashconst {
    
    /** Headers matching regex */
    const char* const HeadersRegex = "^[[:blank:]]*[Hh]eaders?[[:blank:]]*$";
}

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
            return RegexMatch(content, snowcrashconst::HeadersRegex);
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
    
    /** Internal list items classifier, Headers Context */
    template <>
    FORCEINLINE SectionType ClassifyInternaListBlock<HeaderCollection>(const BlockIterator& begin,
                                                                       const BlockIterator& end) {
        return UndefinedSectionType;
    }
    
    /** Children List Block Classifier, HeaderCollection context. */
    template <>
    FORCEINLINE SectionType ClassifyChildrenListBlock<HeaderCollection>(const BlockIterator& begin,
                                                                        const BlockIterator& end){
        return UndefinedSectionType;
    }
    
    /** Block Classifier, Headers Context */
    template <>
    FORCEINLINE SectionType ClassifyBlock<HeaderCollection>(const BlockIterator& begin,
                                                        const BlockIterator& end,
                                                        const SectionType& context) {
        
        if (context == UndefinedSectionType &&
            HasHeaderSignature(begin, end)) {
            return HeadersSectionType;
        }
        else if (context == HeadersSectionType) {
            
            // SectionType closure
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSectionType;
            
            // Adjacent list item
            if (begin->type == ListItemBlockBeginType)
                return UndefinedSectionType;
        }
        
        return (context == HeadersSectionType) ? context : UndefinedSectionType;
    }
    
    //
    // Headers SectionType Parser
    //
    template<>
    struct SectionParser<HeaderCollection> {
        
        static ParseSectionResult ParseSection(const BlueprintSection& section,
                                               const BlockIterator& cur,
                                               BlueprintParserCore& parser,
                                               HeaderCollection& headers) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            switch (section.type) {
                case HeadersSectionType:
                    result = HandleHeadersSectionBlock(section, cur, parser, headers);
                    break;
                    
                case UndefinedSectionType:
                    result.second = CloseList(cur, section.bounds.second);
                    break;
                    
                default:
                    result.first.error = UnexpectedBlockError(section, cur, parser.sourceData);
                    break;
            }
            
            return result;
        }
        
        static void Finalize(const SectionBounds& bounds,
                             BlueprintParserCore& parser,
                             HeaderCollection& headers,
                             Result& result) {}

        static ParseSectionResult HandleHeadersSectionBlock(const BlueprintSection& section,
                                                            const BlockIterator& cur,
                                                            BlueprintParserCore& parser,
                                                            HeaderCollection& headers) {

            SourceData data;
            SourceDataBlock sourceMap;
            ParseSectionResult result = ParseListPreformattedBlock<HeaderCollection>(section,
                                                                                     cur,
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
                                                                MapSourceDataBlock(sourceMap, parser.sourceData)));
                        
                    }
                        
                    headers.push_back(header);
                }
                else {
                    // WARN: unable to parse header
                    result.first.warnings.push_back(Warning("unable to parse HTTP header, expected"
                                                            " '<header name> : <header value>', one header per line",
                                               FormattingWarning,
                                               MapSourceDataBlock(sourceMap, parser.sourceData)));
                }
            }
            
            return result;
        }
    };

    typedef BlockParser<HeaderCollection, SectionParser<HeaderCollection> > HeadersParser;
    
    /**
     *  Generic HeaderSection parser handler
     */
    template <class T>
    ParseSectionResult HandleHeaders(const BlueprintSection& section,
                                     const BlockIterator& cur,
                                     BlueprintParserCore& parser,
                                     T& t)
    {
        size_t headerCount = t.headers.size();
        ParseSectionResult result = HeadersParser::Parse(cur,
                                                         section.bounds.second,
                                                         section,
                                                         parser,
                                                         t.headers);
        if (result.first.error.code != Error::OK)
            return result;
        
        if (t.headers.size() == headerCount) {
            BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
            SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, parser.sourceData);
            result.first.warnings.push_back(Warning("no headers specified",
                                                    FormattingWarning,
                                                    sourceBlock));
        }
        return result;
    }
    
    
    /** Helper for handling parsing of deprecated header sections */
    template <typename T>
    ParseSectionResult HandleDeprecatedHeaders(const BlueprintSection& section,
                                               const BlockIterator& cur,
                                               BlueprintParserCore& parser,
                                               T& t) {
        
        ParseSectionResult result = HandleHeaders<T>(section, cur, parser, t);
        
        // WARN: Deprecated header sections
        std::stringstream ss;
        ss << "the 'headers' section at this level is deprecated and will be removed in a future, use respective payload header section(s) instead";
        BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
        SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, parser.sourceData);
        result.first.warnings.push_back(Warning(ss.str(),
                                                DeprecatedWarning,
                                                sourceBlock));
        return result;
    }
    
    /** \brief Copy headers into example paylods headers. */
    FORCEINLINE void InjectDeprecatedHeaders(const Collection<Header>::type& headers,
                                        Collection<TransactionExample>::type& examples)
    {
        for (Collection<TransactionExample>::iterator exampleIt = examples.begin();
             exampleIt != examples.end();
             ++exampleIt) {
            
            // Requests
            for (Collection<Request>::iterator reqIt = exampleIt->requests.begin();
                 reqIt != exampleIt->requests.end();
                 ++reqIt) {
                
                reqIt->headers.insert(reqIt->headers.begin(),
                                      headers.begin(),
                                      headers.end());
            }
            
            // Responses
            for (Collection<Response>::iterator resIt = exampleIt->responses.begin();
                 resIt != exampleIt->responses.end();
                 ++resIt) {
                
                resIt->headers.insert(resIt->headers.begin(),
                                      headers.begin(),
                                      headers.end());
            }
        }
    }
    
    // Checks T's headers for occurence of R's headers, warns if a match is found.
    template <class T, class R>
    void CheckHeaderDuplicates(const T& left,
                               const R& right,
                               const SourceDataBlock& rightSourceMap,
                               const SourceData& sourceData,
                               Result& result) {
        
        for (HeaderIterator it = right.headers.begin(); it != right.headers.end(); ++it) {
            if (FindHeader(left.headers, *it) != left.headers.end()) {
                // WARN: overshadowing header definition
                std::stringstream ss;
                ss << "overshadowing previous '" << it->first << "' header definition";
                result.warnings.push_back(Warning(ss.str(),
                                                  RedefinitionWarning,
                                                  MapSourceDataBlock(rightSourceMap, sourceData)));
            }
        }
    }
}

#endif
