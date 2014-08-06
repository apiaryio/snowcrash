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

    /** Internal type alias for Collection of Action */
    typedef Collection<Action>::type Actions;

    typedef Collection<Action>::const_iterator ActionIterator;

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
                                                     bool& parsingRedirect,
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
            std::stringstream ss;

            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);

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

                    checkPayload(sectionType, sourceMap, payload, out, report);

                    out.examples.back().requests.push_back(payload);
                    break;

                case ResponseSectionType:
                case ResponseBodySectionType:
                    cur = PayloadParser::parse(node, siblings, pd, report, payload);

                    if (out.examples.empty()) {
                        TransactionExample transaction;
                        out.examples.push_back(transaction);
                    }

                    checkPayload(sectionType, sourceMap, payload, out, report);

                    out.examples.back().responses.push_back(payload);
                    break;

                case HeadersSectionType:
                    return SectionProcessor<Action>::handleDeprecatedHeaders(node, siblings, pd, report, out.headers);

                case BodySectionType:
                case SchemaSectionType:
                    // TODO: Can improve this

                    // WARN: Ignoring section
                    ss << "Ignoring " << SectionName(sectionType) << " list item, ";
                    ss << SectionName(sectionType) << " list item is expected to be indented by  4  spaces or 1 tab";

                    report.warnings.push_back(Warning(ss.str(),
                                                      IgnoringWarning,
                                                      sourceMap));

                    cur = ++MarkdownNodeIterator(node);
                    break;

                default:
                    break;
            }

            return cur;
        }

        static MarkdownNodeIterator processUnexpectedNode(const MarkdownNodeIterator& node,
                                                          const MarkdownNodes& siblings,
                                                          SectionParserData& pd,
                                                          SectionType& sectionType,
                                                          Report& report,
                                                          Action& out) {

            if ((node->type == mdp::ParagraphMarkdownNodeType ||
                 node->type == mdp::CodeMarkdownNodeType) &&
                (sectionType == ResponseBodySectionType ||
                 sectionType == ResponseSectionType) &&
                !out.examples.empty() &&
                !out.examples.back().responses.empty()) {

                CodeBlockUtility::addDanglingAsset(node, pd, sectionType, report, out.examples.back().responses.back().body);
            } else {

                // WARN: Ignoring unexpected node
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning("ignoring unrecognized block",
                                                  IgnoringWarning,
                                                  sourceMap));
            }

            return ++MarkdownNodeIterator(node);
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::HeaderMarkdownNodeType
                && !node->text.empty()) {

                mdp::ByteBuffer subject = node->text;
                TrimString(subject);

                if (RegexMatch(subject, ActionHeaderRegex) ||
                    RegexMatch(subject, NamedActionHeaderRegex)) {

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

            // Check if headers section
            nestedType = SectionProcessor<Headers>::sectionType(node);

            if (nestedType == HeadersSectionType) {
                return nestedType;
            }

            // Check if payload section
            nestedType = SectionProcessor<Payload>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            // Check if asset section (dangling)
            nestedType = SectionProcessor<Asset>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            return UndefinedSectionType;
        }

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested, types;

            // Parameters & descendants
            nested.push_back(ParametersSectionType);
            types = SectionProcessor<Parameters>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            // Payload & descendants
            nested.push_back(ResponseBodySectionType);
            nested.push_back(ResponseSectionType);
            nested.push_back(RequestBodySectionType);
            nested.push_back(RequestSectionType);

            types = SectionProcessor<Payload>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            return nested;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             Report& report,
                             Action& out) {

            if (!out.headers.empty()) {

                SectionProcessor<Headers>::injectDeprecatedHeaders(out.headers, out.examples);
                out.headers.clear();
            }

            if (out.examples.empty()) {

                // WARN: No response for action
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning("action is missing a response",
                                                  EmptyDefinitionWarning,
                                                  sourceMap));
            } else if (!out.examples.empty() &&
                !out.examples.back().requests.empty() &&
                out.examples.back().responses.empty()) {

                // WARN: No response for request
                std::stringstream ss;
                ss << "action is missing a response for ";

                if (out.examples.back().requests.back().name.empty()) {
                    ss << "a request";
                } else {
                    ss << "the '" << out.examples.back().requests.back().name << "' request";
                }

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning(ss.str(),
                                                  EmptyDefinitionWarning,
                                                  sourceMap));
            }
        }

        /**
         *  \brief  Check & report payload validity.
         *  \param  sectionType A section of the payload.
         *  \param  sourceMap   Payload signature source map.
         *  \param  payload     The payload to be checked.
         *  \param  action      The Action to which payload belongs to.
         *  \param  report      Parser report.
         */
        static void checkPayload(SectionType sectionType,
                                 const mdp::CharactersRangeSet sourceMap,
                                 const Payload& payload,
                                 const Action& action,
                                 Report& report) {

            if (isPayloadDuplicate(sectionType, payload, action.examples.back())) {

                // WARN: Duplicate payload
                std::stringstream ss;
                ss << SectionName(sectionType) << " payload `" << payload.name << "`";
                ss << " already defined for `" << action.method << "` method";

                report.warnings.push_back(Warning(ss.str(),
                                                  DuplicateWarning,
                                                  sourceMap));
            }

            if (sectionType == ResponseSectionType || sectionType == ResponseBodySectionType) {

                HTTPStatusCode code;

                if (!payload.name.empty()) {
                    std::stringstream(payload.name) >> code;
                }

                HTTPMethodTraits methodTraits = GetMethodTrait(action.method);

                if (!methodTraits.allowBody && !payload.body.empty()) {

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
        static bool isPayloadDuplicate(SectionType& sectionType,
                                       const Payload& payload,
                                       const TransactionExample& example) {

            if (sectionType == RequestSectionType) {

                Collection<Request>::const_iterator duplicate = FindRequest(example, payload);
                return duplicate != example.requests.end();
            } else if (sectionType == ResponseSectionType) {

                Collection<Response>::const_iterator duplicate = FindResponse(example, payload);
                return duplicate != example.responses.end();
            }

            return false;
        }

        /** Warn about deprecated headers */
        static MarkdownNodeIterator handleDeprecatedHeaders(const MarkdownNodeIterator& node,
                                            const MarkdownNodes& siblings,
                                            SectionParserData& pd,
                                            Report& report,
                                            Headers& headers) {

            MarkdownNodeIterator cur = HeadersParser::parse(node, siblings, pd, report, headers);

            // WARN: Deprecated header sections
            std::stringstream ss;
            ss << "the 'headers' section at this level is deprecated and will be removed in a future, use respective payload header section(s) instead";

            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
            report.warnings.push_back(Warning(ss.str(),
                                              DeprecatedWarning,
                                              sourceMap));

            return cur;
        }
    };

    /** Action Section Parser */
    typedef SectionParser<Action, HeaderSectionAdapter> ActionParser;
}

#endif
