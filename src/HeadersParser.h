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

    /** Internal type alias for collection of HTTP headers */
    typedef Collection<Header>::type Headers;

    /** Header Iterator in its containment group */
    typedef Collection<Header>::const_iterator HeaderIterator;
    
    /**
     *  Headers Section Processor
     */
    template<>
    struct SectionProcessor<Headers, HeadersSM> : public SectionProcessorBase<Headers, HeadersSM> {
        
        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     Report& report,
                                                     Headers& out,
                                                     HeadersSM& outSM) {

            mdp::ByteBuffer content;
            CodeBlockUtility::signatureContentAsCodeBlock(node, pd, report, content);

            headersFromContent(node, content, pd, report, out);

            return ++MarkdownNodeIterator(node);
        }
        
        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       Report& report,
                                                       Headers& out,
                                                       HeadersSM& outSM) {

            return node;
        }

        static MarkdownNodeIterator processContent(const MarkdownNodeIterator& node,
                                                   const MarkdownNodes& siblings,
                                                   SectionParserData& pd,
                                                   Report& report,
                                                   Headers& out,
                                                   HeadersSM& outSM) {

            mdp::ByteBuffer content;
            CodeBlockUtility::contentAsCodeBlock(node, pd, report, content);

            headersFromContent(node, content, pd, report, out);

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
                             Report& report,
                             Headers& out,
                             HeadersSM& outSM) {

            if (out.empty()) {

                // WARN: No headers defined
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning("no headers specified",
                                                  FormattingWarning,
                                                  sourceMap));
            }
        }

        /** Retrieve headers from content */
        static void headersFromContent(const MarkdownNodeIterator& node,
                                       const mdp::ByteBuffer& content,
                                       SectionParserData& pd,
                                       Report& report,
                                       Headers& headers) {

            std::vector<std::string> lines = Split(content, '\n');

            for (std::vector<std::string>::iterator line = lines.begin();
                 line != lines.end();
                 ++line) {
                if (TrimString(*line).empty()) {
                    continue;
                }

                Header header;

                if (CodeBlockUtility::keyValueFromLine(*line, header)) {
                    if (findHeader(headers, header) != headers.end()) {
                        // WARN: duplicate header on this level
                        std::stringstream ss;

                        ss << "duplicate definition of '" << header.first << "' header";

                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        report.warnings.push_back(Warning(ss.str(),
                                                          DuplicateWarning,
                                                          sourceMap));
                    }

                    headers.push_back(header);
                } else {
                    // WARN: unable to parse header
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(Warning("unable to parse HTTP header, expected '<header name> : <header value>', one header per line",
                                                      FormattingWarning,
                                                      sourceMap));
                }
            }
        }

        /** Inject headers into transaction examples requests and responses */
        static void injectDeprecatedHeaders(const Headers& headers,
                                            Collection<TransactionExample>::type& examples) {

            for (Collection<TransactionExample>::iterator exampleIt = examples.begin();
                 exampleIt != examples.end();
                 ++exampleIt) {

                // Requests
                for (Collection<Request>::iterator reqIt = exampleIt->requests.begin();
                     reqIt != exampleIt->requests.end();
                     ++reqIt) {

                     reqIt->headers.insert(reqIt->headers.begin(), headers.begin(), headers.end());
                }

                // Responses
                for (Collection<Response>::iterator resIt = exampleIt->responses.begin();
                     resIt != exampleIt->responses.end();
                     ++resIt) {

                    resIt->headers.insert(resIt->headers.begin(), headers.begin(), headers.end());
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
    typedef SectionParser<Headers, HeadersSM, ListSectionAdapter> HeadersParser;
}

#endif
