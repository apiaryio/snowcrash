//
//  MSONPropertyMemberParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONPROPERTYMEMBERPARSER_H
#define SNOWCRASH_MSONPROPERTYMEMBERPARSER_H

#include "MSONValueMemberParser.h"

using namespace scpl;

namespace snowcrash {

    /**
     * MSON Property Member Section Processor
     */
    template<>
    struct SectionProcessor<mson::PropertyMember> : public SignatureSectionProcessorBase<mson::PropertyMember> {

        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(SignatureTraits::IdentifierTrait |
                                            SignatureTraits::ValuesTrait |
                                            SignatureTraits::AttributesTrait |
                                            SignatureTraits::ContentTrait);

            return signatureTraits;
        }

        static MarkdownNodeIterator finalizeSignature(const MarkdownNodeIterator& node,
                                                      SectionParserData& pd,
                                                      const Signature& signature,
                                                      const ParseResultRef<mson::PropertyMember>& out) {

            mson::parsePropertyName(node, pd, signature.identifier, out.report, out.node.name);

            if (pd.exportSourceMap() && !out.node.name.empty()) {
                out.sourceMap.name.sourceMap = node->sourceMap;
            }

            return SectionProcessor<mson::ValueMember>::useSignatureData(node, pd, signature, out.report, out.node, out.sourceMap);
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       const ParseResultRef<mson::PropertyMember>& out) {

            return SectionProcessor<mson::ValueMember>::blockDescription(node, pd, out.node.sections, out.sourceMap.sections);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<mson::PropertyMember>& out) {

            ParseResultRef<mson::TypeSections> typeSections(out.report, out.node.sections, out.sourceMap.sections);

            return SectionProcessor<mson::ValueMember>
                    ::processNestedMembers<MSONTypeSectionListParser>(node, siblings, pd, typeSections,
                                                                      out.node.valueDefinition.typeDefinition.baseType);
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            return SectionProcessor<mson::ValueMember>::isDescriptionNode(node, sectionType);
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            return SectionProcessor<mson::ValueMember>::nestedSectionType(node);
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<mson::PropertyMember>& out) {

            SectionProcessor<mson::ValueMember>::finalizeImplicitBaseType(out.node.valueDefinition.typeDefinition.baseType);
        }
    };

    /** MSON Property Member Section Parser */
    typedef SectionParser<mson::PropertyMember, ListSectionAdapter> MSONPropertyMemberParser;
}

#endif
