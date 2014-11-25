//
//  AttributesParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/25/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_ATTRIBUTESPARSER_H
#define SNOWCRASH_ATTRIBUTESPARSER_H

#include "SectionParser.h"
#include "RegexMatch.h"

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

        NO_SECTION_DESCRIPTION(Attributes)

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
    };

    /** Attributes Section Parser */
    typedef SectionParser<Attributes, ListSectionAdapter> AttributesParser;
}

#endif
