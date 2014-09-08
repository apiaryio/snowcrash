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
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     ParseResult<Headers>& out) {

            mdp::ByteBuffer content;
            CodeBlockUtility::signatureContentAsCodeBlock(node, pd, out.report, content);

            headersFromContent(node, content, pd, out);

            return ++MarkdownNodeIterator(node);
        }
        
        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       SectionParserData& pd,
                                                       ParseResult<Headers>& out) {

            return node;
        }

        static MarkdownNodeIterator processContent(const MarkdownNodeIterator& node,
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
                TrimString(subject);

                if (RegexMatch(subject, HeadersRegex))
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
                    if (findHeader(out.node, header) != out.node.end()) {
                        // WARN: duplicate header on this level
                        std::stringstream ss;

                        ss << "duplicate definition of '" << header.first << "' header";

                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        out.report.warnings.push_back(Warning(ss.str(),
                                                              DuplicateWarning,
                                                              sourceMap));
                    }

                    out.node.push_back(header);

                    if (pd.exportSM()) {
                        out.sourceMap.sourceMap.push_back(node->sourceMap);
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
            Collection<SourceMap<TransactionExample>>::iterator exampleSMIt;

            if (pd.exportSM()) {
                exampleSMIt = examplesSM.sourceMap.begin();
            }

            while (exampleIt != examples.end()) {

                Collection<Request>::iterator reqIt = exampleIt->requests.begin();
                Collection<SourceMap<Request>>::iterator reqSMIt;

                if (pd.exportSM()) {
                    reqSMIt = exampleSMIt->requests.sourceMap.begin();
                }

                // Requests
                while (reqIt != exampleIt->requests.end()) {

                     reqIt->headers.insert(reqIt->headers.begin(), headers.begin(), headers.end());
                     ++reqIt;

                     if (pd.exportSM()) {
                         reqSMIt->headers.sourceMap.insert(reqSMIt->headers.sourceMap.begin(), headersSM.sourceMap.begin(), headersSM.sourceMap.end());
                         ++reqSMIt;
                     }
                }

                Collection<Response>::iterator resIt = exampleIt->responses.begin();
                Collection<SourceMap<Response>>::iterator resSMIt;

                if (pd.exportSM()) {
                    resSMIt = exampleSMIt->responses.sourceMap.begin();
                }

                // Responses
                while(resIt != exampleIt->responses.end()) {

                    resIt->headers.insert(resIt->headers.begin(), headers.begin(), headers.end());
                    ++resIt;

                    if (pd.exportSM()) {
                        resSMIt->headers.sourceMap.insert(resSMIt->headers.sourceMap.begin(), headersSM.sourceMap.begin(), headersSM.sourceMap.end());
                        ++resSMIt;
                    }
                }

                ++exampleIt;

                if (pd.exportSM()) {
                    ++exampleSMIt;
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

#endif
