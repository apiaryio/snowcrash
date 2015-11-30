//
//  RelationParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 04/03/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_RELATIONPARSER_H
#define SNOWCRASH_RELATIONPARSER_H

#include "SectionParser.h"
#include "RegexMatch.h"

/** Macro for relation regex */
#define RELATION_REGEX "^[[:blank:]]*[Rr]elation[[:blank:]]*:"

namespace snowcrash {

    /** Link Relation matching regex */
    const char* const RelationRegex = RELATION_REGEX;

    const char* const RelationIdentifierRegex = RELATION_REGEX "[[:blank:]]*([a-z][a-z0-9.-]*)?[[:blank:]]*$";

    /**
     *  Relation Section Processor
     */
    template<>
    struct SectionProcessor<Relation> : public SectionProcessorBase<Relation> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     const ParseResultRef<Relation>& out) {

            mdp::ByteBuffer signature, remainingContent;
            CaptureGroups captureGroups;

            signature = GetFirstLine(node->text, remainingContent);
            TrimString(signature);

            if (RegexCapture(signature, RelationIdentifierRegex, captureGroups, 3)) {
                out.node.str = captureGroups[1];
                TrimString(out.node.str);
            }
            else {
                // WARN: Relation identifier contains illegal characters
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning("relation identifier contains illegal characters (only lower case letters, numbers, '-' and '.' allowed)",
                                                      FormattingWarning,
                                                      sourceMap));
            }

            if (pd.exportSourceMap() && !out.node.str.empty()) {
                out.sourceMap.sourceMap.append(node->sourceMap);
            }

            return ++MarkdownNodeIterator(node);
        }

        NO_SECTION_DESCRIPTION(Relation)

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {

                mdp::ByteBuffer remaining, subject = node->children().front().text;

                subject = GetFirstLine(subject, remaining);
                TrimString(subject);

                if (RegexMatch(subject, RelationRegex)) {
                    return RelationSectionType;
                }
            }

            return UndefinedSectionType;
        }
    };

    /** Relation Section Parser */
    typedef SectionParser<Relation, ListSectionAdapter> RelationParser;
}

#endif
