//
//  ActionParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_ACTIONPARSER_H
#define SNOWCRASH_ACTIONPARSER_H

#include "HTTP.h"
#include "SectionParser.h"
#include "ParametersParser.h"
#include "PayloadParser.h"
#include "RegexMatch.h"

namespace snowcrash {

    /** Nameless action matching regex */
    const char* const ActionHeaderRegex = "^[[:blank:]]*" HTTP_REQUEST_METHOD "[[:blank:]]*" URI_TEMPLATE "?$";

    /** Named action matching regex */
    const char* const NamedActionHeaderRegex = "^[[:blank:]]*" SYMBOL_IDENTIFIER "\\[" HTTP_REQUEST_METHOD "]$";

    // Method signature
    enum ActionSignature {
        NoActionSignature = 0,
        MethodActionSignature,      // # GET
        MethodURIActionSignature,   // # GET /uri
        NamedActionSignature,       // # My Method [GET]
        UndefinedActionSignature = -1
    };

    /**
     * Action Section processor
     */
    template<>
    struct SectionProcessor<Action> : public SectionProcessorBase<Action> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     Report& report,
                                                     Action& out) {

            mdp::ByteBuffer signature, remainingContent;
            CaptureGroups captureGroups;

            signature = GetFirstLine(node->text, remainingContent);

            if (RegexCapture(signature, ActionHeaderRegex, captureGroups, 3)) {
                out.method = captureGroups[1];
            } else if (RegexCapture(signature, NamedActionHeaderRegex, captureGroups, 3)) {
                out.name = captureGroups[1];
                out.method = captureGroups[2];

                TrimString(out.name);
            }

            if (!remainingContent.empty()) {
                out.description += remainingContent;
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         Action& out) {

            SectionType sectionType = pd.sectionContext();
            MarkdownNodeIterator cur = node;
            Payload payload;

            switch (sectionType) {
                case ParametersSectionType:
                    return ParametersParser::parse(node, siblings, pd, report, out.parameters);

                case RequestSectionType:
                case RequestBodySectionType:
                    cur = PayloadParser::parse(node, siblings, pd, report, payload);

                    if (out.examples.empty() || !out.examples.back().responses.empty()) {
                        TransactionExample transaction;
                        out.examples.push_back(transaction);
                    }

                    checkPayload(node, pd, sectionType, payload, out, report);

                    out.examples.back().requests.push_back(payload);
                    break;

                case ResponseSectionType:
                case ResponseBodySectionType:
                    cur = PayloadParser::parse(node, siblings, pd, report, payload);

                    if (out.examples.empty()) {
                        TransactionExample transaction;
                        out.examples.push_back(transaction);
                    }

                    checkPayload(node, pd, sectionType, payload, out, report);

                    out.examples.back().responses.push_back(payload);
                    break;

                default:
                    break;
            }

            return cur;
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::HeaderMarkdownNodeType
                && !node->text.empty()) {

                mdp::ByteBuffer subject = node->text;

                if (RegexMatch(subject, ActionHeaderRegex)) {
                    return ActionSectionType;
                }

                if (RegexMatch(subject, NamedActionHeaderRegex)) {
                    return ActionSectionType;
                }
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            SectionType nestedType = UndefinedSectionType;

            // Check if parameters section
            nestedType = SectionProcessor<Parameters>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            // Check if payload section
            nestedType = SectionProcessor<Payload>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            return UndefinedSectionType;
        }

        static void checkPayload(const MarkdownNodeIterator& node,
                                 SectionParserData& pd,
                                 SectionType sectionType,
                                 Payload& payload,
                                 Action& action,
                                 Report& report) {

            bool warnEmptyBody = false;

            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);

            mdp::ByteBuffer contentLength;
            mdp::ByteBuffer transferEncoding;

            if (isPayloadDuplicate(sectionType, payload, action.examples.back())) {

                // WARN: Duplicate payload
                std::stringstream ss;
                ss << SectionName(sectionType) << " payload `" << payload.name << "`";
                ss << " already defined for `" << action.method << "` method";

                report.warnings.push_back(Warning(ss.str(),
                                                  DuplicateWarning,
                                                  sourceMap));
            }

            for (Collection<Header>::const_iterator it = payload.headers.begin();
                 it != payload.headers.end();
                 ++it) {

                if (it->first == HTTPHeaderName::ContentLength) {
                    contentLength = it->second;
                }

                if (it->first == HTTPHeaderName::TransferEncoding) {
                    transferEncoding = it->second;
                }
            }

            if ((sectionType == RequestSectionType || sectionType == RequestBodySectionType) && payload.body.empty()) {

                // Warn when content-length or transfer-encoding is specified or both headers and body are empty
                if (payload.headers.empty()) {
                    warnEmptyBody = true;
                } else {
                    warnEmptyBody = !contentLength.empty() || !transferEncoding.empty();
                }

                if (warnEmptyBody) {
                    // WARN: empty body
                    std::stringstream ss;
                    ss << "empty " << SectionName(sectionType) << " " << SectionName(BodySectionType);

                    if (!contentLength.empty()) {
                        ss << ", expected " << SectionName(BodySectionType) << " for '" << contentLength << "' Content-Length";
                    } else if (!transferEncoding.empty()) {
                        ss << ", expected " << SectionName(BodySectionType) << " for '" << transferEncoding << "' Transfer-Encoding";
                    }

                    report.warnings.push_back(Warning(ss.str(),
                                                      EmptyDefinitionWarning,
                                                      sourceMap));
                }
            } else if (sectionType == ResponseSectionType || sectionType == ResponseBodySectionType) {

                HTTPStatusCode code;

                if (!payload.name.empty()) {
                    std::stringstream(payload.name) >> code;
                }

                StatusCodeTraits statusCodeTraits = GetStatusCodeTrait(code);
                HTTPMethodTraits methodTraits = GetMethodTrait(action.method);

                if ((!statusCodeTraits.allowBody || !methodTraits.allowBody) && !payload.body.empty()) {

                    if (!statusCodeTraits.allowBody) {
                        // WARN: not empty body
                        std::stringstream ss;
                        ss << "the " << code << " response MUST NOT include a " << SectionName(BodySectionType);

                        report.warnings.push_back(Warning(ss.str(),
                                                          EmptyDefinitionWarning,
                                                          sourceMap));
                    }

                    // WARN: Edge case for 2xx CONNECT
                    if (action.method == HTTPMethodName::Connect && code/100 == 2) {

                        std::stringstream ss;
                        ss << "the response for " << code << " " << action.method << " request MUST NOT include a " << SectionName(BodySectionType);

                        report.warnings.push_back(Warning(ss.str(),
                                                          EmptyDefinitionWarning,
                                                          sourceMap));
                    } else if (action.method != HTTPMethodName::Connect && !methodTraits.allowBody) {

                        std::stringstream ss;
                        ss << "the response for " << action.method << " request MUST NOT include a " << SectionName(BodySectionType);

                        report.warnings.push_back(Warning(ss.str(),
                                                          EmptyDefinitionWarning,
                                                          sourceMap));
                    }

                    return;
                }
            }
        }


        /**
         *  Checks whether given section payload has duplicate within its transaction examples
         *  \return True when a duplicate is found, false otherwise.
         */
        static bool isPayloadDuplicate(const SectionType& sectionType,
                                       Payload& payload,
                                       TransactionExample& example) {

            if (sectionType == RequestSectionType) {

                Collection<Request>::const_iterator duplicate = FindRequest(example, payload);
                return duplicate != example.requests.end();
            } else if (sectionType == ResponseSectionType) {

                Collection<Response>::const_iterator duplicate = FindResponse(example, payload);
                return duplicate != example.responses.end();
            }

            return false;
        }
    };

    /** Action Section Parser */
    typedef SectionParser<Action, HeaderSectionAdapter> ActionParser;
}

#endif
