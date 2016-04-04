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

            ParseResultRef<mson::TypeSections> typeSections(out.report, out.node.sections, out.sourceMap.sections);

            return SectionProcessor<mson::ValueMember>
                    ::processNestedMembers<MSONTypeSectionListParser>(node, siblings, pd, typeSections,
                                                                      out.node.valueDefinition.typeDefinition.baseType);
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            if (SectionProcessor<mson::ValueMember>::nestedSectionType(node) != MSONSectionType ||
                node->type == mdp::HeaderMarkdownNodeType) {

                return false;
            }

            if (node->type != mdp::ListItemMarkdownNodeType) {
                return true;
            }

            return SectionProcessorBase<mson::ValueMember>::isDescriptionNode(node, sectionType);
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator&);

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<mson::ValueMember>& out) {

            SectionProcessor<mson::ValueMember>::finalizeImplicitBaseType(out.node.valueDefinition.typeDefinition.baseType);
        }

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

            if (pd.exportSourceMap() && !signature.content.empty()) {
                sourceMap.description.sourceMap = node->sourceMap;
            }

            mson::parseTypeDefinition(node, pd, signature.attributes, report, valueMember.valueDefinition.typeDefinition);
            parseRemainingContent(node, pd, signature.remainingContent, valueMember.sections, sourceMap.sections);

            // Check for circular references
            mson::TypeSpecification typeSpecification = valueMember.valueDefinition.typeDefinition.typeSpecification;

            if (typeSpecification.name.base == mson::ArrayTypeName ||
                typeSpecification.name.base == mson::EnumTypeName) {

                for (mson::TypeNames::iterator it = typeSpecification.nestedTypes.begin();
                     it != typeSpecification.nestedTypes.end();
                     ++it) {

                    if (!it->symbol.literal.empty() && !it->symbol.variable) {
                        mson::addDependency(node, pd, it->symbol.literal, pd.namedTypeContext, report);
                    }
                }
            }
            else if (typeSpecification.name.base == mson::UndefinedTypeName &&
                     !typeSpecification.name.symbol.literal.empty() &&
                     !typeSpecification.name.symbol.variable) {

                mson::addDependency(node, pd, typeSpecification.name.symbol.literal, pd.namedTypeContext, report);
            }

            // Properly parse the values in the signature
            if (signature.values.size() > 1) {

                if (valueMember.valueDefinition.typeDefinition.baseType == mson::PrimitiveBaseType) {

                    valueMember.valueDefinition.values.push_back(mson::parseValue(signature.value));

                    if (pd.exportSourceMap()) {
                        sourceMap.valueDefinition.sourceMap = node->sourceMap;
                    }

                    return ++MarkdownNodeIterator(node);
                }
                else if (valueMember.valueDefinition.typeDefinition.baseType == mson::UndefinedBaseType) {
                    valueMember.valueDefinition.typeDefinition.baseType = mson::ImplicitValueBaseType;
                }
            }

            for (std::vector<mdp::ByteBuffer>::const_iterator it = signature.values.begin();
                 it != signature.values.end();
                 it++) {

                valueMember.valueDefinition.values.push_back(mson::parseValue(*it));
            }

            if (pd.exportSourceMap() && !valueMember.valueDefinition.empty()) {
                sourceMap.valueDefinition.sourceMap = node->sourceMap;
            }

            if (pd.exportSourceMap()) {
                sourceMap.sourceMap = node->sourceMap;
            }

            return ++MarkdownNodeIterator(node);
        }

        /**
         * \brief Parse the signature's remaining content
         *
         * \param node Node to process
         * \param pd Section Parser Data
         * \param remainingContent Signature's remaining content
         * \param sections MSON Type Section collection
         * \param sourceMap MSON Type Section collection source map
         */
        static void parseRemainingContent(const MarkdownNodeIterator& node,
                                          SectionParserData& pd,
                                          const mdp::ByteBuffer& remainingContent,
                                          mson::TypeSections& sections,
                                          SourceMap<mson::TypeSections>& sourceMap) {

            if (remainingContent.empty()) {
                return;
            }

            mson::TypeSection typeSection(mson::TypeSection::BlockDescriptionClass);

            typeSection.content.description = remainingContent;
            sections.push_back(typeSection);

            if (pd.exportSourceMap()) {

                SourceMap<mson::TypeSection> typeSectionSM;

                typeSectionSM.description.sourceMap = node->sourceMap;
                sourceMap.collection.push_back(typeSectionSM);
            }
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
                 sections[0].klass == mson::TypeSection::BlockDescriptionClass)) {

                if (sections.empty()) {

                    mson::TypeSection typeSection(mson::TypeSection::BlockDescriptionClass);
                    sections.push_back(typeSection);

                    if (pd.exportSourceMap()) {

                        SourceMap<mson::TypeSection> typeSectionSM;
                        sourceMap.collection.push_back(typeSectionSM);
                    }
                }

                if (!sections[0].content.description.empty()) {
                    TwoNewLines(sections[0].content.description);
                }

                mdp::ByteBuffer content = mdp::MapBytesRangeSet(node->sourceMap, pd.sourceData);
                sections[0].content.description += content;

                if (pd.exportSourceMap() && !content.empty()) {
                    sourceMap.collection[0].description.sourceMap.append(node->sourceMap);
                }

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
         * \param sections MSON Type Section collection Parse Result
         * \param baseType Base Type of the MSON member to be sent for nested type sections
         */
        template<typename PARSER>
        static MarkdownNodeIterator processNestedMembers(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<mson::TypeSections>& sections,
                                                         mson::BaseType& baseType) {

            MarkdownNodeIterator cur = node;

            if (pd.sectionContext() == MSONSectionType ||
                pd.sectionContext() == MSONMixinSectionType ||
                pd.sectionContext() == MSONOneOfSectionType) {

                cur = processMSONSection(node, siblings, pd, sections, baseType);
            }
            else {

                // Try to resolve base type if not given before parsing further
                resolveImplicitBaseType(node, pd.sectionContext(), baseType);

                IntermediateParseResult<mson::TypeSection> typeSection(sections.report);
                typeSection.node.baseType = baseType;

                cur = PARSER::parse(node, siblings, pd, typeSection);

                if (typeSection.node.klass != mson::TypeSection::UndefinedClass) {
                    sections.node.push_back(typeSection.node);

                    if (pd.exportSourceMap()) {
                        sections.sourceMap.collection.push_back(typeSection.sourceMap);
                    }
                }
            }

            return cur;
        }

        /**
         * \brief Given a MSONSectionType nested section, process it
         *
         * \param node Node to process
         * \param siblings Siblings of the node being processed
         * \param pd Section Parser Data
         * \param sections MSON Type Section collection Parse Result
         * \param baseType Base Type of the MSON member to be sent for nested type sections
         */
        static MarkdownNodeIterator processMSONSection(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       const ParseResultRef<mson::TypeSections>& sections,
                                                       mson::BaseType& baseType);

        /**
         * \brief If base type is still undefined, make it implicit string
         *
         * \param MSON member base type
         */
        static void finalizeImplicitBaseType(mson::BaseType& baseType) {

            if (baseType == mson::UndefinedBaseType) {
                baseType = mson::ImplicitPrimitiveBaseType;
            }
        }

        /**
         * \brief Resolve base types if possible, based on nested node given
         *
         * \param node Node to process
         * \param sectionType Section Type
         * \param baseType Base Type of the MSON member that needs to be resolved
         */
        static void resolveImplicitBaseType(const MarkdownNodeIterator& node,
                                            const SectionType& sectionType,
                                            mson::BaseType& baseType) {

            if (baseType != mson::UndefinedBaseType) {
                return;
            }

            switch (sectionType) {
                case MSONSectionType:
                case MSONOneOfSectionType:
                case MSONMixinSectionType:
                {
                    if (node->type == mdp::ListItemMarkdownNodeType) {
                        baseType = mson::ImplicitObjectBaseType;
                    }

                    break;
                }

                case MSONPropertyMembersSectionType:
                {
                    baseType = mson::ImplicitObjectBaseType;
                    break;
                }

                case MSONSampleDefaultSectionType:
                {
                    baseType = mson::ImplicitPrimitiveBaseType;
                    break;
                }

                default:
                    break;
            }
        }
    };

    /** MSON Value Member Section Parser */
    typedef SectionParser<mson::ValueMember, ListSectionAdapter> MSONValueMemberParser;
}

#endif
