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

    /** Parameter Values */
    typedef Collection<Value>::type Values; // TODO: Move this into core later

    /** Parameter Values matching regex */
    const char* const ValuesRegex = "^[[:blank:]]*[Vv]alues[[:blank:]]*$";

    /**
     * Values section processor
     */
    template<>
    struct SectionProcessor<Values, ValuesSM> : public SectionProcessorBase<Values, ValuesSM> {

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         Values& out,
                                                         ValuesSM& outSM) {

            if (pd.sectionContext() == ValueSectionType) {

                mdp::ByteBuffer content = node->children().front().text;
                CaptureGroups captureGroups;

                RegexCapture(content, PARAMETER_VALUE, captureGroups);

                if (captureGroups.size() > 1) {
                    out.push_back(captureGroups[1]);

                    if (pd.exportSM()) {
                        outSM.push_back(node->sourceMap);
                    }
                } else {
                    TrimString(content);

                    // WARN: Ignoring the unexpected param value
                    std::stringstream ss;
                    ss << "ignoring the '" << content << "' element";
                    ss << ", expected '`" << content << "`'";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(Warning(ss.str(),
                                                      IgnoringWarning,
                                                      sourceMap));
                }

                return ++MarkdownNodeIterator(node);
            }

            return node;
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       SectionParserData& pd,
                                                       Report& report,
                                                       Values& out,
                                                       ValuesSM& outSM) {

            return node;
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            return false;
        }

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
    typedef SectionParser<Values, ValuesSM, ListSectionAdapter> ValuesParser;
}

#endif
