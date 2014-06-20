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

            MarkdownNodeIterator returnNode = node;
            Payload payload;

            switch (pd.sectionContext()) {
                case ParametersSectionType:
                    return ParametersParser::parse(node, siblings, pd, report, out.parameters);

                case RequestSectionType:
                case RequestBodySectionType:
                    returnNode = PayloadParser::parse(node, siblings, pd, report, payload);

//                    if (out.examples.empty() || !out.examples.back().responses.empty()) {
//                        TransactionExample transaction;
//                        out.examples.push_back(transaction);
//                    }
//
//                    out.examples.back().requests.push_back(payload);
                    break;

                case ResponseSectionType:
                case ResponseBodySectionType:
                    returnNode = PayloadParser::parse(node, siblings, pd, report, payload);
//                    out.examples.back().responses.push_back(payload);
                    break;

                default:
                    break;
            }

            return returnNode;
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
    };

    /** Action Section Parser */
    typedef SectionParser<Action, HeaderSectionAdapter> ActionParser;
}

#endif
