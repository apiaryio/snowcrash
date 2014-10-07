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

    /** Headers matching regex */
    const char* const HeadersRegex = "^[[:blank:]]*[Hh]eaders?[[:blank:]]*$";

    /** Header Iterator in its containment group */
    typedef Collection<Header>::const_iterator HeaderIterator;
    
    /**
     *  Headers Section Processor
     */
    template<>
    struct SectionProcessor<Headers> : public SectionProcessorBase<Headers> {
        
        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     ParseResult<Headers>& out) {

            mdp::ByteBuffer content;
            CodeBlockUtility::signatureContentAsCodeBlock(node, pd, out.report, content);

            headersFromContent(node, content, pd, out);

            return ++MarkdownNodeIterator(node);
        }
        
        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       ParseResult<Headers>& out) {

            return node;
        }

        static MarkdownNodeIterator processContent(const MarkdownNodeIterator& node,
                                                   const MarkdownNodes& siblings,
                                                   SectionParserData& pd,
                                                   ParseResult<Headers>& out) {

            mdp::ByteBuffer content;
            CodeBlockUtility::contentAsCodeBlock(node, pd, out.report, content);

            headersFromContent(node, content, pd, out);

            return ++MarkdownNodeIterator(node);
        }
        
        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {
            return false;
        }
        
        static bool isContentNode(const MarkdownNodeIterator& node,
                                  SectionType sectionType) {

            return (SectionKeywordSignature(node) == UndefinedSectionType);
        }
        
        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {

                mdp::ByteBuffer subject = node->children().front().text;
                mdp::ByteBuffer signature;
                mdp::ByteBuffer remainingContent;
                
                signature = GetFirstLine(subject, remainingContent);
                TrimString(signature);

                if (RegexMatch(signature, HeadersRegex))
                    return HeadersSectionType;
            }

            return UndefinedSectionType;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             ParseResult<Headers>& out) {

            if (out.node.empty()) {

                // WARN: No headers defined
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.warnings.push_back(Warning("no headers specified",
                                                      FormattingWarning,
                                                      sourceMap));
            }
        }

        /** Retrieve headers from content */
        static void headersFromContent(const MarkdownNodeIterator& node,
                                       const mdp::ByteBuffer& content,
                                       SectionParserData& pd,
                                       ParseResult<Headers>& out) {

            std::vector<std::string> lines = Split(content, '\n');

            for (std::vector<std::string>::iterator line = lines.begin();
                 line != lines.end();
                 ++line) {

                if (TrimString(*line).empty()) {
                    continue;
                }

                Header header;

                if (CodeBlockUtility::keyValueFromLine(*line, header)) {
                    if (findHeader(out.node, header) != out.node.end() && !isAllowedMultipleDefinition(header)) {
                        // WARN: duplicate header on this level
                        std::stringstream ss;

                        ss << "duplicate definition of '" << header.first << "' header";

                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        out.report.warnings.push_back(Warning(ss.str(),
                                                              DuplicateWarning,
                                                              sourceMap));
                    }

                    out.node.push_back(header);

                    if (pd.exportSourceMap()) {
                        SourceMap<Header> headerSM;
                        headerSM.sourceMap = node->sourceMap;
                        out.sourceMap.collection.push_back(headerSM);
                    }
                } else {
                    // WARN: unable to parse header
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.warnings.push_back(Warning("unable to parse HTTP header, expected '<header name> : <header value>', one header per line",
                                                          FormattingWarning,
                                                          sourceMap));
                }
            }
        }

        /** Inject headers into transaction examples requests and responses */
        static void injectDeprecatedHeaders(SectionParserData& pd,
                                            const Headers& headers,
                                            const SourceMap<Headers>& headersSM,
                                            TransactionExamples& examples,
                                            SourceMap<TransactionExamples>& examplesSM) {

            Collection<TransactionExample>::iterator exampleIt = examples.begin();
            Collection<SourceMap<TransactionExample> >::iterator exampleSourceMapIt;

            if (pd.exportSourceMap()) {
                exampleSourceMapIt = examplesSM.collection.begin();
            }

            while (exampleIt != examples.end()) {

                Collection<Request>::iterator requestIt = exampleIt->requests.begin();
                Collection<SourceMap<Request> >::iterator requestSourceMapIt;

                if (pd.exportSourceMap()) {
                    requestSourceMapIt = exampleSourceMapIt->requests.collection.begin();
                }

                // Requests
                while (requestIt != exampleIt->requests.end()) {

                     requestIt->headers.insert(requestIt->headers.begin(), headers.begin(), headers.end());
                     ++requestIt;

                     if (pd.exportSourceMap()) {
                         requestSourceMapIt->headers.collection.insert(requestSourceMapIt->headers.collection.begin(),
                                                                       headersSM.collection.begin(),
                                                                       headersSM.collection.end());
                         ++requestSourceMapIt;
                     }
                }

                Collection<Response>::iterator responseIt = exampleIt->responses.begin();
                Collection<SourceMap<Response> >::iterator responseSourceMapIt;

                if (pd.exportSourceMap()) {
                    responseSourceMapIt = exampleSourceMapIt->responses.collection.begin();
                }

                // Responses
                while(responseIt != exampleIt->responses.end()) {

                    responseIt->headers.insert(responseIt->headers.begin(), headers.begin(), headers.end());
                    ++responseIt;

                    if (pd.exportSourceMap()) {
                        responseSourceMapIt->headers.collection.insert(responseSourceMapIt->headers.collection.begin(),
                                                                       headersSM.collection.begin(),
                                                                       headersSM.collection.end());
                        ++responseSourceMapIt;
                    }
                }

                ++exampleIt;

                if (pd.exportSourceMap()) {
                    ++exampleSourceMapIt;
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

        typedef std::vector<std::string> HeadersKeyCollection;
        /** Get collection of allowed keywords - workarround due to C++98 restriction - static initialization of vector */
        static const HeadersKeyCollection& getAllowedMultipleDefinitions() {
            static std::string keys[] = {
                HTTPHeaderName::SetCookie,
                HTTPHeaderName::Link,
            };

            static const HeadersKeyCollection allowedMultipleDefinitions(keys, keys + (sizeof(keys)/sizeof(keys[0])));
            return allowedMultipleDefinitions;
        }

        /** Check if Header name has allowed multiple definitions */
        static bool isAllowedMultipleDefinition(const Header& header) {
              const HeadersKeyCollection& keys = getAllowedMultipleDefinitions();
              return std::find_if(keys.begin(),
                                  keys.end(),
                                  std::bind1st(MatchFirstWith<Header, std::string>(), header)) != keys.end();
        }
    };

    /** Headers Section Parser */
    typedef SectionParser<Headers, ListSectionAdapter> HeadersParser;
}

#endif
