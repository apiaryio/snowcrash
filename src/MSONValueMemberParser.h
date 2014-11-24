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

        static MarkdownNodeIterator finalizeSignature(const MarkdownNodeIterator& node,
                                                      SectionParserData& pd,
                                                      const Signature& signature,
                                                      const ParseResultRef<mson::ValueMember>& out) {

            return SectionProcessor<mson::ValueMember>::useSignatureData(node, pd, signature, out.report, out.node, out.sourceMap);
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       const ParseResultRef<mson::ValueMember>& out) {

            return SectionProcessor<mson::ValueMember>::blockDescription(node, pd, out.node.sections, out.sourceMap.sections);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<mson::ValueMember>& out) {

            return SectionProcessor<mson::ValueMember>::processNestedMembers(node, siblings, pd, out.report,
                                                                             out.node.sections, out.sourceMap.sections,
                                                                             out.node.valueDefinition.typeDefinition.baseType);
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            if (node->type != mdp::ListItemMarkdownNodeType) {
                return true;
            }

            return SectionProcessorBase<mson::ValueMember>::isDescriptionNode(node, sectionType);
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator&);

        /**
         * \brief Use signature data to fill up the AST
         *
         * \param node Node to process
         * \param pd Section Parser Data
         * \param signature Signature data
         * \param report Parse result report
         * \param valueMember MSON Value Member
         * \param sourceMap MSON Value Member source map
         */
        static MarkdownNodeIterator useSignatureData(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     const Signature& signature,
                                                     Report& report,
                                                     mson::ValueMember& valueMember,
                                                     SourceMap<mson::ValueMember>& sourceMap) {

            valueMember.description = signature.content;

            for (std::vector<mdp::ByteBuffer>::const_iterator it = signature.values.begin();
                 it != signature.values.end();
                 it++) {

                valueMember.valueDefinition.values.push_back(mson::parseValue(*it));
            }

            mson::parseTypeDefinition(node, pd, signature.attributes, report, valueMember.valueDefinition.typeDefinition);

            if (!signature.remainingContent.empty()) {

                mson::TypeSection typeSection(mson::BlockDescriptionTypeSectionType);

                typeSection.content.description = signature.remainingContent;
                valueMember.sections.push_back(typeSection);
            }

            return ++MarkdownNodeIterator(node);
        }

        /**
         * \brief Add block description to the mson member
         *
         * \param node Node to process
         * \param pd Section Parser Data
         * \param sections MSON Type Section collection
         * \param sourceMap MSON Type Section collection source map
         */
        static MarkdownNodeIterator blockDescription(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     mson::TypeSections& sections,
                                                     SourceMap<mson::TypeSections>& sourceMap) {

            if (sections.empty() ||
                (sections.size() == 1 &&
                 sections[0].type == mson::BlockDescriptionTypeSectionType)) {

                if (sections.empty()) {

                    mson::TypeSection typeSection(mson::BlockDescriptionTypeSectionType);
                    sections.push_back(typeSection);
                }

                if (!sections[0].content.description.empty()) {
                    TwoNewLines(sections[0].content.description);
                }

                mdp::ByteBuffer content = mdp::MapBytesRangeSet(node->sourceMap, pd.sourceData);
                sections[0].content.description += content;

                return ++MarkdownNodeIterator(node);
            }

            return node;
        }

        /**
         * \brief Process nested member types for a member (either property or value) and Named Type.
         *
         * \param node Node to process
         * \param siblings Siblings of the node being processed
         * \param pd Section Parser Data
         * \param report Section Parser result report
         * \param sections MSON Type Section collection
         * \param sourceMap MSON Type Section collection source map
         * \param baseType Base Type of the MSON member to be sent for nested type sections
         */
        static MarkdownNodeIterator processNestedMembers(const MarkdownNodeIterator&,
                                                         const MarkdownNodes&,
                                                         SectionParserData&,
                                                         Report&,
                                                         mson::TypeSections&,
                                                         SourceMap<mson::TypeSections>&,
                                                         mson::BaseType&);
    };

    /** MSON Value Member Section Parser */
    typedef SectionParser<mson::ValueMember, ListSectionAdapter> MSONValueMemberParser;
}

#endif
