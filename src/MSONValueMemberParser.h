//
//  MSONValueMemberParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/22/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONVALUEMEMBERPARSER_H
#define SNOWCRASH_MSONVALUEMEMBERPARSER_H

#include "SectionParser.h"
#include "MSONUtility.h"
#include "MSONTypeSectionParser.h"

using namespace scpl;

namespace snowcrash {

    /**
     * MSON Value Member Section Processor
     */
    template<>
    struct SectionProcessor<mson::ValueMember> : public SignatureSectionProcessorBase<mson::ValueMember> {

        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(SignatureTraits::ValuesTrait |
                                            SignatureTraits::AttributesTrait |
                                            SignatureTraits::ContentTrait);

            return signatureTraits;
        }

        static void finalizeSignature(const MarkdownNodeIterator& node,
                                      SectionParserData& pd,
                                      const Signature& signature,
                                      const ParseResultRef<mson::ValueMember>& out) {

            out.node.description = signature.content;

            for (std::vector<mdp::ByteBuffer>::const_iterator it = signature.values.begin();
                 it != signature.values.end();
                 it++) {

                out.node.valueDefinition.values.push_back(mson::parseValue(*it));
            }

            ParseResultRef<mson::TypeDefinition> typeDefinition(out.report, out.node.valueDefinition.typedefinition, out.sourceMap.valueDefinition.typeDefinition);
            mson::parseTypeDefinition(node, pd, signature.attributes, typeDefinition);

            if (!signature.remainingContent.empty()) {

                mson::TypeSection typeSection(mson::BlockDescriptionTypeSectionType);

                typeSection.content.description = signature.remainingContent;
                out.node.sections.push_back(typeSection);
            }
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       const ParseResultRef<mson::ValueMember>& out) {

            if (out.node.sections.empty() ||
                (out.node.sections.size() == 1 &&
                 out.node.sections[0].type == mson::BlockDescriptionTypeSectionType)) {

                if (out.node.sections.empty()) {

                    mson::TypeSection typeSection(mson::BlockDescriptionTypeSectionType);
                    out.node.sections.push_back(typeSection);
                }

                if (!out.node.sections[0].content.description.empty()) {
                    TwoNewLines(out.node.sections[0].content.description);
                }

                mdp::ByteBuffer content = mdp::MapBytesRangeSet(node->sourceMap, pd.sourceData);
                out.node.sections[0].content.description += content;

                return ++MarkdownNodeIterator(node);
            }

            return node;
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<mson::ValueMember>& out) {

            if ((pd.sectionContext() != MSONTypeSectionSectionType) &&
                (pd.sectionContext() != MSONMemberTypeGroupSectionType)) {

                return node;
            }

            IntermediateParseResult<mson::TypeSection> typeSection(out.report);

            MSONTypeSectionListParser::parse(node, siblings, pd, typeSection);

            out.node.sections.push_back(typeSection.node);

            return ++MarkdownNodeIterator(node);
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            if (node->type != mdp::ListItemMarkdownNodeType) {
                return true;
            }

            return SectionProcessorBase<mson::ValueMember>::isDescriptionNode(node, sectionType);
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            SectionType nestedType = UndefinedSectionType;

            // Check if mson type section section
            nestedType = SectionProcessor<mson::TypeSection>::sectionType(node);

            return nestedType;
        }
    };

    /** MSON Value Member Section Parser */
    typedef SectionParser<mson::ValueMember, ListSectionAdapter> MSONValueMemberParser;
}

#endif
