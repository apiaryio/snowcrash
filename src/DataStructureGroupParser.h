//
//  DataStructureGroupParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 02/12/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_DATASTRUCTUREGROUPPARSER_H
#define SNOWCRASH_DATASTRUCTUREGROUPPARSER_H

#include "MSONNamedTypeParser.h"

using namespace scpl;

namespace snowcrash {

    /** Data structure group matching regex */
    const char* const DataStructureGroupRegex = "^[[:blank:]]*[Dd]ata[[:blank:]]+[Ss]tructures?[[:blank:]]*$";

    /**
     * Data structure group section processor
     */
    template<>
    struct SectionProcessor<DataStructureGroup> : public SectionProcessorBase<DataStructureGroup> {

        NO_SECTION_DESCRIPTION(DataStructureGroup)

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<DataStructureGroup>& out) {

            MarkdownNodeIterator cur = node;

            if (pd.sectionContext() == MSONNamedTypeSectionType) {

                IntermediateParseResult<mson::NamedType> namedType(out.report);
                cur = MSONNamedTypeParser::parse(node, siblings, pd, namedType);

                if (isNamedTypeDuplicate(pd.blueprint, namedType.node.name.symbol.literal)) {

                    // WARN: duplicate named type
                    std::stringstream ss;
                    ss << "named type with name '" << namedType.node.name.symbol.literal << "' already exists";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          DuplicateWarning,
                                                          sourceMap));
                    return cur;
                }

                Element element(Element::DataStructureElement);
                element.content.dataStructure = namedType.node;

                out.node.content.elements().push_back(element);

                if (pd.exportSourceMap()) {

                    SourceMap<Element> elementSM(Element::DataStructureElement);

                    elementSM.content.dataStructure.name = namedType.sourceMap.name;
                    elementSM.content.dataStructure.typeDefinition = namedType.sourceMap.typeDefinition;
                    elementSM.content.dataStructure.sections = namedType.sourceMap.sections;

                    out.sourceMap.content.elements().collection.push_back(elementSM);
                }
            }

            return cur;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<DataStructureGroup>& out) {

            out.node.element = Element::CategoryElement;
            out.node.category = Element::DataStructureGroupCategory;

            if (pd.exportSourceMap()) {

                out.sourceMap.element = out.node.element;
                out.sourceMap.category = out.node.category;
            }
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::HeaderMarkdownNodeType &&
                !node->text.empty()) {

                mdp::ByteBuffer remaining, subject = node->text;

                subject = GetFirstLine(subject, remaining);
                TrimString(subject);

                if (RegexMatch(subject, DataStructureGroupRegex)) {
                    return DataStructureGroupSectionType;
                }
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            return SectionProcessor<mson::NamedType>::sectionType(node);
        }

        static SectionTypes upperSectionTypes() {
            return {DataStructureGroupSectionType, ResourceGroupSectionType, ResourceSectionType};
        }

        /**
         * \brief Check if a named type already exists with the given name
         *
         * \param blueprint The blueprint which is formed until now
         * \param name The named type name to be checked
         */
        static bool isNamedTypeDuplicate(const Blueprint& blueprint,
                                         mdp::ByteBuffer& name) {

            for (Elements::const_iterator it = blueprint.content.elements().begin();
                 it != blueprint.content.elements().end();
                 ++it) {

                if (it->element == Element::CategoryElement) {

                    for (Elements::const_iterator subIt = it->content.elements().begin();
                         subIt != it->content.elements().end();
                         ++subIt) {

                        if (subIt->element == Element::ResourceElement &&
                            subIt->content.resource.attributes.name.symbol.literal == name) {

                            return true;
                        }

                        if (subIt->element == Element::DataStructureElement &&
                            subIt->content.dataStructure.name.symbol.literal == name) {

                            return true;
                        }
                    }
                }
            }

            return false;
        }
    };

    /** Data Structures Parser */
    typedef SectionParser<DataStructureGroup, HeaderSectionAdapter> DataStructureGroupParser;
}

#endif
