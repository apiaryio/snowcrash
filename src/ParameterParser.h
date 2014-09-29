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

/** Parameter Value regex */
#define PARAMETER_VALUE "`([^`]+)`"

/** Parameter Identifier */
#define PARAMETER_IDENTIFIER "(([[:alnum:]_.-])*|(%[A-Fa-f0-9]{2})*)+"

/** Lead in and out for comma separated values regex */
#define CSV_LEADINOUT "[[:blank:]]*,?[[:blank:]]*"

namespace snowcrash {

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
    
    /** Parameter Values matching regex */
    const char* const ParameterValuesRegex = "^[[:blank:]]*[Vv]alues[[:blank:]]*$";

    /** Values expected content */
    const char* const ExpectedValuesContent = "nested list of possible parameter values, one element per list item e.g. '`value`'";
    
    /** Parameter description delimiter */
    const std::string DescriptionIdentifier = "...";

    /**
     * Parameter section processor
     */
    template<>
    struct SectionProcessor<Parameter> : public SectionProcessorBase<Parameter> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     ParseResult<Parameter>& out) {

            mdp::ByteBuffer signature, remainingContent;
            signature = GetFirstLine(node->text, remainingContent);

            parseSignature(node, pd, signature, out);

            if (!remainingContent.empty()) {
                out.node.description += "\n" + remainingContent + "\n";

                if (pd.exportSourceMap()) {
                    out.sourceMap.description.sourceMap.append(node->sourceMap);
                }
            }

            return ++MarkdownNodeIterator(node);
        }


        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         ParseResult<Parameter>& out) {

            if (pd.sectionContext() != ValuesSectionType) {
                return node;
            }

            // Check redefinition
            if (!out.node.values.empty()) {
                // WARN: parameter values are already defined
                std::stringstream ss;
                ss << "overshadowing previous 'values' definition";
                ss << " for parameter '" << out.node.name << "'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      RedefinitionWarning,
                                                      sourceMap));
            }

            // Clear any previous values
            out.node.values.clear();

            if (pd.exportSourceMap()) {
                out.sourceMap.values.collection.clear();
            }

            ParseResult<Values> values;
            ValuesParser::parse(node, siblings, pd, values);

            out.report += values.report;
            out.node.values = values.node;
            out.sourceMap.values = values.sourceMap;

            if (out.node.values.empty()) {
                // WARN: empty definition
                std::stringstream ss;
                ss << "no possible values specified for parameter '" << out.node.name << "'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      EmptyDefinitionWarning,
                                                      sourceMap));
            }

            if ((!out.node.exampleValue.empty() || !out.node.defaultValue.empty()) &&
                 !out.node.values.empty()) {

                checkExampleAndDefaultValue(node, pd, out);
            }

            return ++MarkdownNodeIterator(node);
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {

                mdp::ByteBuffer subject, remainingContent;
                subject = GetFirstLine(node->children().front().text, remainingContent);
                TrimString(subject);

                if (isValidParameterSignature(subject)) {
                    return ParameterSectionType;
                }
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            return SectionProcessor<Values>::sectionType(node);
        }

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested;

            nested.push_back(ValuesSectionType);

            return nested;
        }

        static void parseSignature(const mdp::MarkdownNodeIterator& node,
                                   SectionParserData& pd,
                                   mdp::ByteBuffer& signature,
                                   ParseResult<Parameter>& out) {

            out.node.use = UndefinedParameterUse;

            TrimString(signature);

            CaptureGroups captureGroups;

            if (isValidParameterSignature(signature)) {
                
                mdp::ByteBuffer innerSignature = signature;
                innerSignature = TrimString(innerSignature);
                
                size_t firstSpace = innerSignature.find(" ");

                if (firstSpace == std::string::npos) {
                    // Name
                    out.node.name = signature;
                }
                else {
                    out.node.name = innerSignature.substr(0, firstSpace);
                    innerSignature = innerSignature.substr(firstSpace + 1);

                    size_t descriptionPos = innerSignature.find(snowcrash::DescriptionIdentifier);

                    if (descriptionPos != std::string::npos) {
                        // Description
                        out.node.description = innerSignature.substr(descriptionPos);
                        out.node.description = TrimString(out.node.description.replace(0, snowcrash::DescriptionIdentifier.length(), ""));

                        innerSignature = innerSignature.substr(0, descriptionPos);
                        innerSignature = TrimString(innerSignature);
                    }
                    
                    size_t attributesPos = innerSignature.find("(");

                    if (attributesPos != std::string::npos) {
                        size_t endOfAttributesPos = innerSignature.find_last_of(")");

                        if (endOfAttributesPos - attributesPos > 1) {
                            std::string attributes = innerSignature.substr(attributesPos, endOfAttributesPos - attributesPos);
                            attributes = attributes.substr(1);

                            parseAdditionalTraits(node, pd, attributes, out);

                            innerSignature = innerSignature.substr(0, attributesPos);
                            innerSignature = TrimString(innerSignature);
                        }
                    }
                    
                    if (innerSignature.length() > 0) {
                        // Remove =
                        out.node.defaultValue = innerSignature;

                        out.node.defaultValue.erase(std::remove(out.node.defaultValue.begin(), out.node.defaultValue.end(), '='), out.node.defaultValue.end());
                        out.node.defaultValue.erase(std::remove(out.node.defaultValue.begin(), out.node.defaultValue.end(), '`'), out.node.defaultValue.end());

                        out.node.defaultValue = TrimString(out.node.defaultValue);
                    }
                }

                if (pd.exportSourceMap()) {
                    if (!out.node.name.empty()) {
                        out.sourceMap.name.sourceMap = node->sourceMap;
                    }

                    if (!out.node.description.empty()) {
                        out.sourceMap.description.sourceMap = node->sourceMap;
                    }

                    if (!out.node.defaultValue.empty()) {
                        out.sourceMap.defaultValue.sourceMap = node->sourceMap;
                    }
                }

                // Check possible required vs default clash
                if (out.node.use != OptionalParameterUse &&
                    !out.node.defaultValue.empty()) {

                    // WARN: Required vs default clash
                    std::stringstream ss;
                    ss << "specifying parameter '" << out.node.name << "' as required supersedes its default value"\
                          ", declare the parameter as 'optional' to specify its default value";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          LogicalErrorWarning,
                                                          sourceMap));
                }
            } else {
                // ERR: unable to parse
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.error = Error("unable to parse parameter specification",
                                         BusinessError,
                                         sourceMap);
            }
        }

        static void parseAdditionalTraits(const mdp::MarkdownNodeIterator& node,
                                          SectionParserData& pd,
                                          mdp::ByteBuffer& traits,
                                          ParseResult<Parameter>& out) {

            TrimString(traits);

            CaptureGroups captureGroups;

            // Cherry pick example value, if any
            if (RegexCapture(traits, AdditionalTraitsExampleRegex, captureGroups) &&
                captureGroups.size() > 1) {

                out.node.exampleValue = captureGroups[1];
                std::string::size_type pos = traits.find(captureGroups[0]);

                if (pos != std::string::npos) {
                    traits.replace(pos, captureGroups[0].length(), std::string());
                }

                if (pd.exportSourceMap()) {
                    out.sourceMap.exampleValue.sourceMap = node->sourceMap;
                }
             }

            captureGroups.clear();

            // Cherry pick use attribute, if any
            if (RegexCapture(traits, AdditionalTraitsUseRegex, captureGroups) &&
                captureGroups.size() > 1) {

                out.node.use = RegexMatch(captureGroups[1], ParameterOptionalRegex) ? OptionalParameterUse : RequiredParameterUse;
                std::string::size_type pos = traits.find(captureGroups[0]);

                if (pos != std::string::npos) {
                    traits.replace(pos, captureGroups[0].length(), std::string());
                }

                if (pd.exportSourceMap()) {
                    out.sourceMap.use.sourceMap = node->sourceMap;
                }
            }

            captureGroups.clear();

            // Finish with type
            if (RegexCapture(traits, AdditionalTraitsTypeRegex, captureGroups) &&
                captureGroups.size() > 1) {

                out.node.type = captureGroups[1];
                std::string::size_type pos = traits.find(captureGroups[0]);

                if (pos != std::string::npos) {
                    traits.replace(pos, captureGroups[0].length(), std::string());
                }

                if (pd.exportSourceMap()) {
                    out.sourceMap.type.sourceMap = node->sourceMap;
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
                out.report.warnings.push_back(Warning(ss.str(),
                                                            FormattingWarning,
                                                            sourceMap));

                out.node.type.clear();
                out.node.exampleValue.clear();
                out.node.use = UndefinedParameterUse;

                if (pd.exportSourceMap()) {
                    out.sourceMap.type.sourceMap.clear();
                    out.sourceMap.exampleValue.sourceMap.clear();
                    out.sourceMap.use.sourceMap.clear();
                }
            }
        }

        static void checkExampleAndDefaultValue(const mdp::MarkdownNodeIterator& node,
                                                SectionParserData& pd,
                                                ParseResult<Parameter>& out) {

            bool isExampleFound = false;
            bool isDefaultFound = false;

            std::stringstream ss;
            bool printWarning = false;

            for (Collection<Value>::iterator it = out.node.values.begin();
                 it != out.node.values.end();
                 ++it) {

                if (out.node.exampleValue == *it) {
                    isExampleFound = true;
                }

                if (out.node.defaultValue == *it) {
                    isDefaultFound = true;
                }
            }

            if(!out.node.exampleValue.empty() &&
               !isExampleFound) {

                // WARN: missing example in values.
                ss << "the example value '" << out.node.exampleValue << "' of parameter '"<< out.node.name <<"' is not in its list of expected values";
                printWarning = true;
            }

            if(!out.node.defaultValue.empty() &&
               !isDefaultFound) {

                // WARN: missing default in values.
                ss << "the default value '" << out.node.defaultValue << "' of parameter '"<< out.node.name <<"' is not in its list of expected values";
                printWarning = true;
            }

            if (printWarning) {
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      LogicalErrorWarning,
                                                      sourceMap));
            }
        }
        
        /** Determine if a signature is a valid parameter*/
        static bool isValidParameterSignature(const mdp::ByteBuffer& signature) {
            
            mdp::ByteBuffer innerSignature = signature;
            innerSignature = TrimString(innerSignature);
            
            if (innerSignature.length() == 0) {
                return false; // Empty string, invalid
            }
            
            size_t firstSpace = innerSignature.find(" ");

            if (firstSpace == std::string::npos) {
                return RegexMatch(innerSignature, "^" PARAMETER_IDENTIFIER "$");
            }
            
            std::string paramName = innerSignature.substr(0, firstSpace);

            if (!RegexMatch(paramName, "^" PARAMETER_IDENTIFIER "$")) {
                return false; // Invalid param name
            }

            // Remove param name
            innerSignature = innerSignature.substr(firstSpace + 1);
            size_t descriptionPos = innerSignature.find(snowcrash::DescriptionIdentifier);

            // Remove description
            if (descriptionPos != std::string::npos) {
                innerSignature = innerSignature.substr(0, descriptionPos);
                innerSignature = TrimString(innerSignature);
            }
            
            size_t attributesPos = innerSignature.find("(");

            if (attributesPos != std::string::npos) {
                size_t endOfAttributesPos = innerSignature.find_last_of(")");

                if (endOfAttributesPos == std::string::npos) {
                    return false; // Expecting close of attributes
                }

                // Remove attributes
                innerSignature = innerSignature.substr(0, attributesPos);
                innerSignature = TrimString(innerSignature);
            }
            
            if (innerSignature.length() == 0) {
                return true;
            }
            
            if (innerSignature.substr(0,1) == "=") {
                innerSignature = innerSignature.substr(1);
                innerSignature = TrimString(innerSignature);

                if (innerSignature.length() == 0) {
                    return false; // No default value
                }

                if (innerSignature.substr(0,1) == "`" && innerSignature.substr(innerSignature.length()-1,1) == "`") {
                    return true;
                }
            }
            
            return false;
        }

    };

    /** Parameter Section Parser */
    typedef SectionParser<Parameter, ListSectionAdapter> ParameterParser;
}

#endif
