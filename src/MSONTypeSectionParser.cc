//
//  MSONTypeSectionParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/12/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "MSONTypeSectionParser.h"
#include "MSONOneOfParser.h"
#include "MSONPropertyMemberParser.h"
#include "MSONValueMemberParser.h"

namespace snowcrash {

    /** Implementation of processNestedSection */
    MarkdownNodeIterator SectionProcessor<mson::TypeSection>::processNestedSection(const MarkdownNodeIterator& node,
                                                                                   const MarkdownNodes& siblings,
                                                                                   SectionParserData& pd,
                                                                                   const ParseResultRef<mson::TypeSection>& out) {

        MarkdownNodeIterator cur = node;
        SectionType parentSectionType = pd.parentSectionContext();

        if (parentSectionType == MSONPropertyMembersSectionType ||
            parentSectionType == MSONValueMembersSectionType) {

            mson::MemberType memberType;

            switch (pd.sectionContext()) {
                case MSONMixinSectionType:
                {
                    IntermediateParseResult<mson::Mixin> mixin(out.report);
                    cur = MSONMixinParser::parse(node, siblings, pd, mixin);

                    mson::buildMemberType(mixin.node, memberType);
                    break;
                }

                case MSONOneOfSectionType:
                {
                    if (parentSectionType != MSONPropertyMembersSectionType) {
                        break;
                    }

                    IntermediateParseResult<mson::OneOf> oneOf(out.report);
                    cur = MSONOneOfParser::parse(node, siblings, pd, oneOf);

                    mson::buildMemberType(oneOf.node, memberType);
                    break;
                }

                case MSONSectionType:
                {
                    if (parentSectionType == MSONPropertyMembersSectionType) {

                        IntermediateParseResult<mson::PropertyMember> propertyMember(out.report);
                        cur = MSONPropertyMemberParser::parse(node, siblings, pd, propertyMember);

                        mson::buildMemberType(propertyMember.node, memberType);
                    }
                    else {

                        IntermediateParseResult<mson::ValueMember> valueMember(out.report);
                        cur = MSONValueMemberParser::parse(node, siblings, pd, valueMember);

                        mson::buildMemberType(valueMember.node, memberType);
                    }

                    break;
                }

                default:
                    break;
            }

            if (memberType.type != mson::UndefinedMemberType) {
                out.node.content.members().push_back(memberType);
            }
        }
        else if (parentSectionType == MSONTypeSectionSectionType) {

            // TODO: Build sample/default type sections
        }

        return cur;
    }

    /** Implementation of nestedSectionType */
    SectionType SectionProcessor<mson::TypeSection>::nestedSectionType(const MarkdownNodeIterator& node) {

        SectionType nestedType = UndefinedSectionType;

        // Check if mson mixin section
        nestedType = SectionProcessor<mson::Mixin>::sectionType(node);

        if (nestedType != UndefinedSectionType) {
            return nestedType;
        }

        // Check if mson one of section
        nestedType = SectionProcessor<mson::OneOf>::sectionType(node);

        if (nestedType != UndefinedSectionType) {
            return nestedType;
        }

        return MSONSectionType;
    }
}