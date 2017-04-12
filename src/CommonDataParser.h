//
//  CommonDataParser.h
//  snowcrash
//
//  Created by Egor Baranov on 30/03/2017.
//  Copyright © 2017 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_COMMONDATAPARSER_H
#define SNOWCRASH_COMMONDATAPARSER_H

#include "HeadersParser.h"

using namespace scpl;

namespace snowcrash {

    /** Common data matching regex */
    const char* const CommonDataRegex = "^[[:blank:]]*[Cc]ommon[[:blank:]]+[Dd]ata[[:blank:]]*$";

    /**
     * Common data section processor
     */
    template<>
    struct SectionProcessor<CommonData> : public SectionProcessorBase<CommonData> {

        NO_SECTION_DESCRIPTION(CommonData)

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<CommonData>& out) {

            MarkdownNodeIterator cur = node;
            switch (pd.sectionContext()) {
                case ResponseSectionType:
                case ResponseBodySectionType:
                {
                    IntermediateParseResult<Payload> payload(out.report);

                    cur = PayloadParser::parse(node, siblings, pd, payload);

                    out.node.content.responses.push_back(payload.node);

                    if (pd.exportSourceMap()) {
                        out.sourceMap.content.responses.collection.push_back(payload.sourceMap);
                    }

                    break;
                }

                default:
                    break;
            }

            return cur;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<CommonData>& out) {

            out.node.element = Element::CommonDataElement;

            if (pd.exportSourceMap()) {
                out.sourceMap.element = out.node.element;
            }
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {
            if (node->type == mdp::HeaderMarkdownNodeType &&
                !node->text.empty()) {

                mdp::ByteBuffer remaining, subject = node->text;

                subject = GetFirstLine(subject, remaining);
                TrimString(subject);

                if (RegexMatch(subject, CommonDataRegex)) {
                    return CommonDataSectionType;
                }
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {
            return SectionProcessor<Action>::nestedSectionType(node);
        }

        static SectionTypes upperSectionTypes() {
            return {CommonDataSectionType, DataStructureGroupSectionType, ResourceGroupSectionType, ResourceSectionType};
        }
    };

    /** Common Data Parser */
    typedef SectionParser<CommonData, HeaderSectionAdapter> CommonDataParser;
}

#endif
