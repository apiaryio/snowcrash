//
//  ValuesParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 6/12/14
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_VALUESPARSER_H
#define SNOWCRASH_VALUESPARSER_H

#include "SectionParser.h"
#include "RegexMatch.h"
#include "StringUtility.h"

/** Parameter Value regex */
#define PARAMETER_VALUE "`([^`]+)`"

namespace snowcrash {

    /** Parameter Values matching regex */
    const char* const ValuesRegex = "^[[:blank:]]*[Vv]alues[[:blank:]]*$";

    /**
     * Values section processor
     */
    template<>
    struct SectionProcessor<Values> : public SectionProcessorBase<Values> {

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<Values>& out) {

            if (pd.sectionContext() == ValueSectionType) {

                mdp::ByteBuffer content = node->children().front().text;
                CaptureGroups captureGroups;

                RegexCapture(content, PARAMETER_VALUE, captureGroups);

                if (captureGroups.size() > 1) {
                    out.node.push_back(captureGroups[1]);

                    if (pd.exportSourceMap()) {
                        SourceMap<Value> valueSM;
                        valueSM.sourceMap = node->sourceMap;
                        out.sourceMap.collection.push_back(valueSM);
                    }
                } else {
                    TrimString(content);

                    // WARN: Ignoring the unexpected param value
                    std::stringstream ss;
                    ss << "ignoring the '" << content << "' element";
                    ss << ", expected '`" << content << "`'";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          IgnoringWarning,
                                                          sourceMap));
                }

                return ++MarkdownNodeIterator(node);
            }

            return node;
        }

        NO_SECTION_DESCRIPTION(Values)

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {

                mdp::ByteBuffer subject = node->children().front().text;
                TrimString(subject);

                if (RegexMatch(subject, ValuesRegex)) {
                    return ValuesSectionType;
                }
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {

                mdp::ByteBuffer subject = node->children().front().text;
                TrimString(subject);

                if (node->children().size() == 1 &&
                    !subject.empty()) {

                    return ValueSectionType;
                }
            }

            return UndefinedSectionType;
        }
    };

    /** Parameter Section Parser */
    typedef SectionParser<Values, ListSectionAdapter> ValuesParser;
}

#endif
