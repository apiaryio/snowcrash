//
//  MSONNamedTypeParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONNAMEDTYPEPARSER_H
#define SNOWCRASH_MSONNAMEDTYPEPARSER_H

#include "SectionParser.h"
#include "MSONUtility.h"
#include "MSONPropertyMemberParser.h"
#include "MSONTypeSectionParser.h"
#include "MSONValueMemberParser.h"

using namespace scpl;

namespace snowcrash {

    /**
     * MSON Named Type Section Processor
     */
    template<>
    struct SectionProcessor<mson::NamedType> : public SignatureSectionProcessorBase<mson::NamedType> {

        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(SignatureTraits::IdentifierTrait |
                                            SignatureTraits::AttributesTrait);

            return signatureTraits;
        }

        static void finalizeSignature(const MarkdownNodeIterator& node,
                                      SectionParserData& pd,
                                      const Signature& signature,
                                      const ParseResultRef<mson::NamedType>& out) {

            mson::parseTypeName(signature.identifier, out.node.name);
            mson::parseTypeDefinition(node, pd, signature.attributes, out.report, out.node.base);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<mson::NamedType>& out) {

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

                         switch (out.node.base.baseType) {
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

                                 mson::buildMemberType(propertyMember.node, memberType);
                                 break;
                             }

                             case mson::ValueBaseType:
                             {
                                 IntermediateParseResult<mson::ValueMember> valueMember(out.report);
                                 cur = MSONValueMemberParser::parse(node, siblings, pd, valueMember);

                                 mson::buildMemberType(valueMember.node, memberType);
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

                SectionProcessor<mson::NamedType>::processDescription(node, siblings, pd, out);
            }
            else {
                
                IntermediateParseResult<mson::TypeSection> typeSection(out.report);
                typeSection.node.baseType = out.node.base.baseType;
                
                MSONTypeSectionListParser::parse(node, siblings, pd, typeSection);
                
                out.node.sections.push_back(typeSection.node);
            }
            
            return ++MarkdownNodeIterator(node);
        }


        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       const ParseResultRef<mson::NamedType>& out) {

            if (out.node.sections.empty() ||
                (out.node.sections.size() == 1 &&
                 out.node.sections[0].type == mson::BlockDescriptionTypeSectionType)) {

                if (out.node.sections.empty()) {

                    mson::TypeSection typeSection(mson::BlockDescriptionTypeSectionType);
                    out.node.sections.push_back(typeSection);
                }

                if (!out.node.sections[0].content.description.empty()) {
                    TwoNewLines(out.node.sections[0].content.description);
                }

                mdp::ByteBuffer content = mdp::MapBytesRangeSet(node->sourceMap, pd.sourceData);
                out.node.sections[0].content.description += content;

                return ++MarkdownNodeIterator(node);
            }
            
            return node;
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            return SectionProcessor<mson::ValueMember>::isDescriptionNode(node, sectionType);
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            return MSONNamedTypeSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            return SectionProcessor<mson::ValueMember>::nestedSectionType(node);
        }
    };
    
    /** MSON Named Type Section Parser */
    typedef SectionParser<mson::NamedType, HeaderSectionAdapter> MSONNamedTypeParser;
}

#endif
