//
//  DataStructuresParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 02/12/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_DATASTRUCTURESPARSER_H
#define SNOWCRASH_DATASTRUCTURESPARSER_H

#include "MSONNamedTypeParser.h"

using namespace scpl;

namespace snowcrash {

    /** Data structures matching regex */
    const char* const DataStructuresRegex = "^[[:blank:]]*[Dd]ata[[:blank:]]+[Ss]tructures?[[:blank:]]*$";

    /**
     * Data structures section processor
     */
    template<>
    struct SectionProcessor<DataStructures> : public SectionProcessorBase<DataStructures> {

        NO_SECTION_DESCRIPTION(DataStructures)

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<DataStructures>& out) {

            MarkdownNodeIterator cur = node;

            if (pd.sectionContext() == MSONNamedTypeSectionType) {

                DataStructure dataStructure;
                IntermediateParseResult<mson::NamedType> namedType(out.report);

                cur = MSONNamedTypeParser::parse(node, siblings, pd, namedType);

                if (isNamedTypeDuplicate(pd.blueprint, namedType.node.name.symbol.literal)) {

                    // WARN: duplicate named type
                    std::stringstream ss;
                    ss << "named type with name '" << namedType.node.name.symbol.literal << "' already exists";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          DuplicateWarning,
                                                          sourceMap));
                }
                else {

                    dataStructure.source = namedType.node;
                    out.node.push_back(dataStructure);
                }
            }

            return cur;
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::HeaderMarkdownNodeType &&
                !node->text.empty()) {

                mdp::ByteBuffer remaining, subject = node->text;

                subject = GetFirstLine(subject, remaining);
                TrimString(subject);

                if (RegexMatch(subject, DataStructuresRegex)) {
                    return DataStructuresSectionType;
                }
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            return SectionProcessor<mson::NamedType>::sectionType(node);
        }

        /**
         * \brief Check if a named type already exists with the given name
         *
         * \param blueprint The blueprint which is formed until now
         * \param name The named type name to be checked
         */
        static bool isNamedTypeDuplicate(const Blueprint& blueprint,
                                         mdp::ByteBuffer& name) {

            for (Collection<ResourceGroup>::const_iterator resourceGroupIt = blueprint.resourceGroups.begin();
                 resourceGroupIt != blueprint.resourceGroups.end();
                 ++resourceGroupIt) {

                for (Collection<Resource>::const_iterator resourceIt = resourceGroupIt->resources.begin();
                     resourceIt != resourceGroupIt->resources.end();
                     ++resourceIt) {

                    if (resourceIt->attributes.source.name.symbol.literal == name) {
                        return true;
                    }
                }
            }

            for (DataStructures::const_iterator it = blueprint.dataStructures.begin();
                 it != blueprint.dataStructures.end();
                 ++it) {

                if (it->source.name.symbol.literal == name) {
                    return true;
                }
            }

            return false;
        }
    };

    /** Data Structures Parser */
    typedef SectionParser<DataStructures, HeaderSectionAdapter> DataStructuresParser;
}

#endif
