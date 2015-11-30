//
//  MSONOneOfParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONONEOFPARSER_H
#define SNOWCRASH_MSONONEOFPARSER_H

#include "MSONMixinParser.h"

using namespace scpl;

namespace snowcrash {

    /** MSON One Of matching regex */
    const char* const MSONOneOfRegex = "^[[:blank:]]*[Oo]ne[[:blank:]]+[Oo]f[[:blank:]]*$";

    /**
     * MSON One Of Section Processor
     */
    template<>
    struct SectionProcessor<mson::OneOf> : public SignatureSectionProcessorBase<mson::OneOf> {

        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(SignatureTraits::IdentifierTrait);

            return signatureTraits;
        }

        NO_SECTION_DESCRIPTION(mson::OneOf)

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator&,
                                                         const MarkdownNodes&,
                                                         SectionParserData&,
                                                         const ParseResultRef<mson::OneOf>&);

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<mson::OneOf>& out) {

            if (out.node.empty()) {

                // WARN: one of type do not have nested members
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning("one of type must have nested members",
                                                      EmptyDefinitionWarning,
                                                      sourceMap));
            }
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

        static SectionType nestedSectionType(const MarkdownNodeIterator&);
    };

    /** MSON One Of Section Parser */
    typedef SectionParser<mson::OneOf, ListSectionAdapter> MSONOneOfParser;
}

#endif
