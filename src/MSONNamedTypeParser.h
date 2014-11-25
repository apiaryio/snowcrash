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

            mson::parseTypeName(signature.identifier, out.node.name);
            mson::parseTypeDefinition(node, pd, signature.attributes, out.report, out.node.base);

            // Named types should have type specification when sub-typed from primitive types
            if (out.node.base.baseType == mson::UndefinedBaseType) {
                out.node.base.baseType = mson::ImplicitObjectBaseType;
            }

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
                                                                        out.node.base.baseType);
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            return SectionProcessor<mson::ValueMember>::isDescriptionNode(node, sectionType);
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            return MSONNamedTypeSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            return SectionProcessor<mson::ValueMember>::nestedSectionType(node);
        }
    };

    /** MSON Named Type Section Parser */
    typedef SectionParser<mson::NamedType, HeaderSectionAdapter> MSONNamedTypeParser;
}

#endif