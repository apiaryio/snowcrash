//
//  MSONValueMemberParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/13/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "MSONOneOfParser.h"
#include "MSONTypeSectionParser.h"
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

    /** Implementation of processNestedMembers */
    MarkdownNodeIterator SectionProcessor<mson::ValueMember>::processNestedMembers(const MarkdownNodeIterator& node,
                                                                                   const MarkdownNodes& siblings,
                                                                                   SectionParserData& pd,
                                                                                   Report& report,
                                                                                   mson::TypeSections& sections,
                                                                                   SourceMap<mson::TypeSections>& sourceMap,
                                                                                   mson::BaseType& baseType) {

        if (pd.sectionContext() == MSONSectionType) {

            if (node->type == mdp::ListItemMarkdownNodeType &&
                (sections.empty() ||
                 (sections.size() == 1 &&
                  sections[0].type == mson::MemberTypeSectionType))) {

                // Build a section to indicate nested members
                if (sections.empty()) {

                    mson::TypeSection typeSection(mson::MemberTypeSectionType);
                    sections.push_back(typeSection);
                }

                MarkdownNodeIterator cur = node;
                mson::MemberType memberType;

                switch (baseType) {
                    case mson::PrimitiveBaseType:
                    {
                        //WARN: Primitive type members should not have nested members
                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        report.warnings.push_back(Warning("primitive base type members should not have nested members",
                                                          LogicalErrorWarning,
                                                          sourceMap));
                        break;
                    }

                    case mson::PropertyBaseType:
                    {
                        IntermediateParseResult<mson::PropertyMember> propertyMember(report);
                        cur = MSONPropertyMemberParser::parse(node, siblings, pd, propertyMember);

                        memberType.build(propertyMember.node);
                        break;
                    }

                    case mson::ValueBaseType:
                    {
                        IntermediateParseResult<mson::ValueMember> valueMember(report);
                        cur = MSONValueMemberParser::parse(node, siblings, pd, valueMember);

                        memberType.build(valueMember.node);
                        break;
                    }

                    default:
                        break;
                }

                if (memberType.type != mson::UndefinedMemberType) {
                    sections[0].content.members().push_back(memberType);
                }

                return cur;
            }

            SectionProcessor<mson::ValueMember>::blockDescription(node, pd, sections, sourceMap);
        }
        else {
            
            IntermediateParseResult<mson::TypeSection> typeSection(report);
            typeSection.node.baseType = baseType;
            
            MSONTypeSectionListParser::parse(node, siblings, pd, typeSection);
            
            if (typeSection.node.type != mson::UndefinedTypeSectionType) {
                sections.push_back(typeSection.node);
            }
        }
        
        return ++MarkdownNodeIterator(node);
    }
}
