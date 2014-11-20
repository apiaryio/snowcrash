//
//  MSONValueMemberParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/13/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "MSONPropertyMemberParser.h"
#include "MSONTypeSectionParser.h"
#include "MSONValueMemberParser.h"

namespace snowcrash {

    /** Implementation of processNestedSection */
    MarkdownNodeIterator SectionProcessor<mson::ValueMember>::processNestedSection(const MarkdownNodeIterator& node,
                                                                                   const MarkdownNodes& siblings,
                                                                                   SectionParserData& pd,
                                                                                   const ParseResultRef<mson::ValueMember>& out) {

        if (pd.sectionContext() == MSONSectionType) {

            if (node->type == mdp::ListItemMarkdownNodeType &&
                (out.node.sections.empty() ||
                 (out.node.sections.size() == 1 &&
                  out.node.sections[0].type == mson::MemberTypeSectionType))) {

                // Build a section to indicate nested members
                if (out.node.sections.empty()) {

                    mson::TypeSection typeSection(mson::MemberTypeSectionType);
                    out.node.sections.push_back(typeSection);
                }

                MarkdownNodeIterator cur = node;
                mson::MemberType memberType;

                switch (out.node.valueDefinition.typeDefinition.baseType) {
                    case mson::PrimitiveBaseType:
                    {
                        //WARN: Primitive type members should not have nested members
                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        out.report.warnings.push_back(Warning("primitive base type members should not have nested members",
                                                              LogicalErrorWarning,
                                                              sourceMap));
                        break;
                    }

                    case mson::PropertyBaseType:
                    {
                        IntermediateParseResult<mson::PropertyMember> propertyMember(out.report);
                        cur = MSONPropertyMemberParser::parse(node, siblings, pd, propertyMember);

                        memberType.build(propertyMember.node);
                        break;
                    }

                    case mson::ValueBaseType:
                    {
                        IntermediateParseResult<mson::ValueMember> valueMember(out.report);
                        cur = MSONValueMemberParser::parse(node, siblings, pd, valueMember);

                        memberType.build(valueMember.node);
                        break;
                    }

                    default:
                        break;
                }

                if (memberType.type != mson::UndefinedMemberType) {
                    out.node.sections[0].content.members().push_back(memberType);
                }

                return cur;
            }

            SectionProcessor<mson::ValueMember>::processDescription(node, siblings, pd, out);
        }
        else {

            IntermediateParseResult<mson::TypeSection> typeSection(out.report);
            typeSection.node.baseType = out.node.valueDefinition.typeDefinition.baseType;

            MSONTypeSectionListParser::parse(node, siblings, pd, typeSection);

            out.node.sections.push_back(typeSection.node);
        }

        return ++MarkdownNodeIterator(node);
    }

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
}
