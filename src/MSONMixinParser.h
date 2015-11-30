//
//  MSONMixinParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONMIXINPARSER_H
#define SNOWCRASH_MSONMIXINPARSER_H

#include "SectionParser.h"
#include "MSONUtility.h"

using namespace scpl;

namespace snowcrash {

    /** MSON Mixin matching regex */
    const char* const MSONMixinRegex = "^[[:blank:]]*([Ii]nclude[[:blank:]]+)";

    /**
     * MSON Mixin Section Processor
     */
    template<>
    struct SectionProcessor<mson::Mixin> : public SignatureSectionProcessorBase<mson::Mixin> {

        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(SignatureTraits::IdentifierTrait |
                                            SignatureTraits::AttributesTrait);

            return signatureTraits;
        }

        static MarkdownNodeIterator finalizeSignature(const MarkdownNodeIterator& node,
                                                      SectionParserData& pd,
                                                      const Signature& signature,
                                                      const ParseResultRef<mson::Mixin>& out) {

            CaptureGroups captureGroups;
            std::vector<mdp::ByteBuffer> attributes = signature.attributes;

            if (RegexCapture(signature.identifier, MSONMixinRegex, captureGroups, 2) &&
                !captureGroups[1].empty()) {

                // Get the type name and insert it into attributes
                std::string typeName = signature.identifier.substr(captureGroups[1].length());
                attributes.insert(attributes.begin(), typeName);
            }

            mson::parseTypeDefinition(node, pd, attributes, out.report, out.node);

            if (pd.exportSourceMap()) {
                out.sourceMap.sourceMap = node->sourceMap;
            }

            if ((out.node.baseType == mson::PrimitiveBaseType) ||
                (out.node.baseType == mson::UndefinedBaseType)) {

                // WARN: invalid mixin base type
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning("mixin type may not include a type of a primitive sub-type",
                                                      FormattingWarning,
                                                      sourceMap));
            }

            // Check circular references
            if (out.node.typeSpecification.name.base == mson::UndefinedTypeName &&
                !out.node.typeSpecification.name.symbol.literal.empty() &&
                !out.node.typeSpecification.name.symbol.variable) {

                mson::addDependency(node, pd, out.node.typeSpecification.name.symbol.literal, pd.namedTypeContext, out.report);
            }

            return ++MarkdownNodeIterator(node);
        }

        NO_SECTION_DESCRIPTION(mson::Mixin)

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {

                mdp::ByteBuffer subject = node->children().front().text;

                TrimString(subject);

                if (RegexMatch(subject, MSONMixinRegex)) {
                    return MSONMixinSectionType;
                }
            }

            return UndefinedSectionType;
        }
    };

    /** MSON Mixin Section Parser */
    typedef SectionParser<mson::Mixin, ListSectionAdapter> MSONMixinParser;
}

#endif
