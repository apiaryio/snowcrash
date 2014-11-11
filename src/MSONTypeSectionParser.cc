//
//  MSONTypeSectionParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/12/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "MSONTypeSectionParser.h"
#include "MSONOneOfParser.h"

namespace snowcrash {

    /** Implementation of processNestedSection */
    MarkdownNodeIterator SectionProcessor<mson::TypeSection>::processNestedSection(const MarkdownNodeIterator& node,
                                                                                   const MarkdownNodes& siblings,
                                                                                   SectionParserData& pd,
                                                                                   const ParseResultRef<mson::TypeSection>& out) {

        MarkdownNodeIterator cur = node;

        switch (pd.sectionContext()) {
            case MSONMixinSectionType:
            {
                IntermediateParseResult<mson::Mixin> mixin(out.report);
                cur = MSONMixinParser::parse(node, siblings, pd, mixin);

                mson::MemberType memberType;
                mson::buildMemberType(mixin.node, memberType);

                out.node.content.members().push_back(memberType);
                break;
            }

            case MSONOneOfSectionType:
            {
                IntermediateParseResult<mson::OneOf> oneOf(out.report);
                cur = MSONOneOfParser::parse(node, siblings, pd, oneOf);

                mson::MemberType memberType;
                mson::buildMemberType(oneOf.node, memberType);

                out.node.content.members().push_back(memberType);
                break;
            }

            default:
                break;
        }

        return cur;
    }

    /** Implementation of nestedSectionType */
    SectionType SectionProcessor<mson::TypeSection>::nestedSectionType(const MarkdownNodeIterator& node) {

        SectionType nestedType = UndefinedSectionType;

        // Check if mson mixin section
        nestedType = SectionProcessor<mson::Mixin>::sectionType(node);

        if (nestedType != MSONMixinSectionType) {
            return nestedType;
        }

        // Check if mson one of section
        nestedType = SectionProcessor<mson::OneOf>::sectionType(node);

        if (nestedType != MSONOneOfSectionType) {
            return nestedType;
        }

        return UndefinedSectionType;
    }
}