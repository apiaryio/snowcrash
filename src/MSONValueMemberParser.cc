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
        if (!sections.node.empty() && sections.node.back().klass == mson::TypeSection::BlockDescriptionClass) {
            return SectionProcessor<mson::ValueMember>::blockDescription(node, pd, sections.node, sections.sourceMap);
        }

        // Try to resolve base type if not given
        resolveImplicitBaseType(node, pd.sectionContext(), baseType);

        // Build a section to indicate nested members
        if (sections.node.empty() ||
            (!sections.node.empty() && sections.node.back().klass != mson::TypeSection::MemberTypeClass)) {

            mson::TypeSection typeSection(mson::TypeSection::MemberTypeClass);

            typeSection.baseType = baseType;
            sections.node.push_back(typeSection);

            if (pd.exportSourceMap()) {

                SourceMap<mson::TypeSection> typeSectionSM;
                sections.sourceMap.collection.push_back(typeSectionSM);
            }
        }

        mson::Element element;
        SourceMap<mson::Element> elementSM;

        if (pd.sectionContext() == MSONMixinSectionType) {

            IntermediateParseResult<mson::Mixin> mixin(sections.report);
            cur = MSONMixinParser::parse(node, siblings, pd, mixin);

            if (!isSameBaseType(baseType, mixin.node.baseType)) {

                // WARN: Mixin base type should be compatible with the parent base type
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                sections.report.warnings.push_back(Warning("mixin base type should be the same as parent base type. objects should contain object mixins. arrays should contain array mixins",
                                                           LogicalErrorWarning,
                                                           sourceMap));
            }
            else {
                element.build(mixin.node);

                if (pd.exportSourceMap()) {
                    elementSM.mixin = mixin.sourceMap;
                }
            }
        }
        else if (pd.sectionContext() == MSONOneOfSectionType) {

            if (baseType != mson::ObjectBaseType &&
                baseType != mson::ImplicitObjectBaseType) {

                // WARN: One of can not be a nested member for a non object structure type
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                sections.report.warnings.push_back(Warning("one of may be a nested member of a object sub-types only",
                                                           LogicalErrorWarning,
                                                           sourceMap));

                return cur;
            }

            IntermediateParseResult<mson::OneOf> oneOf(sections.report);
            cur = MSONOneOfParser::parse(node, siblings, pd, oneOf);

            element.build(oneOf.node);

            if (pd.exportSourceMap()) {
                elementSM = oneOf.sourceMap;
            }
        }
        else {

            if ((baseType == mson::ValueBaseType ||
                 baseType == mson::ImplicitValueBaseType) &&
                node->type == mdp::ListItemMarkdownNodeType) {

                IntermediateParseResult<mson::ValueMember> valueMember(sections.report);
                cur = MSONValueMemberParser::parse(node, siblings, pd, valueMember);

                element.build(valueMember.node);

                if ((valueMember.node.valueDefinition.typeDefinition.baseType ==  mson::ImplicitObjectBaseType ||
                     valueMember.node.valueDefinition.typeDefinition.baseType ==  mson::ObjectBaseType) &&
                    !valueMember.node.valueDefinition.values.empty()) {
                    // WARN: object definition contain value 
                    // e.g
                    // - a (array)
                    //   - key (object)
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                    sections.report.warnings.push_back(Warning("array member definition of type 'object' contains value. You should use type definition without value eg. '- (object)'",
                                                               LogicalErrorWarning,
                                                               sourceMap));
                }

                if (pd.exportSourceMap()) {
                    elementSM.value = valueMember.sourceMap;
                }
            }
            else if ((baseType == mson::ObjectBaseType ||
                      baseType == mson::ImplicitObjectBaseType) &&
                     node->type == mdp::ListItemMarkdownNodeType) {

                IntermediateParseResult<mson::PropertyMember> propertyMember(sections.report);
                cur = MSONPropertyMemberParser::parse(node, siblings, pd, propertyMember);

                element.build(propertyMember.node);

                if ((propertyMember.node.valueDefinition.typeDefinition.baseType ==  mson::ImplicitObjectBaseType ||
                     propertyMember.node.valueDefinition.typeDefinition.baseType ==  mson::ObjectBaseType) &&
                    !propertyMember.node.valueDefinition.values.empty()) {
                    // WARN: object definition contain value 
                    // e.g
                    // - key: value (object)
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                    sections.report.warnings.push_back(Warning("'object' with value definition. You should use type definition without value eg. '- key (object)'",
                                                               LogicalErrorWarning,
                                                               sourceMap));
                }

                if (pd.exportSourceMap()) {
                    elementSM.property = propertyMember.sourceMap;
                }
            }
            else if (baseType == mson::PrimitiveBaseType ||
                     baseType == mson::ImplicitPrimitiveBaseType) {

                // WARN: Primitive type members should not have nested members
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                sections.report.warnings.push_back(Warning("sub-types of primitive types should not have nested members",
                                                           LogicalErrorWarning,
                                                           sourceMap));
            }
            else {

                // WARN: Ignoring unrecognized block in mson nested members
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                sections.report.warnings.push_back(Warning("ignorning unrecognized block",
                                                           IgnoringWarning,
                                                           sourceMap));

                cur = ++MarkdownNodeIterator(node);
            }
        }

        if (element.klass != mson::Element::UndefinedClass) {
            sections.node.back().content.elements().push_back(element);

            if (pd.exportSourceMap()) {
                sections.sourceMap.collection.back().elements().collection.push_back(elementSM);
            }
        }

        return cur;
    }
}
