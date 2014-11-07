//
//  MSONPropertyMemberParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONPROPERTYMEMBERPARSER_H
#define SNOWCRASH_MSONPROPERTYMEMBERPARSER_H

#include "SectionParser.h"
#include "MSONUtility.h"
#include "MSONValueMemberParser.h"

using namespace scpl;

namespace snowcrash {

    /**
     * MSON Property Member Section Processor
     */
    template<>
    struct SectionProcessor<mson::PropertyMember> : public SectionProcessor<mson::ValueMember> {

        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(SignatureTraits::IdentifierTrait |
                                            SignatureTraits::ValuesTrait |
                                            SignatureTraits::AttributesTrait |
                                            SignatureTraits::ContentTrait);

            return signatureTraits;
        }

        static void finalizeSignature(const MarkdownNodeIterator& node,
                                      SectionParserData& pd,
                                      const Signature& signature,
                                      const ParseResultRef<mson::PropertyMember>& out) {

        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<mson::PropertyMember>& out) {

            if ((pd.sectionContext() != MSONTypeSectionSectionType) &&
                (pd.sectionContext() != MSONMemberTypeGroupSectionType)) {

                return node;
            }

            IntermediateParseResult<mson::TypeSection> typeSection(out.report);

            MSONTypeSectionListParser::parse(node, siblings, pd, typeSection);

            out.node.sections.push_back(typeSection.node);

            return ++MarkdownNodeIterator(node);
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            SectionType nestedType = UndefinedSectionType;

            // Check if mson type section section
            nestedType = SectionProcessor<mson::TypeSection>::sectionType(node);

            return nestedType;
        }
    };
    
    /** MSON Property Member Section Parser */
    typedef SectionParser<mson::PropertyMember, ListSectionAdapter> MSONPropertyMemberParser;
}

#endif
