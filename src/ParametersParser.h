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

            if (pd.sectionContext() == ParameterSectionType) {
                ParameterParser::parse(node, siblings, pd, parameter);
            }
            else if (pd.sectionContext() == MSONParameterSectionType) {
                IntermediateParseResult<MSONParameter> msonParameter(out.report);
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

    /**
     * \brief Check Parameter validity in URI template
     */

    // It must either be in path "{param" or at the begining
    // "?param" or in the list ",param". And any of the params
    // must end either with "}" or ",".

    //FIXME: The implementation is very naive and can be sped up.
    static bool isValidUriTemplateParam(const std::string &uriTemplate, const std::string &param) {

        if (uriTemplate.find("{"+param) == std::string::npos &&
            uriTemplate.find("?"+param) == std::string::npos &&
            uriTemplate.find(","+param) == std::string::npos) {
            return false;
        }

        if (uriTemplate.find(param+"}") == std::string::npos &&
            uriTemplate.find(param+",") == std::string::npos) {
            return false;
        }

        return true;
    }

    /**
     * \brief Check Parameters eligibility in URI template
     *
     * \warning Do not specialise this.
     */
    template<typename T>
    static void checkParametersEligibility(const MarkdownNodeIterator& node,
                                           const SectionParserData& pd,
                                           Parameters& parameters,
                                           const ParseResultRef<T>& out) {

        for (ParameterIterator it = parameters.begin();
             it != parameters.end();
             ++it) {

            if (!isValidUriTemplateParam(out.node.uriTemplate, it->name)) {

                // WARN: parameter name not present
                std::stringstream ss;
                ss << "parameter '" << it->name << "' is not found within the URI template '" << out.node.uriTemplate << "'";

                if (!out.node.name.empty()) {
                    ss << " for '" << out.node.name << "' ";
                }

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      LogicalErrorWarning,
                                                      sourceMap));
            }
        }
    }

    /** Parameters Section parser */
    typedef SectionParser<Parameters, ListSectionAdapter> ParametersParser;
}

#endif
