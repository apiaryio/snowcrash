//
//  MSONNamedTypeParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONNAMEDTYPEPARSER_H
#define SNOWCRASH_MSONNAMEDTYPEPARSER_H

#include "MSONValueMemberParser.h"

using namespace scpl;

namespace snowcrash {

    /** MSON reserved characters matching regex */
    const char* const MSONReservedCharsRegex = "[]:\()<>\{}[_*+`-]+";

    /**
     * MSON Named Type Section Processor
     */
    template<>
    struct SectionProcessor<mson::NamedType> : public SignatureSectionProcessorBase<mson::NamedType> {

        static SignatureTraits signatureTraits() {


            SignatureTraits signatureTraits(SignatureTraits::IdentifierTrait |
                                            SignatureTraits::AttributesTrait);

            return signatureTraits;
        }

        static MarkdownNodeIterator finalizeSignature(const MarkdownNodeIterator& node,
                                                      SectionParserData& pd,
                                                      const Signature& signature,
                                                      const ParseResultRef<mson::NamedType>& out) {

            mson::parseTypeName(signature.identifier, out.node.name, false);
            mson::parseTypeDefinition(node, pd, signature.attributes, out.report, out.node.typeDefinition);

            mdp::ByteBuffer subject = node->text;
            TrimString(subject);

            if (subject[0] != '`' && RegexMatch(out.node.name.symbol.literal, MSONReservedCharsRegex)) {

                // WARN: named type name should not contain reserved characters
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning("please escape the name of the data structure using backticks since it contains MSON reserved characters",
                                                      FormattingWarning,
                                                      sourceMap));
            }

            if (pd.exportSourceMap()) {

                if (!out.node.name.empty()) {
                    out.sourceMap.name.sourceMap = node->sourceMap;
                }

                if (!out.node.typeDefinition.empty()) {
                    out.sourceMap.typeDefinition.sourceMap = node->sourceMap;
                }
            }

            // Named types should have type specification when sub-typed from primitive types
            if (out.node.typeDefinition.baseType == mson::UndefinedBaseType) {
                out.node.typeDefinition.baseType = mson::ImplicitObjectBaseType;
            }

            // Setup named type context
            pd.namedTypeContext = out.node.name.symbol.literal;

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       const ParseResultRef<mson::NamedType>& out) {

            return SectionProcessor<mson::ValueMember>::blockDescription(node, pd, out.node.sections, out.sourceMap.sections);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<mson::NamedType>& out) {

            ParseResultRef<mson::TypeSections> typeSections(out.report, out.node.sections, out.sourceMap.sections);

            return SectionProcessor<mson::ValueMember>
                    ::processNestedMembers<MSONTypeSectionHeaderParser>(node, siblings, pd, typeSections,
                                                                        out.node.typeDefinition.baseType);
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            return SectionProcessor<mson::ValueMember>::isDescriptionNode(node, sectionType);
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            SectionType sectionType = SectionKeywordSignature(node);

            if (node->type == mdp::HeaderMarkdownNodeType &&
                sectionType == UndefinedSectionType) {

                return MSONNamedTypeSectionType;
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            return SectionProcessor<mson::ValueMember>::nestedSectionType(node);
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<mson::NamedType>& out) {

            // Clear named type context
            pd.namedTypeContext.clear();
        }
    };

    /** MSON Named Type Section Parser */
    typedef SectionParser<mson::NamedType, HeaderSectionAdapter> MSONNamedTypeParser;
}

#endif
