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

                mson::TypeSection typeSection;

                typeSection.type = mson::BlockDescriptionTypeSectionType;
                typeSection.content.description = signature.remainingContent;

                out.node.sections.push_back(typeSection);
            }
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       const ParseResultRef<mson::ValueMember>& out) {

            return node;
        }
    };

    /** MSON Value Member Section Parser */
    typedef SectionParser<mson::ValueMember, ListSectionAdapter> MSONValueMemberParser;
}

#endif
