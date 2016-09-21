//
//  AttributesParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/25/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_ATTRIBUTESPARSER_H
#define SNOWCRASH_ATTRIBUTESPARSER_H

#include "RegexMatch.h"
#include "MSONValueMemberParser.h"

using namespace scpl;

namespace snowcrash {

    /** Attributes matching regex */
    const char* const AttributesRegex = "^[[:blank:]]*[Aa]ttributes?[[:blank:]]*(\\(.*\\))?$";

    /**
     * Attributes section processor
     */
    template<>
    struct SectionProcessor<Attributes> : public SignatureSectionProcessorBase<Attributes> {

        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(SignatureTraits::IdentifierTrait |
                                            SignatureTraits::AttributesTrait);

            return signatureTraits;
        }

        static MarkdownNodeIterator finalizeSignature(const MarkdownNodeIterator& node,
                                                      SectionParserData& pd,
                                                      const Signature& signature,
                                                      const ParseResultRef<Attributes>& out) {

            if (!IEqual<std::string>()(signature.identifier, "Attribute") &&
                !IEqual<std::string>()(signature.identifier, "Attributes")) {

                return node;
            }

            mson::parseTypeDefinition(node, pd, signature.attributes, out.report, out.node.typeDefinition);

            if (pd.exportSourceMap()) {

                if (!out.node.typeDefinition.empty()) {
                    out.sourceMap.typeDefinition.sourceMap = node->sourceMap;
                }
                
                out.sourceMap.sourceMap = node->sourceMap;
            }

            // Default to `object` type specification
            if (out.node.typeDefinition.baseType == mson::UndefinedBaseType) {
                out.node.typeDefinition.baseType = mson::ImplicitObjectBaseType;
            }

            SectionProcessor<mson::ValueMember>::parseRemainingContent(node, pd, signature.remainingContent,
                                                                       out.node.sections, out.sourceMap.sections);

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       const ParseResultRef<Attributes>& out) {

            return SectionProcessor<mson::ValueMember>::blockDescription(node, pd, out.node.sections, out.sourceMap.sections);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<Attributes>& out) {

            ParseResultRef<mson::TypeSections> typeSections(out.report, out.node.sections, out.sourceMap.sections);

            return SectionProcessor<mson::ValueMember>
                    ::processNestedMembers<MSONTypeSectionListParser>(node, siblings, pd, typeSections,
                                                                      out.node.typeDefinition.baseType);
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            return SectionProcessor<mson::ValueMember>::isDescriptionNode(node, sectionType);
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::ListItemMarkdownNodeType &&
                !node->children().empty()) {

                mdp::ByteBuffer remaining, subject = node->children().front().text;

                subject = GetFirstLine(subject, remaining);
                TrimString(subject);

                if (RegexMatch(subject, AttributesRegex)) {
                    return AttributesSectionType;
                }
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            return SectionProcessor<mson::ValueMember>::nestedSectionType(node);
        }
    };

    /** Attributes Section Parser */
    typedef SectionParser<Attributes, ListSectionAdapter> AttributesParser;
}

#endif
