//
//  MSONNamedTypeParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONNAMEDTYPEPARSER_H
#define SNOWCRASH_MSONNAMEDTYPEPARSER_H

#include "SectionParser.h"
#include "MSONUtility.h"
#include "MSONTypeSectionParser.h"

using namespace scpl;

namespace snowcrash {

    /**
     * MSON Named Type Section Processor
     */
    template<>
    struct SectionProcessor<mson::NamedType> : public SignatureSectionProcessorBase<mson::NamedType> {

        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(SignatureTraits::IdentifierTrait |
                                            SignatureTraits::AttributesTrait);

            return signatureTraits;
        }

        static void finalizeSignature(const MarkdownNodeIterator& node,
                                      SectionParserData& pd,
                                      const Signature& signature,
                                      const ParseResultRef<mson::NamedType>& out) {

        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<mson::NamedType>& out) {

            if ((pd.sectionContext() != MSONTypeSectionSectionType) &&
                (pd.sectionContext() != MSONMemberTypeGroupSectionType)) {

                return node;
            }

            IntermediateParseResult<mson::TypeSection> typeSection(out.report);

            MSONTypeSectionHeaderParser::parse(node, siblings, pd, typeSection);

            out.node.sections.push_back(typeSection.node);

            return ++MarkdownNodeIterator(node);
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            SectionType nestedType = UndefinedSectionType;

            // Check if mson type section section
            nestedType = SectionProcessor<mson::TypeSection>::sectionType(node);

            return nestedType;
        }
    };
    
    /** MSON Named Type Section Parser */
    typedef SectionParser<mson::NamedType, HeaderSectionAdapter> MSONNamedTypeParser;
}

#endif
