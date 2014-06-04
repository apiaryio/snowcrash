//
//  HeaderParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/22/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_HEADERPARSER_H
#define SNOWCRASH_HEADERPARSER_H

#include "SectionParser.h"
#include "RegexMatch.h"
#include "CodeBlockUtility.h"
#include "StringUtility.h"
#include "BlueprintUtility.h"

namespace snowcrash {
    
    /** Internal type alias for collection of HTTP headers */
    typedef Collection<Header>::type Headers;

    /** Header Iterator in its containment group */
    typedef Collection<Header>::const_iterator HeaderIterator;
    
    /**
     *  Headers Section Processor
     */
    template<>
    struct SectionProcessor<Headers> : public SectionProcessorBase<Headers> {
        
        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     Report& report,
                                                     Headers& out) {
            mdp::ByteBuffer content;
            CodeBlockUtility::signatureContentAsCodeBlock(node, pd, report, content);

            headersFromContent(node, pd, report, out, content);

            return ++MarkdownNodeIterator(node);
        }
        
        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       SectionParserData& pd,
                                                       Report& report,
                                                       Headers& out) {
            return node;
        }
        
        static MarkdownNodeIterator processContent(const MarkdownNodeIterator& node,
                                                   SectionParserData& pd,
                                                   Report& report,
                                                   Headers& out) {
            mdp::ByteBuffer content;
            CodeBlockUtility::contentAsCodeBlock(node, pd, report, content);

            headersFromContent(node, pd, report, out, content);

            return ++MarkdownNodeIterator(node);
        }
        
        static bool isDescriptionNode(const MarkdownNodeIterator& node) {
            return false;
        }
        
        static bool isContentNode(const MarkdownNodeIterator& node) {
            return !RecognizeSection(node);
        }
        
        static SectionType sectionType(const MarkdownNodeIterator& node) {
            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {
                mdp::ByteBuffer subject = node->children().front().text;
                
                /** Headers matching regex */
                static const char* const HeadersRegex = "^[[:blank:]]*[Hh]eaders?[[:blank:]]*$";
                
                if (RegexMatch(subject, HeadersRegex))
                    return HeadersSectionType;
            }

            return UndefinedSectionType;
        }

        /** Retrieve headers from content */
        static void headersFromContent(const MarkdownNodeIterator& node,
                                       SectionParserData& pd,
                                       Report& report,
                                       Headers& headers,
                                       mdp::ByteBuffer content) {
            std::vector<std::string> lines = Split(content, '\n');

            for (std::vector<std::string>::iterator line = lines.begin();
                 line != lines.end();
                 ++line) {
                if (TrimString(*line) == "") {
                    continue;
                }

                Header header;

                if (CodeBlockUtility::keyValueFromLine(*line, header)) {
                    if (findHeader(headers, header) != headers.end()) {
                        std::stringstream ss;

                        ss << "duplicate definition of '" << header.first << "' header";

                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        report.warnings.push_back(Warning(ss.str(),
                                                          DuplicateWarning,
                                                          sourceMap));
                    }

                    headers.push_back(header);
                } else {
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(Warning("unable to parse HTTP header, expected '<header name> : <header value>', one header per line",
                                                      FormattingWarning,
                                                      sourceMap));
                }
            }
        }

        /** Finds a header in its containment group by its key (first) */
        static HeaderIterator findHeader(const Headers& headers,
                                         const Header& header) {
            return std::find_if(headers.begin(),
                                headers.end(),
                                std::bind2nd(MatchFirsts<Header>(), header));
        }
    };
    
    /** Headers Section Parser */
    typedef SectionParser<Headers, ListSectionAdapter> HeadersParser;
}

//namespace snowcrash {
//    
//    /** Helper for handling parsing of deprecated header sections */
//    template <typename T>
//    ParseSectionResult HandleDeprecatedHeaders(const BlueprintSection& section,
//                                               const BlockIterator& cur,
//                                               BlueprintParserCore& parser,
//                                               T& t) {
//        
//        ParseSectionResult result = HandleHeaders<T>(section, cur, parser, t);
//        
//        // WARN: Deprecated header sections
//        std::stringstream ss;
//        ss << "the 'headers' section at this level is deprecated and will be removed in a future, use respective payload header section(s) instead";
//        BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
//        SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, parser.sourceData);
//        result.first.warnings.push_back(Warning(ss.str(),
//                                                DeprecatedWarning,
//                                                sourceBlock));
//        return result;
//    }
//    
//    /** \brief Copy headers into example paylods headers. */
//    FORCEINLINE void InjectDeprecatedHeaders(const Collection<Header>::type& headers,
//                                        Collection<TransactionExample>::type& examples)
//    {
//        for (Collection<TransactionExample>::iterator exampleIt = examples.begin();
//             exampleIt != examples.end();
//             ++exampleIt) {
//            
//            // Requests
//            for (Collection<Request>::iterator reqIt = exampleIt->requests.begin();
//                 reqIt != exampleIt->requests.end();
//                 ++reqIt) {
//                
//                reqIt->headers.insert(reqIt->headers.begin(),
//                                      headers.begin(),
//                                      headers.end());
//            }
//            
//            // Responses
//            for (Collection<Response>::iterator resIt = exampleIt->responses.begin();
//                 resIt != exampleIt->responses.end();
//                 ++resIt) {
//                
//                resIt->headers.insert(resIt->headers.begin(),
//                                      headers.begin(),
//                                      headers.end());
//            }
//        }
//    }
//    
//    // Checks T's headers for occurence of R's headers, warns if a match is found.
//    template <class T, class R>
//    void CheckHeaderDuplicates(const T& left,
//                               const R& right,
//                               const SourceDataBlock& rightSourceMap,
//                               const SourceData& sourceData,
//                               Result& result) {
//        
//        for (HeaderIterator it = right.headers.begin(); it != right.headers.end(); ++it) {
//            if (FindHeader(left.headers, *it) != left.headers.end()) {
//                // WARN: overshadowing header definition
//                std::stringstream ss;
//                ss << "overshadowing previous '" << it->first << "' header definition";
//                result.warnings.push_back(Warning(ss.str(),
//                                                  RedefinitionWarning,
//                                                  MapSourceDataBlock(rightSourceMap, sourceData)));
//            }
//        }
//    }
//}

#endif
