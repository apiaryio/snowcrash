//
//  MSONParameterParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 12/03/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONPARAMETERPARSER_H
#define SNOWCRASH_MSONPARAMETERPARSER_H

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

            parseAttributes(node, pd, signature.attributes, out);

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

                    if (!typeSection.node.content.value.empty()) {
                        out.node.defaultValue = typeSection.node.content.value;

                        if (pd.exportSourceMap()) {
                            out.sourceMap.defaultValue.sourceMap = typeSection.sourceMap.value.sourceMap;
                        }
                    }

                    break;
                }

                case MSONValueMembersSectionType:
                {
                    typeSection.node.baseType = mson::ImplicitValueBaseType;
                    cur = MSONTypeSectionListParser::parse(node, siblings, pd, typeSection);

                    for (size_t i = 0; i < typeSection.node.content.elements().size(); i++) {
                        mson::ValueMember valueMember = typeSection.node.content.elements().at(i).content.value;
                        SourceMap<mson::ValueMember> valueMemberSM = typeSection.sourceMap.elements().collection.at(i).value;

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

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            SectionType nestedType = UndefinedSectionType;

            // Recognize `Default` and `Members` sections
            nestedType = SectionProcessor<mson::TypeSection>::sectionType(node);

            return nestedType;
        }

        static void parseAttributes(const MarkdownNodeIterator& node,
                                    SectionParserData& pd,
                                    const std::vector<mdp::ByteBuffer>& attributes,
                                    const ParseResultRef<MSONParameter>& out) {

            if (attributes.size() <= 2) {
                size_t i = 0;
                bool definedUse = false;
                out.node.use = RequiredParameterUse;

                // Traverse over parameter's traits
                while (i < attributes.size()) {
                    if (attributes[i] == "optional" && !definedUse) {
                        out.node.use = OptionalParameterUse;
                        definedUse = true;
                    }
                    else if (attributes[i] == "required" && !definedUse) {
                        definedUse = true;
                    }
                    else {
                        if (RegexMatch(attributes[i], "^enum\\[[^][]+]$")) {
                            out.node.type = attributes[i].substr(5, attributes[i].length() - 6);
                        }
                        else {
                            out.node.type = attributes[i];
                        }
                    }

                    i++;
                }

                if (pd.exportSourceMap()) {
                    if (!out.node.type.empty()) {
                        out.sourceMap.type.sourceMap = node->sourceMap;
                    }

                    if (definedUse) {
                        out.sourceMap.use.sourceMap = node->sourceMap;
                    }
                }
            }
            else {
                out.node.use = UndefinedParameterUse;

                // WARN: Additional parameters traits warning
                std::stringstream ss;

                ss << "unable to parse additional parameter traits";
                ss << ", expected '([required | optional], [<type> | enum[<type>])'";
                ss << ", e.g. '(optional, string)'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      FormattingWarning,
                                                      sourceMap));
            }
        }
    };

    /** MSON Parameter Section Parser */
    typedef SectionParser<MSONParameter, ListSectionAdapter> MSONParameterParser;
}

#endif
