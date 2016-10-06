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
#include "RelationParser.h"
#include "RegexMatch.h"

namespace snowcrash {

    /** Nameless action matching regex */
    const char* const ActionHeaderRegex = "^[[:blank:]]*" HTTP_REQUEST_METHOD "[[:blank:]]*" URI_TEMPLATE "?$";

    /** Named action matching regex */
    const char* const NamedActionHeaderRegex = "^[[:blank:]]*" SYMBOL_IDENTIFIER "\\[" HTTP_REQUEST_METHOD "[[:blank:]]*" URI_TEMPLATE "?]$";

    /** Miss leading slash in URI */
    const char* const NamedActionNonAbsoluteURIRegex = "^[[:blank:]]*" SYMBOL_IDENTIFIER "\\[" HTTP_REQUEST_METHOD "[[:blank:]]+[^/]+]$";

    /** Internal type alias for Collection iterator of Action */
    typedef Collection<Action>::const_iterator ActionIterator;

    /** Action Definition Type */
    enum ActionType {
        NotActionType = 0,
        DependentActionType,      /// Action isn't fully defined, depends on parents resource URI
        CompleteActionType,       /// Action is fully defined including its URI
        UndefinedActionType = -1
    };

    /**
     * Action Section processor
     */
    template<>
    struct SectionProcessor<Action> : public SectionProcessorBase<Action> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     const ParseResultRef<Action>& out) {

            actionHTTPMethodAndName(node, out.node.method, out.node.name, out.node.uriTemplate);
            TrimString(out.node.name);

            mdp::ByteBuffer remainingContent;
            GetFirstLine(node->text, remainingContent);

            if (pd.exportSourceMap()) {
                if (!out.node.method.empty()) {
                    out.sourceMap.method.sourceMap = node->sourceMap;
                }

                if (!out.node.name.empty()) {
                    out.sourceMap.name.sourceMap = node->sourceMap;
                }

                if (!out.node.uriTemplate.empty()) {
                    out.sourceMap.uriTemplate.sourceMap = node->sourceMap;
                }
            }

            if (!remainingContent.empty()) {
                out.node.description += remainingContent;

                if (pd.exportSourceMap()) {
                    out.sourceMap.description.sourceMap.append(node->sourceMap);
                }
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<Action>& out) {

            SectionType sectionType = pd.sectionContext();
            MarkdownNodeIterator cur = node;
            std::stringstream ss;

            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);

            switch (sectionType) {
                case RelationSectionType:
                {
                    ParseResultRef<Relation> relation(out.report, out.node.relation, out.sourceMap.relation);
                    return RelationParser::parse(node, siblings, pd, relation);
                }

                case ParametersSectionType:
                {
                    ParseResultRef<Parameters> parameters(out.report, out.node.parameters, out.sourceMap.parameters);
                    return ParametersParser::parse(node, siblings, pd, parameters);
                }

                case RequestSectionType:
                case RequestBodySectionType:
                {
                    IntermediateParseResult<Payload> payload(out.report);

                    cur = PayloadParser::parse(node, siblings, pd, payload);

                    if (out.node.examples.empty() || !out.node.examples.back().responses.empty()) {
                        TransactionExample transaction;
                        SourceMap<TransactionExample> transactionSM;

                        out.node.examples.push_back(transaction);

                        if (pd.exportSourceMap()) {
                            out.sourceMap.examples.collection.push_back(transactionSM);
                        }
                    }

                    checkPayload(sectionType, sourceMap, payload.node, out);

                    out.node.examples.back().requests.push_back(payload.node);

                    if (pd.exportSourceMap()) {
                        out.sourceMap.examples.collection.back().requests.collection.push_back(payload.sourceMap);
                    }

                    break;
                }

                case ResponseSectionType:
                case ResponseBodySectionType:
                {
                    IntermediateParseResult<Payload> payload(out.report);

                    cur = PayloadParser::parse(node, siblings, pd, payload);

                    if (out.node.examples.empty()) {
                        TransactionExample transaction;
                        SourceMap<TransactionExample> transactionSM;

                        out.node.examples.push_back(transaction);

                        if (pd.exportSourceMap()) {
                            out.sourceMap.examples.collection.push_back(transactionSM);
                        }
                    }

                    checkPayload(sectionType, sourceMap, payload.node, out);

                    out.node.examples.back().responses.push_back(payload.node);

                    if (pd.exportSourceMap()) {
                        out.sourceMap.examples.collection.back().responses.collection.push_back(payload.sourceMap);
                    }

                    break;
                }

                case HeadersSectionType:
                {
                    ParseResultRef<Headers> headers(out.report, out.node.headers, out.sourceMap.headers);
                    return SectionProcessor<Action>::handleDeprecatedHeaders(node, siblings, pd, headers);
                }

                case AttributesSectionType:
                {
                    ParseResultRef<Attributes> attributes(out.report, out.node.attributes, out.sourceMap.attributes);
                    return AttributesParser::parse(node, siblings, pd, attributes);
                }

                default:
                    break;
            }

            return cur;
        }

        static bool isUnexpectedNode(const MarkdownNodeIterator& node,
                                     SectionType sectionType) {

            if (SectionProcessor<Asset>::sectionType(node) != UndefinedSectionType) {
                return true;
            }

            return SectionProcessorBase<Action>::isUnexpectedNode(node, sectionType);
        }

        static MarkdownNodeIterator processUnexpectedNode(const MarkdownNodeIterator& node,
                                                          const MarkdownNodes& siblings,
                                                          SectionParserData& pd,
                                                          SectionType& sectionType,
                                                          const ParseResultRef<Action>& out) {

            if ((node->type == mdp::ParagraphMarkdownNodeType ||
                 node->type == mdp::CodeMarkdownNodeType) &&
                (sectionType == ResponseBodySectionType ||
                 sectionType == ResponseSectionType) &&
                !out.node.examples.empty() &&
                !out.node.examples.back().responses.empty()) {

                mdp::ByteBuffer content = CodeBlockUtility::addDanglingAsset(node, pd, sectionType, out.report, out.node.examples.back().responses.back().body);

                if (pd.exportSourceMap() && !content.empty()) {
                    out.sourceMap.examples.collection.back().responses.collection.back().body.sourceMap.append(node->sourceMap);
                }

                return ++MarkdownNodeIterator(node);
            }

            if ((node->type == mdp::ParagraphMarkdownNodeType ||
                 node->type == mdp::CodeMarkdownNodeType) &&
                (sectionType == RequestBodySectionType ||
                 sectionType == RequestSectionType) &&
                !out.node.examples.empty() &&
                !out.node.examples.back().requests.empty()) {

                mdp::ByteBuffer content = CodeBlockUtility::addDanglingAsset(node, pd, sectionType, out.report, out.node.examples.back().requests.back().body);

                if (pd.exportSourceMap() && !content.empty()) {
                    out.sourceMap.examples.collection.back().requests.collection.back().body.sourceMap.append(node->sourceMap);
                }

                return ++MarkdownNodeIterator(node);
            }

            SectionType assetType = SectionProcessor<Asset>::sectionType(node);

            if (assetType != UndefinedSectionType) {

                // WARN: Ignoring section
                std::stringstream ss;
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);

                ss << "Ignoring " << SectionName(assetType) << " list item, ";
                ss << SectionName(assetType) << " list item is expected to be indented by 4 spaces or 1 tab";

                out.report.warnings.push_back(Warning(ss.str(),
                                                      IgnoringWarning,
                                                      sourceMap));

                return ++MarkdownNodeIterator(node);
            }

            return SectionProcessorBase<Action>::processUnexpectedNode(node, siblings, pd, sectionType, out);
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

            // Check if relation section
            nestedType = SectionProcessor<Relation>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            // Check if parameters section
            nestedType = SectionProcessor<Parameters>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            // Check if headers section
            nestedType = SectionProcessor<Headers>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            // Check if attributes section
            nestedType = SectionProcessor<Attributes>::sectionType(node);

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

        static SectionTypes upperSectionTypes() {
            return {ActionSectionType, ResourceSectionType, ResourceGroupSectionType, DataStructureGroupSectionType};
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<Action>& out) {

            if (!out.node.uriTemplate.empty()) {
                checkParametersEligibility<Action>(node, pd, out.node.parameters, out);
            }

            if (!out.node.headers.empty()) {

                SectionProcessor<Headers>::injectDeprecatedHeaders(pd, out.node.headers, out.sourceMap.headers, out.node.examples, out.sourceMap.examples);
                out.node.headers.clear();

                if (pd.exportSourceMap()) {
                    out.sourceMap.headers.collection.clear();
                }
            }

            if (out.node.examples.empty()) {

                // WARN: No response for action
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning("action is missing a response",
                                                      EmptyDefinitionWarning,
                                                      sourceMap));
            } else if (!out.node.examples.empty() &&
                !out.node.examples.back().requests.empty() &&
                out.node.examples.back().responses.empty()) {

                // WARN: No response for request
                std::stringstream ss;
                ss << "action is missing a response for ";

                if (out.node.examples.back().requests.back().name.empty()) {
                    ss << "a request";
                } else {
                    ss << "the '" << out.node.examples.back().requests.back().name << "' request";
                }

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(ss.str(),
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
                                 const ParseResultRef<Action>& out) {

            if (isPayloadDuplicate(sectionType, payload, out.node.examples.back())) {

                // WARN: Duplicate payload
                std::stringstream ss;
                ss << SectionName(sectionType) << " payload `" << payload.name << "`";
                ss << " already defined for `" << out.node.method << "` method";

                out.report.warnings.push_back(Warning(ss.str(),
                                                      DuplicateWarning,
                                                      sourceMap));
            }

            if (sectionType == ResponseSectionType || sectionType == ResponseBodySectionType) {

                HTTPStatusCode code;

                if (!payload.name.empty()) {
                    std::stringstream(payload.name) >> code;
                }

                HTTPMethodTraits methodTraits = GetMethodTrait(out.node.method);

                if (!methodTraits.allowBody && !payload.body.empty()) {

                    // WARN: Edge case for 2xx CONNECT
                    if (out.node.method == HTTPMethodName::Connect && code/100 == 2) {

                        std::stringstream ss;
                        ss << "the response for " << code << " " << out.node.method << " request MUST NOT include a " << SectionName(BodySectionType);

                        out.report.warnings.push_back(Warning(ss.str(),
                                                              EmptyDefinitionWarning,
                                                              sourceMap));
                    } else if (out.node.method != HTTPMethodName::Connect && !methodTraits.allowBody) {

                        std::stringstream ss;
                        ss << "the response for " << out.node.method << " request MUST NOT include a " << SectionName(BodySectionType);

                        out.report.warnings.push_back(Warning(ss.str(),
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

                RequestIterator duplicate = SectionProcessor<Payload>::findRequest(example, payload);
                return duplicate != example.requests.end();
            } else if (sectionType == ResponseSectionType) {

                ResponseIterator duplicate = SectionProcessor<Payload>::findResponse(example, payload);
                return duplicate != example.responses.end();
            }

            return false;
        }

        /** Warn about deprecated headers */
        static MarkdownNodeIterator handleDeprecatedHeaders(const MarkdownNodeIterator& node,
                                                            const MarkdownNodes& siblings,
                                                            SectionParserData& pd,
                                                            const ParseResultRef<Headers>& out) {

            MarkdownNodeIterator cur = HeadersParser::parse(node, siblings, pd, out);

            // WARN: Deprecated header sections
            std::stringstream ss;
            ss << "the 'headers' section at this level is deprecated and will be removed in a future, use respective payload header section(s) instead";

            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
            out.report.warnings.push_back(Warning(ss.str(),
                                                  DeprecatedWarning,
                                                  sourceMap));

            return cur;
        }

        /** \return %ActionType of a node */
        static ActionType actionType(const MarkdownNodeIterator& node) {

            if (node->type != mdp::HeaderMarkdownNodeType || node->text.empty())
                return NotActionType;

            mdp::ByteBuffer subject = node->text;
            TrimString(subject);

            if (RegexMatch(subject, NamedActionHeaderRegex)) {
                return DependentActionType;
            }

            CaptureGroups captureGroups;

            if (RegexCapture(subject, ActionHeaderRegex, captureGroups, 3)) {

                if (captureGroups[2].empty()) {
                    return DependentActionType;
                }
                else {
                    return CompleteActionType;
                }
            }

            return NotActionType;
        }

        /** \return HTTP request method and name of an action */
        static void actionHTTPMethodAndName(const MarkdownNodeIterator& node,
                                            mdp::ByteBuffer& method,
                                            mdp::ByteBuffer& name,
                                            mdp::ByteBuffer& uriTemplate) {

            CaptureGroups captureGroups;
            mdp::ByteBuffer subject, remaining;

            subject = GetFirstLine(node->text, remaining);
            TrimString(subject);

            if (RegexCapture(subject, ActionHeaderRegex, captureGroups, 3)) {
                method = captureGroups[1];
            } else if (RegexCapture(subject, NamedActionHeaderRegex, captureGroups, 4)) {
                name = captureGroups[1];
                method = captureGroups[2];
                uriTemplate = captureGroups[3];
            }

            return;
        }

        /** Finds an action inside an actions collection */
        static ActionIterator findAction(const Actions& actions,
                                         const Action& action) {

            return std::find_if(actions.begin(),
                                actions.end(),
                                std::bind2nd(MatchAction(), action));
        }

        /** Finds a relation identifier inside an actions collection */
        static ActionIterator findRelation(const Actions& actions,
                                           const Relation& relation) {

            return std::find_if(actions.begin(),
                                actions.end(),
                                std::bind2nd(MatchRelation(), relation));
        }


        static void checkForTypoMistake(const MarkdownNodeIterator& node, SectionParserData& pd, Report& report) {

            if (node->type != mdp::HeaderMarkdownNodeType) {
                return;
            }

            if (RegexMatch(node->text, NamedActionNonAbsoluteURIRegex)) {
                std::stringstream ss;
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);

                ss << "URI path in '" << node->text << "' is not absolute, it should have a leading forward slash";

                report.warnings.push_back(Warning(ss.str(),
                                          URIWarning,
                                          sourceMap));

            }
        }
    };

    /** Action Section Parser */
    typedef SectionParser<Action, HeaderSectionAdapter> ActionParser;
}

#endif
