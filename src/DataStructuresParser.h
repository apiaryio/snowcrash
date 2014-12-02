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

                dataStructure.source = namedType.node;

                out.node.types.push_back(dataStructure);
            }

            return cur;
        }

        static bool isUnexpectedNode(const MarkdownNodeIterator& node,
                                     SectionType sectionType) {

            return true;
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
    };

    /** Data Structures Parser */
    typedef SectionParser<DataStructures, HeaderSectionAdapter> DataStructuresParser;
}

#endif
