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

        return MSONSectionType;
    }

    /** Implementation of resolveAndProcessMSONSection */
    MarkdownNodeIterator SectionProcessor<mson::ValueMember>::processMSONSection(const MarkdownNodeIterator& node,
                                                                                 const MarkdownNodes& siblings,
                                                                                 SectionParserData& pd,
                                                                                 const ParseResultRef<mson::TypeSections>& sections,
                                                                                 mson::BaseType& baseType) {

        MarkdownNodeIterator cur = node;

        // If the nodes follow after some block description without member
        // seperator, then they are treated as description
        if (!sections.node.empty() && sections.node.back().type == mson::BlockDescriptionTypeSectionType) {
            return SectionProcessor<mson::ValueMember>::blockDescription(node, pd, sections.node, sections.sourceMap);
        }

        // Try to resolve base type if not given
        resolveImplicitBaseType(node, pd.sectionContext(), baseType);

        // Build a section to indicate nested members
        if (sections.node.empty() ||
            (!sections.node.empty() && sections.node.back().type != mson::MemberTypeSectionType)) {

            mson::TypeSection typeSection(mson::MemberTypeSectionType);
            typeSection.baseType = baseType;

            sections.node.push_back(typeSection);
        }

        mson::MemberType memberType;

        if (baseType == mson::ValueBaseType &&
            node->type == mdp::ListItemMarkdownNodeType) {

            IntermediateParseResult<mson::ValueMember> valueMember(sections.report);
            cur = MSONValueMemberParser::parse(node, siblings, pd, valueMember);

            memberType.build(valueMember.node);
        }
        else if ((baseType == mson::PropertyBaseType ||
                  baseType == mson::ImplicitPropertyBaseType) &&
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

        if (memberType.type != mson::UndefinedMemberType) {
            sections.node.back().content.members().push_back(memberType);
        }

        return cur;
    }
}
