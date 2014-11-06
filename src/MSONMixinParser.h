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
    const char* const MSONMixinRegex = "^[[:blank:]]*[Ii]nclude[[:blank:]]+";

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

        }

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

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            return UndefinedSectionType;
        }
    };

    /** MSON Mixin Section Parser */
    typedef SectionParser<mson::Mixin, ListSectionAdapter> MSONMixinParser;
}

#endif
