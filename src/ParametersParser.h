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
    struct SectionProcessor<Parameters, ParametersSM> : public SectionProcessorBase<Parameters, ParametersSM> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     Report& report,
                                                     Parameters& out,
                                                     ParametersSM& outSM) {

            mdp::ByteBuffer remainingContent;

            GetFirstLine(node->text, remainingContent);

            if (!remainingContent.empty()) {

                // WARN: Extra content in parameters section
                std::stringstream ss;
                ss << "ignoring additional content after 'parameters' keyword,";
                ss << " expected a nested list of parameters, one parameter per list item";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning(ss.str(),
                                                  IgnoringWarning,
                                                  sourceMap));
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       Report& report,
                                                       Parameters& out,
                                                       ParametersSM& outSM) {

            return node;
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         Parameters& out,
                                                         ParametersSM& outSM) {

            if (pd.sectionContext() != ParameterSectionType) {
                return node;
            }

            Parameter parameter;
            ParameterSM parameterSM;

            ParameterParser::parse(node, siblings, pd, report, parameter, parameterSM);

            if (!out.empty()) {

                ParameterIterator duplicate = findParameter(out, parameter);

                if (duplicate != out.end()) {

                    // WARN: Parameter already defined
                    std::stringstream ss;
                    ss << "overshadowing previous parameter '" << parameter.name << "' definition";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(Warning(ss.str(),
                                                      RedefinitionWarning,
                                                      sourceMap));
                }
            }

            out.push_back(parameter);

            if (pd.exportSM()) {
                outSM.push_back(parameterSM);
            }

            return ++MarkdownNodeIterator(node);
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            return false;
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

            return SectionProcessor<Parameter, ParameterSM>::sectionType(node);
        }

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested;

            // Parameter & descendants
            nested.push_back(ParameterSectionType);
            SectionTypes types = SectionProcessor<Parameter, ParameterSM>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            return nested;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             Report& report,
                             Parameters& out,
                             ParametersSM& outSM) {

            if (out.empty()) {

                // WARN: No parameters defined
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning(NoParametersMessage,
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
    typedef SectionParser<Parameters, ParametersSM, ListSectionAdapter> ParametersParser;
}

#endif
