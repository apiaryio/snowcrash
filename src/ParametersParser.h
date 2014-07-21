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

    /** Internal type alias for Collection of Parameter */
    typedef Collection<Parameter>::type Parameters;

    typedef Collection<Parameter>::iterator ParameterIterator;

    /**
     * Parameters section processor
     */
    template<>
    struct SectionProcessor<Parameters> : public SectionProcessorBase<Parameters> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     bool& parsingRedirect,
                                                     Report& report,
                                                     Parameters& out) {

            mdp::ByteBuffer remainingContent;

            GetFirstLine(node->text, remainingContent);

            if (!remainingContent.empty()) {

                // WARN: Extra content in parameters section
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning("additional content in parameters sections",
                                                  IgnoringWarning,
                                                  sourceMap));
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       SectionParserData& pd,
                                                       Report& report,
                                                       Parameters& out) {

            return node;
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         Parameters& out) {

            if (pd.sectionContext() != ParameterSectionType) {
                return node;
            }

            Parameter parameter;
            ParameterParser::parse(node, siblings, pd, report, parameter);

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

                    out.erase(duplicate);
                }
            }

            out.push_back(parameter);

            return ++MarkdownNodeIterator(node);
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

                if (RegexMatch(subject, ParametersRegex)) {
                    return ParametersSectionType;
                }
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            return SectionProcessor<Parameter>::sectionType(node);
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             Report& report,
                             Parameters& out) {

            if (out.empty()) {

                // WARN: No parameters defined
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning("No parameters defined in parameters section",
                                                  EmptyDefinitionWarning,
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
