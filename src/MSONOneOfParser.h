//
//  MSONOneOfParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONONEOFPARSER_H
#define SNOWCRASH_MSONONEOFPARSER_H

#include "SectionParser.h"
#include "MSONUtility.h"

using namespace scpl;

namespace snowcrash {

    /** MSON One Of matching regex */
    const char* const MSONOneOfRegex = "^[[:blank:]]*[Oo]ne [Oo]f[[:blank:]]*$";

    /**
     * MSON One Of Section Processor
     */
    template<>
    struct SectionProcessor<mson::OneOf> : public SignatureSectionProcessorBase<mson::OneOf> {

        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(SignatureTraits::IdentifierTrait);

            return signatureTraits;
        }

        static void finalizeSignature(const MarkdownNodeIterator& node,
                                      SectionParserData& pd,
                                      const Signature& signature,
                                      const ParseResultRef<mson::OneOf>& out) {

        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<mson::OneOf>& out) {

            return node;
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {

                mdp::ByteBuffer remaining, subject = node->children().front().text;

                subject = GetFirstLine(subject, remaining);
                TrimString(subject);

                if (RegexMatch(subject, MSONOneOfRegex)) {
                    return MSONOneOfSectionType;
                }
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            return UndefinedSectionType;
        }
    };

    /** MSON One Of Section Parser */
    typedef SectionParser<mson::OneOf, ListSectionAdapter> MSONOneOfParser;
}

#endif
