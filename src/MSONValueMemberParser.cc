//
//  MSONValueMemberParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/13/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "MSONValueMemberParser.h"
#include "MSONTypeSectionParser.h"

namespace snowcrash {

    /** Implementation of processNestedSection */
    MarkdownNodeIterator SectionProcessor<mson::ValueMember>::processNestedSection(const MarkdownNodeIterator& node,
                                                                                   const MarkdownNodes& siblings,
                                                                                   SectionParserData& pd,
                                                                                   const ParseResultRef<mson::ValueMember>& out) {

        if ((pd.sectionContext() != MSONTypeSectionSectionType) &&
            (pd.sectionContext() != MSONPropertyMembersSectionType) &&
            (pd.sectionContext() != MSONValueMembersSectionType)) {

            return node;
        }

        IntermediateParseResult<mson::TypeSection> typeSection(out.report);

        MSONTypeSectionListParser::parse(node, siblings, pd, typeSection);

        out.node.sections.push_back(typeSection.node);

        return ++MarkdownNodeIterator(node);
    }

    /** Implementation of nestedSectionType */
    SectionType SectionProcessor<mson::ValueMember>::nestedSectionType(const MarkdownNodeIterator& node) {

        SectionType nestedType = UndefinedSectionType;

        // Check if mson type section section
        nestedType = SectionProcessor<mson::TypeSection>::sectionType(node);
        
        return nestedType;
    }
}