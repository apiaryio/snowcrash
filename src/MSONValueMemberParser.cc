//
//  MSONValueMemberParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/13/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "MSONOneOfParser.h"
#include "MSONPropertyMemberParser.h"

namespace snowcrash {

    /** Implementation of nestedSectionType */
    SectionType SectionProcessor<mson::ValueMember>::nestedSectionType(const MarkdownNodeIterator& node) {

        SectionType nestedType = UndefinedSectionType;

        // Check if mson type section section
        nestedType = SectionProcessor<mson::TypeSection>::sectionType(node);

        if (nestedType != UndefinedSectionType) {
            return nestedType;
        }

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

    /** Implementation of resolveAndProcessMSONSection */
    MarkdownNodeIterator SectionProcessor<mson::ValueMember>::processMSONSection(const MarkdownNodeIterator& node,
                                                                                 const MarkdownNodes& siblings,
                                                                                 SectionParserData& pd,
                                                                                 const ParseResultRef<mson::TypeSections>& sections,
                                                                                 mson::BaseType& baseType) {

        MarkdownNodeIterator cur = node;

        // If we encounter a header node, stop parsing. Headers which are not
        // type section signatures are ineligible to be nested sections of a MSON section
        if (node->type == mdp::HeaderMarkdownNodeType) {
            return cur;
        }

        // If the nodes follow after some block description without member
        // seperator, then they are treated as description
        if (!sections.node.empty() && sections.node.back().type == mson::TypeSection::BlockDescriptionType) {
            return SectionProcessor<mson::ValueMember>::blockDescription(node, pd, sections.node, sections.sourceMap);
        }

        // Try to resolve base type if not given
        resolveImplicitBaseType(node, pd.sectionContext(), baseType);

        // Build a section to indicate nested members
        if (sections.node.empty() ||
            (!sections.node.empty() && sections.node.back().type != mson::TypeSection::MemberType)) {

            mson::TypeSection typeSection(mson::TypeSection::MemberType);
            typeSection.baseType = baseType;

            sections.node.push_back(typeSection);
        }

        mson::MemberType memberType;

        if (pd.sectionContext() == MSONMixinSectionType) {

            IntermediateParseResult<mson::Mixin> mixin(sections.report);
            cur = MSONMixinParser::parse(node, siblings, pd, mixin);

            memberType.build(mixin.node);
        }
        else if (pd.sectionContext() == MSONOneOfSectionType) {

            if (baseType != mson::ObjectBaseType &&
                baseType != mson::ImplicitObjectBaseType) {

                // WARN: One of can not be a nested member for a non object structure type
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                sections.report.warnings.push_back(Warning("one of may be a nested member of a object sub-types only",
                                                           LogicalErrorWarning,
                                                           sourceMap));

                return cur;
            }

            IntermediateParseResult<mson::OneOf> oneOf(sections.report);
            cur = MSONOneOfParser::parse(node, siblings, pd, oneOf);

            memberType.build(oneOf.node);
        }
        else {

            if (baseType == mson::ValueBaseType &&
                node->type == mdp::ListItemMarkdownNodeType) {

                IntermediateParseResult<mson::ValueMember> valueMember(sections.report);
                cur = MSONValueMemberParser::parse(node, siblings, pd, valueMember);

                memberType.build(valueMember.node);
            }
            else if ((baseType == mson::ObjectBaseType ||
                      baseType == mson::ImplicitObjectBaseType) &&
                     node->type == mdp::ListItemMarkdownNodeType) {

                IntermediateParseResult<mson::PropertyMember> propertyMember(sections.report);
                cur = MSONPropertyMemberParser::parse(node, siblings, pd, propertyMember);

                memberType.build(propertyMember.node);
            }
            else if (baseType == mson::PrimitiveBaseType ||
                     baseType == mson::ImplicitPrimitiveBaseType) {

                // WARN: Primitive type members should not have nested members
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                sections.report.warnings.push_back(Warning("sub-types of primitive types should not have nested members",
                                                           LogicalErrorWarning,
                                                           sourceMap));
            }
            else {

                // WARN: Ignoring unrecognized block in mson nested members
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                sections.report.warnings.push_back(Warning("ignorning unrecognized block",
                                                           IgnoringWarning,
                                                           sourceMap));
                
                cur = ++MarkdownNodeIterator(node);
            }
        }

        if (memberType.type != mson::MemberType::UndefinedType) {
            sections.node.back().content.members().push_back(memberType);
        }

        return cur;
    }
}
