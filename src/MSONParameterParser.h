//
//  MSONParameterParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 12/03/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONPARAMETERPARSER_H
#define SNOWCRASH_MSONPARAMETERPARSER_H

#include "ParameterParser.h"
#include "MSONValueMemberParser.h"
#include "MSONTypeSectionParser.h"

using namespace scpl;

namespace snowcrash {

    /**
     * MSON Parameter Section Processor
     */
    template<>
    struct SectionProcessor<MSONParameter> : public SignatureSectionProcessorBase<MSONParameter> {

        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(SignatureTraits::IdentifierTrait |
                                            SignatureTraits::ValuesTrait |
                                            SignatureTraits::AttributesTrait |
                                            SignatureTraits::ContentTrait);

            return signatureTraits;
        }

        static MarkdownNodeIterator finalizeSignature(const MarkdownNodeIterator& node,
                                                      SectionParserData& pd,
                                                      const Signature& signature,
                                                      const ParseResultRef<MSONParameter>& out) {

            out.node.name = signature.identifier;
            out.node.description = signature.content;
            out.node.exampleValue = signature.value;

            if (!signature.remainingContent.empty()) {
                out.node.description += "\n" + signature.remainingContent + "\n";
            }

            SectionProcessor<Parameter>::parseAttributes(node, pd, signature.attributes, out, false);

            if (pd.exportSourceMap()) {
                if (!out.node.name.empty()) {
                    out.sourceMap.name.sourceMap = node->sourceMap;
                }

                if (!out.node.description.empty()) {
                    out.sourceMap.description.sourceMap = node->sourceMap;
                }

                if (!out.node.exampleValue.empty()) {
                    out.sourceMap.exampleValue.sourceMap = node->sourceMap;
                }
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<MSONParameter>& out) {

            SectionType sectionType = pd.sectionContext();
            MarkdownNodeIterator cur = node;
            IntermediateParseResult<mson::TypeSection> typeSection(out.report);

            switch (sectionType) {
                case MSONSampleDefaultSectionType:
                {
                    typeSection.node.baseType = mson::ImplicitPrimitiveBaseType;
                    cur = MSONTypeSectionListParser::parse(node, siblings, pd, typeSection);

                    if (typeSection.node.content.value.empty()) {
                        break;
                    }

                    if (typeSection.node.klass == mson::TypeSection::DefaultClass) {
                        out.node.defaultValue = typeSection.node.content.value;

                        if (pd.exportSourceMap()) {
                            out.sourceMap.defaultValue.sourceMap = typeSection.sourceMap.value.sourceMap;
                        }
                    }
                    else if (typeSection.node.klass == mson::TypeSection::SampleClass) {
                        out.node.exampleValue = typeSection.node.content.value;
                        out.sourceMap.exampleValue.sourceMap = typeSection.sourceMap.value.sourceMap;
                    }

                    break;
                }

                case MSONValueMembersSectionType:
                {
                    typeSection.node.baseType = mson::ImplicitValueBaseType;
                    cur = MSONTypeSectionListParser::parse(node, siblings, pd, typeSection);

                    out.node.values.clear();

                    if (pd.exportSourceMap()) {
                        out.sourceMap.values.collection.clear();
                    }

                    for (size_t i = 0; i < typeSection.node.content.elements().size(); i++) {
                        mson::ValueMember valueMember = typeSection.node.content.elements().at(i).content.value;
                        SourceMap<mson::ValueMember> valueMemberSM;

                        if (pd.exportSourceMap()) {
                            valueMemberSM = typeSection.sourceMap.elements().collection.at(i).value;
                        }

                        if (valueMember.valueDefinition.values.size() == 1) {
                            out.node.values.push_back(valueMember.valueDefinition.values[0].literal);

                            if (pd.exportSourceMap()) {
                                SourceMap<Value> valueSM;
                                valueSM.sourceMap = valueMemberSM.valueDefinition.sourceMap;

                                out.sourceMap.values.collection.push_back(valueSM);
                            }
                        }
                    }

                    break;
                }

                default:
                    break;
            }

            return cur;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<MSONParameter>& out) {

            SectionProcessor<Parameter>::checkDefaultAndRequiredClash<MSONParameter>(node, pd, out);
            SectionProcessor<Parameter>::checkExampleAndDefaultValue<MSONParameter>(node, pd, out);
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            SectionType nestedType = UndefinedSectionType;

            // Recognize `Default` and `Members` sections
            nestedType = SectionProcessor<mson::TypeSection>::sectionType(node);

            return nestedType;
        }
    };

    /** MSON Parameter Section Parser */
    typedef SectionParser<MSONParameter, ListSectionAdapter> MSONParameterParser;
}

#endif
