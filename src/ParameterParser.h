//
//  ParameterParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 9/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_PARAMETERPARSER_H
#define SNOWCRASH_PARAMETERPARSER_H

#include "SectionParser.h"
#include "ValuesParser.h"
#include "RegexMatch.h"
#include "StringUtility.h"

/** Parameter Identifier */
#define PARAMETER_IDENTIFIER "([[:alnum:]_.-]+)"

/** Lead in and out for comma separated values regex */
#define CSV_LEADINOUT "[[:blank:]]*,?[[:blank:]]*"

namespace snowcrash {

    /** Parameter Abbreviated definition matching regex */
    const char* const ParameterAbbrevDefinitionRegex = "^" PARAMETER_IDENTIFIER \
                                                        "([[:blank:]]*=[[:blank:]]*`([^`]*)`[[:blank:]]*)?([[:blank:]]*\\(([^)]*)\\)[[:blank:]]*)?([[:blank:]]*\\.\\.\\.[[:blank:]]*(.*))?$";

    /** Parameter Required matching regex */
    const char* const ParameterRequiredRegex = "^[[:blank:]]*[Rr]equired[[:blank:]]*$";

    /** Parameter Optional matching regex */
    const char* const ParameterOptionalRegex = "^[[:blank:]]*[Oo]ptional[[:blank:]]*$";

    /** Additonal Parameter Traits Example matching regex */
    const char* const AdditionalTraitsExampleRegex = CSV_LEADINOUT "`([^`]*)`" CSV_LEADINOUT;

    /** Additonal Parameter Traits Use matching regex */
    const char* const AdditionalTraitsUseRegex = CSV_LEADINOUT "([Oo]ptional|[Rr]equired)" CSV_LEADINOUT;

    /** Additonal Parameter Traits Type matching regex */
    const char* const AdditionalTraitsTypeRegex = CSV_LEADINOUT "([^,]*)" CSV_LEADINOUT;

    /** Values expected content */
    const char* const ExpectedValuesContent = "nested list of possible parameter values, one element per list item e.g. '`value`'";

    /**
     * Parameter section processor
     */
    template<>
    struct SectionProcessor<Parameter, ParameterSM> : public SectionProcessorBase<Parameter, ParameterSM> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     Report& report,
                                                     Parameter& out,
                                                     ParameterSM& outSM) {

            mdp::ByteBuffer signature, remainingContent;
            signature = GetFirstLine(node->text, remainingContent);

            parseSignature(node, pd, signature, report, out, outSM);

            if (!remainingContent.empty()) {
                out.description += "\n" + remainingContent + "\n";

                if (pd.exportSM()) {
                    outSM.description.append(node->sourceMap);
                }
            }

            return ++MarkdownNodeIterator(node);
        }


        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         Parameter& out,
                                                         ParameterSM& outSM) {

            if (pd.sectionContext() != ValuesSectionType) {
                return node;
            }

            // Check redefinition
            if (!out.values.empty()) {
                // WARN: parameter values are already defined
                std::stringstream ss;
                ss << "overshadowing previous 'values' definition";
                ss << " for parameter '" << out.name << "'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning(ss.str(),
                                                  RedefinitionWarning,
                                                  sourceMap));
            }

            // Clear any previous values
            out.values.clear();

            if (pd.exportSM()) {
                outSM.values.clear();
            }

            ValuesParser::parse(node, siblings, pd, report, out.values, outSM.values);

            if (out.values.empty()) {
                // WARN: empty definition
                std::stringstream ss;
                ss << "no possible values specified for parameter '" << out.name << "'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning(ss.str(),
                                                  EmptyDefinitionWarning,
                                                  sourceMap));
            }

            if ((!out.exampleValue.empty() || !out.defaultValue.empty()) &&
                 !out.values.empty()) {

                checkExampleAndDefaultValue(node, pd, report, out);
            }

            return ++MarkdownNodeIterator(node);
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {

                mdp::ByteBuffer subject, remainingContent;
                subject = GetFirstLine(node->children().front().text, remainingContent);
                TrimString(subject);

                if (RegexMatch(subject, ParameterAbbrevDefinitionRegex)) {
                    return ParameterSectionType;
                }
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            return SectionProcessor<Values, ValuesSM>::sectionType(node);
        }

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested;

            nested.push_back(ValuesSectionType);

            return nested;
        }

        static void parseSignature(const mdp::MarkdownNodeIterator& node,
                                   SectionParserData& pd,
                                   mdp::ByteBuffer& signature,
                                   Report& report,
                                   Parameter& parameter,
                                   ParameterSM& parameterSM) {

            parameter.use = UndefinedParameterUse;

            TrimString(signature);

            CaptureGroups captureGroups;

            if (RegexCapture(signature, ParameterAbbrevDefinitionRegex, captureGroups) &&
                captureGroups.size() == 8) {

                // Name
                parameter.name = captureGroups[1];
                TrimString(parameter.name);

                if (pd.exportSM() && !parameter.name.empty()) {
                    parameterSM.name = node->sourceMap;
                }

                // Default value
                if (!captureGroups[3].empty()) {
                    parameter.defaultValue = captureGroups[3];

                    if (pd.exportSM()) {
                        parameterSM.defaultValue = node->sourceMap;
                    }
                }

                // Additional attributes
                if (!captureGroups[5].empty()) {
                    parseAdditionalTraits(node, pd, captureGroups[5], report, parameter, parameterSM);
                }

                // Description
                if (!captureGroups[7].empty()) {
                    parameter.description = captureGroups[7];

                    if (pd.exportSM()) {
                        parameterSM.description = node->sourceMap;
                    }
                }

                if (parameter.use != OptionalParameterUse &&
                    !parameter.defaultValue.empty()) {

                    // WARN: Required vs default clash
                    std::stringstream ss;
                    ss << "specifying parameter '" << parameter.name << "' as required supersedes its default value"\
                          ", declare the parameter as 'optional' to specify its default value";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(Warning(ss.str(),
                                                      LogicalErrorWarning,
                                                      sourceMap));
                }
            } else {
                // ERR: Unable to parse
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.error = Error("unable to parse parameter specification",
                                     BusinessError,
                                     sourceMap);
            }
        }

        static void parseAdditionalTraits(const mdp::MarkdownNodeIterator& node,
                                          SectionParserData& pd,
                                          mdp::ByteBuffer& traits,
                                          Report& report,
                                          Parameter& parameter,
                                          ParameterSM& parameterSM) {

            TrimString(traits);

            CaptureGroups captureGroups;

            // Cherry pick example value, if any
            if (RegexCapture(traits, AdditionalTraitsExampleRegex, captureGroups) &&
                captureGroups.size() > 1) {

                parameter.exampleValue = captureGroups[1];
                std::string::size_type pos = traits.find(captureGroups[0]);

                if (pos != std::string::npos) {
                    traits.replace(pos, captureGroups[0].length(), std::string());
                }

                if (pd.exportSM()) {
                    parameterSM.exampleValue = node->sourceMap;
                }
             }

            captureGroups.clear();

            // Cherry pick use attribute, if any
            if (RegexCapture(traits, AdditionalTraitsUseRegex, captureGroups) &&
                captureGroups.size() > 1) {

                parameter.use = RegexMatch(captureGroups[1], ParameterOptionalRegex) ? OptionalParameterUse : RequiredParameterUse;
                std::string::size_type pos = traits.find(captureGroups[0]);

                if (pos != std::string::npos) {
                    traits.replace(pos, captureGroups[0].length(), std::string());
                }

                if (pd.exportSM()) {
                    parameterSM.use = node->sourceMap;
                }
            }

            captureGroups.clear();

            // Finish with type
            if (RegexCapture(traits, AdditionalTraitsTypeRegex, captureGroups) &&
                captureGroups.size() > 1) {

                parameter.type = captureGroups[1];
                std::string::size_type pos = traits.find(captureGroups[0]);

                if (pos != std::string::npos) {
                    traits.replace(pos, captureGroups[0].length(), std::string());
                }

                if (pd.exportSM()) {
                    parameterSM.type = node->sourceMap;
                }
            }

            // Check what is left
            TrimString(traits);

            if (!traits.empty()) {
                // WARN: Additional parameters traits warning
                std::stringstream ss;
                ss << "unable to parse additional parameter traits";
                ss << ", expected '([required | optional], [<type>], [`<example value>`])'";
                ss << ", e.g. '(optional, string, `Hello World`)'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning(ss.str(),
                                                  FormattingWarning,
                                                  sourceMap));

                parameter.type.clear();
                parameter.exampleValue.clear();
                parameter.use = UndefinedParameterUse;

                if (pd.exportSM()) {
                    parameterSM.type.clear();
                    parameterSM.exampleValue.clear();
                    parameterSM.use.clear();
                }
            }
        }

        static void checkExampleAndDefaultValue(const mdp::MarkdownNodeIterator& node,
                                                SectionParserData& pd,
                                                Report& report,
                                                Parameter& parameter) {

            bool isExampleFound = false;
            bool isDefaultFound = false;

            std::stringstream ss;
            bool printWarning = false;

            for (Collection<Value>::iterator it = parameter.values.begin();
                 it != parameter.values.end();
                 ++it) {

                if (parameter.exampleValue == *it) {
                    isExampleFound = true;
                }

                if (parameter.defaultValue == *it) {
                    isDefaultFound = true;
                }
            }

            if(!parameter.exampleValue.empty() &&
               !isExampleFound) {

                // WARN: missing example in values.
                ss << "the example value '" << parameter.exampleValue << "' of parameter '"<< parameter.name <<"' is not in its list of expected values";
                printWarning = true;
            }

            if(!parameter.defaultValue.empty() &&
               !isDefaultFound) {

                // WARN: missing default in values.
                ss << "the default value '" << parameter.defaultValue << "' of parameter '"<< parameter.name <<"' is not in its list of expected values";
                printWarning = true;
            }

            if (printWarning) {
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning(ss.str(),
                                                  LogicalErrorWarning,
                                                  sourceMap));
            }
        }
    };

    /** Parameter Section Parser */
    typedef SectionParser<Parameter, ParameterSM, ListSectionAdapter> ParameterParser;
}

#endif
