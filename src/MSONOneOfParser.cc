//
//  MSONOneOfParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/12/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "MSONOneOfParser.h"
#include "MSONTypeSectionParser.h"
#include "MSONPropertyMemberParser.h"

namespace snowcrash {

    /** Implementation of processNestedSection */
    MarkdownNodeIterator SectionProcessor<mson::OneOf>::processNestedSection(const MarkdownNodeIterator& node,
                                                                             const MarkdownNodes& siblings,
                                                                             SectionParserData& pd,
                                                                             const ParseResultRef<mson::OneOf>& out) {

        MarkdownNodeIterator cur = node;
        mson::Element element;
        SourceMap<mson::Element> elementSM;

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
                IntermediateParseResult<mson::OneOf> oneOf(out.report);
                cur = MSONOneOfParser::parse(node, siblings, pd, oneOf);

                element.build(oneOf.node);

                if (pd.exportSourceMap()) {
                    elementSM = oneOf.sourceMap;
                }

                break;
            }

            case MSONPropertyMembersSectionType:
            {
                IntermediateParseResult<mson::TypeSection> typeSection(out.report);
                typeSection.node.baseType = mson::ObjectBaseType;

                cur = MSONTypeSectionListParser::parse(node, siblings, pd, typeSection);

                element.buildFromElements(typeSection.node.content.elements());

                if (pd.exportSourceMap()) {
                    elementSM = typeSection.sourceMap.elements();
                }

                break;
            }

            case MSONPropertyMemberSectionType:
            {
                IntermediateParseResult<mson::PropertyMember> propertyMember(out.report);
                cur = MSONPropertyMemberParser::parse(node, siblings, pd, propertyMember);

                element.build(propertyMember.node);

                if (pd.exportSourceMap()) {
                    elementSM.property = propertyMember.sourceMap;
                }

                break;
            }

            default:
                break;
        }

        if (element.klass != mson::Element::UndefinedClass) {
            out.node.push_back(element);

            if (pd.exportSourceMap()) {
                out.sourceMap.collection.push_back(elementSM);
            }
        }

        return cur;
    }

    /** Implementation of nestedSectionType */
    SectionType SectionProcessor<mson::OneOf>::nestedSectionType(const MarkdownNodeIterator& node) {

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

        // Check if mson member type section section
        nestedType = SectionProcessor<mson::TypeSection>::sectionType(node);

        if (nestedType != UndefinedSectionType) {
            return nestedType;
        }

        // Return property member type section if list item
        if (node->type == mdp::ListItemMarkdownNodeType) {
            return MSONPropertyMemberSectionType;
        }

        return UndefinedSectionType;
    }
}
