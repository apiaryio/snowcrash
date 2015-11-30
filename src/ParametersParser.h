//
//  ParametersParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 9/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_PARAMETERSPARSER_H
#define SNOWCRASH_PARAMETERSPARSER_H

#include "SectionParser.h"
#include "ParameterParser.h"
#include "MSONParameterParser.h"
#include "RegexMatch.h"
#include "StringUtility.h"
#include "BlueprintUtility.h"

namespace snowcrash {

    /** Parameters matching regex */
    const char* const ParametersRegex = "^[[:blank:]]*[Pp]arameters?[[:blank:]]*$";

    /** No parameters specified message */
    const char* const NoParametersMessage = "no parameters specified, expected a nested list of parameters, one parameter per list item";

    /** Internal type alias for Collection iterator of Parameter */
    typedef Collection<Parameter>::iterator ParameterIterator;

    /**
     * Parameters section processor
     */
    template<>
    struct SectionProcessor<Parameters> : public SectionProcessorBase<Parameters> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     const ParseResultRef<Parameters>& out) {

            mdp::ByteBuffer remainingContent;

            GetFirstLine(node->text, remainingContent);

            if (!remainingContent.empty()) {

                // WARN: Extra content in parameters section
                std::stringstream ss;
                ss << "ignoring additional content after 'parameters' keyword,";
                ss << " expected a nested list of parameters, one parameter per list item";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      IgnoringWarning,
                                                      sourceMap));
            }

            return ++MarkdownNodeIterator(node);
        }

        NO_SECTION_DESCRIPTION(Parameters)

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<Parameters>& out) {

            IntermediateParseResult<Parameter> parameter(out.report);
            IntermediateParseResult<MSONParameter> msonParameter(out.report);

            if (pd.sectionContext() == ParameterSectionType) {
                ParameterParser::parse(node, siblings, pd, parameter);
            }
            else if (pd.sectionContext() == MSONParameterSectionType) {
                MSONParameterParser::parse(node, siblings, pd, msonParameter);

                // Copy values from MSON Parameter to normal parameter
                parameter.report = msonParameter.report;
                parameter.node = msonParameter.node;
                parameter.sourceMap = msonParameter.sourceMap;
            }
            else {
                return node;
            }

            if (!out.node.empty()) {

                ParameterIterator duplicate = findParameter(out.node, parameter.node);

                if (duplicate != out.node.end()) {

                    // WARN: Parameter already defined
                    std::stringstream ss;
                    ss << "overshadowing previous parameter '" << parameter.node.name << "' definition";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          RedefinitionWarning,
                                                          sourceMap));
                }
            }

            out.node.push_back(parameter.node);

            if (pd.exportSourceMap()) {
                out.sourceMap.collection.push_back(parameter.sourceMap);
            }

            return ++MarkdownNodeIterator(node);
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {

                mdp::ByteBuffer remaining, subject = node->children().front().text;

                subject = GetFirstLine(subject, remaining);
                TrimString(subject);

                if (RegexMatch(subject, ParametersRegex)) {
                    return ParametersSectionType;
                }
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            return SectionProcessor<Parameter>::sectionType(node);
        }

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested;

            // Parameter & descendants
            nested.push_back(ParameterSectionType);
            SectionTypes types = SectionProcessor<Parameter>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            return nested;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<Parameters>& out) {

            if (out.node.empty()) {

                // WARN: No parameters defined
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(NoParametersMessage,
                                                      FormattingWarning,
                                                      sourceMap));
            }
        }

        /** Finds a parameter inside a parameters collection */
        static ParameterIterator findParameter(Parameters& parameters,
                                               const Parameter& parameter) {

            return std::find_if(parameters.begin(),
                                parameters.end(),
                                std::bind2nd(MatchName<Parameter>(), parameter));
        }
    };

    /** Parameters Section parser */
    typedef SectionParser<Parameters, ListSectionAdapter> ParametersParser;
}

#endif
