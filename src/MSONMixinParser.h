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

        static void finalizeSignature(const MarkdownNodeIterator& node,
                                      SectionParserData& pd,
                                      const Signature& signature,
                                      const ParseResultRef<mson::Mixin>& out) {

            CaptureGroups captureGroups;
            ParseResultRef<mson::TypeDefinition> typeDefinition(out.report,
                                                                out.node.typeDefinition,
                                                                out.sourceMap.typeDefinition);

            if (signature.identifier == "Include") {
                mson::parseTypeDefinition(node, pd, signature.attributes, typeDefinition);
            }
            else if (RegexCapture(signature.identifier, MSONMixinRegex, captureGroups, 2) &&
                     !captureGroups[1].empty()) {

                // Get the type name
                std::string typeName = signature.identifier.substr(captureGroups[1].length());

                out.node.typeDefinition.typeSpecification.name.symbol = mson::parseSymbol(typeName);
            }

            if ((out.node.typeDefinition.typeSpecification.name.name == mson::StringTypeName) ||
                (out.node.typeDefinition.typeSpecification.name.name == mson::NumberTypeName) ||
                (out.node.typeDefinition.typeSpecification.name.name == mson::BooleanTypeName)) {

                // WARN: invalid mixin base type
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.warnings.push_back(Warning("mixin type cannot use a non-structure base type",
                                                      FormattingWarning,
                                                      sourceMap));
            }
        }

        NO_DESCRIPTION(mson::Mixin)

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
