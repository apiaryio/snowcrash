//
//  MSONTypeSectionParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/12/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "MSONOneOfParser.h"
#include "MSONPropertyMemberParser.h"

namespace snowcrash {

    /** Implementation of processNestedSection */
    MarkdownNodeIterator SectionProcessor<mson::TypeSection>::processNestedSection(const MarkdownNodeIterator& node,
                                                                                   const MarkdownNodes& siblings,
                                                                                   SectionParserData& pd,
                                                                                   const ParseResultRef<mson::TypeSection>& out) {

        MarkdownNodeIterator cur = node;
        SectionType parentSectionType = pd.parentSectionContext();

        mson::Element element;
        SourceMap<mson::Element> elementSM;

        if (node->type == mdp::HeaderMarkdownNodeType) {
            return cur;
        }

        if (parentSectionType == MSONPropertyMembersSectionType ||
            parentSectionType == MSONValueMembersSectionType) {

            switch (pd.sectionContext()) {
                case MSONMixinSectionType:
                {
                    IntermediateParseResult<mson::Mixin> mixin(out.report);
                    cur = MSONMixinParser::parse(node, siblings, pd, mixin);

                    element.build(mixin.node);

                    if (pd.exportSourceMap()) {
                        elementSM.mixin = mixin.sourceMap;
                    }

                    break;
                }

                case MSONOneOfSectionType:
                {
                    if (parentSectionType != MSONPropertyMembersSectionType) {

                        // WARN: One of can not be a nested member for a non object structure type
                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                        out.report.warnings.push_back(Warning("one-of can not be a nested member for a type not sub typed from object",
                                                              LogicalErrorWarning,
                                                              sourceMap));

                        return cur;
                    }

                    IntermediateParseResult<mson::OneOf> oneOf(out.report);
                    cur = MSONOneOfParser::parse(node, siblings, pd, oneOf);

                    element.build(oneOf.node);

                    if (pd.exportSourceMap()) {
                        elementSM = oneOf.sourceMap;
                    }

                    break;
                }

                case MSONSectionType:
                {
                    if (parentSectionType == MSONPropertyMembersSectionType) {

                        IntermediateParseResult<mson::PropertyMember> propertyMember(out.report);
                        cur = MSONPropertyMemberParser::parse(node, siblings, pd, propertyMember);

                        element.build(propertyMember.node);

                        if (pd.exportSourceMap()) {
                            elementSM.property = propertyMember.sourceMap;
                        }
                    }
                    else {

                        IntermediateParseResult<mson::ValueMember> valueMember(out.report);
                        cur = MSONValueMemberParser::parse(node, siblings, pd, valueMember);

                        element.build(valueMember.node);

                        if (pd.exportSourceMap()) {
                            elementSM.value = valueMember.sourceMap;
                        }
                    }

                    break;
                }

                default:
                    break;
            }
        }
        else if (parentSectionType == MSONSampleDefaultSectionType) {

            switch (pd.sectionContext()) {
                case MSONMixinSectionType:
                case MSONOneOfSectionType:
                {
                    // WARN: mixin and oneOf not supported in sample/default
                    std::stringstream ss;

                    ss << "sample and default type sections cannot have `" << SectionName(pd.sectionContext()) << "` type";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          LogicalErrorWarning,
                                                          sourceMap));
                    break;
                }

                case MSONSectionType:
                {
                    if ((out.node.baseType == mson::ValueBaseType ||
                         out.node.baseType == mson::ImplicitValueBaseType) &&
                        node->type == mdp::ListItemMarkdownNodeType) {

                        IntermediateParseResult<mson::ValueMember> valueMember(out.report);
                        cur = MSONValueMemberParser::parse(node, siblings, pd, valueMember);

                        element.build(valueMember.node);

                        if (pd.exportSourceMap()) {
                            elementSM.value = valueMember.sourceMap;
                        }
                    }
                    else if ((out.node.baseType == mson::ObjectBaseType ||
                              out.node.baseType == mson::ImplicitObjectBaseType) &&
                             node->type == mdp::ListItemMarkdownNodeType) {

                        IntermediateParseResult<mson::PropertyMember> propertyMember(out.report);
                        cur = MSONPropertyMemberParser::parse(node, siblings, pd, propertyMember);

                        element.build(propertyMember.node);

                        if (pd.exportSourceMap()) {
                            elementSM.property = propertyMember.sourceMap;
                        }
                    }

                    if (out.node.baseType == mson::PrimitiveBaseType ||
                        out.node.baseType == mson::ImplicitPrimitiveBaseType) {

                        if (!out.node.content.value.empty()) {
                            TwoNewLines(out.node.content.value);
                        }

                        mdp::ByteBuffer content = mdp::MapBytesRangeSet(node->sourceMap, pd.sourceData);
                        out.node.content.value += content;

                        if (pd.exportSourceMap() && !content.empty()) {
                            out.sourceMap.value.sourceMap.append(node->sourceMap);
                        }

                        cur = ++MarkdownNodeIterator(node);
                    }

                    break;
                }

                default:
                    break;
            }
        }

        if (element.klass != mson::Element::UndefinedClass) {
            out.node.content.elements().push_back(element);

            if (pd.exportSourceMap()) {
                out.sourceMap.elements().collection.push_back(elementSM);
            }
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
