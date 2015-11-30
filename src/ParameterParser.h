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
#include "MSONTypeSectionParser.h"
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

    /** Parameter Definition Type */
    enum ParameterType {
        NotParameterType = 0,
        OldParameterType,        /// Parameter defined using the old syntax
        NewParameterType,        /// Parameter defined using the new MSON-like syntax
        UndefinedParameterType = -1
    };

    /**
     * Parameter section processor
     */
    template<>
    struct SectionProcessor<Parameter> : public SectionProcessorBase<Parameter> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     const ParseResultRef<Parameter>& out) {

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
                                                         const ParseResultRef<Parameter>& out) {

            if (pd.sectionContext() != ValuesSectionType) {
                return node;
            }

            // Check redefinition
            if (!out.node.values.empty()) {
                // WARN: parameter values are already defined
                std::stringstream ss;
                ss << "overshadowing previous 'values' definition";
                ss << " for parameter '" << out.node.name << "'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      RedefinitionWarning,
                                                      sourceMap));
            }

            // Clear any previous values
            out.node.values.clear();

            if (pd.exportSourceMap()) {
                out.sourceMap.values.collection.clear();
            }

            ParseResultRef<Values> values(out.report, out.node.values, out.sourceMap.values);
            ValuesParser::parse(node, siblings, pd, values);

            if (out.node.values.empty()) {
                // WARN: empty definition
                std::stringstream ss;
                ss << "no possible values specified for parameter '" << out.node.name << "'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      EmptyDefinitionWarning,
                                                      sourceMap));
            }

            if ((!out.node.exampleValue.empty() || !out.node.defaultValue.empty()) &&
                 !out.node.values.empty()) {

                checkExampleAndDefaultValue<Parameter>(node, pd, out);
            }

            return ++MarkdownNodeIterator(node);
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {

                mdp::ByteBuffer subject, remainingContent;
                subject = GetFirstLine(node->children().front().text, remainingContent);
                TrimString(subject);

                // Look ahead into nested list items
                for (MarkdownNodeIterator it = node->children().begin();
                     it != node->children().end();
                     ++it) {

                    if (it->type == mdp::ListItemMarkdownNodeType
                        && !it->children().empty()) {

                        mdp::ByteBuffer itSubject, itRemainingContent;
                        itSubject = GetFirstLine(it->children().front().text, itRemainingContent);
                        TrimString(itSubject);

                        if (RegexMatch(itSubject, MSONDefaultTypeSectionRegex) ||
                            RegexMatch(itSubject, MSONSampleTypeSectionRegex) ||
                            RegexMatch(itSubject, MSONValueMembersTypeSectionRegex)) {

                            return MSONParameterSectionType;
                        }
                    }
                }

                ParameterType parameterType = getParameterType(subject);

                if (parameterType == OldParameterType) {
                    return ParameterSectionType;
                }

                if (parameterType == NewParameterType) {
                    return MSONParameterSectionType;
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
                                   const ParseResultRef<Parameter>& out) {

            out.node.use = UndefinedParameterUse;
            TrimString(signature);

            if (pd.sectionContext() == ParameterSectionType) {

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

                checkDefaultAndRequiredClash<Parameter>(node, pd, out);
            }
            else {
                // ERR: unable to parse
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.error = Error("unable to parse parameter specification",
                                         ApplicationError,
                                         sourceMap);
            }
        }

        static void parseAdditionalTraits(const mdp::MarkdownNodeIterator& node,
                                          SectionParserData& pd,
                                          mdp::ByteBuffer& traits,
                                          const ParseResultRef<Parameter>& out) {

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

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
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

        template<typename T>
        static void checkDefaultAndRequiredClash(const mdp::MarkdownNodeIterator& node,
                                                 SectionParserData& pd,
                                                 const ParseResultRef<T>& out) {

            // Check possible required vs default clash
            if (out.node.use != OptionalParameterUse &&
                !out.node.defaultValue.empty()) {

                // WARN: Required vs default clash
                std::stringstream ss;
                ss << "specifying parameter '" << out.node.name << "' as required supersedes its default value"\
                ", declare the parameter as 'optional' to specify its default value";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      LogicalErrorWarning,
                                                      sourceMap));
            }
        }

        template<typename T>
        static void checkExampleAndDefaultValue(const mdp::MarkdownNodeIterator& node,
                                                SectionParserData& pd,
                                                const ParseResultRef<T>& out) {

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
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      LogicalErrorWarning,
                                                      sourceMap));
            }
        }

        /**
         * \brief Determine the type of parameter using the signature
         */
        static ParameterType getParameterType(const mdp::ByteBuffer& signature) {

            mdp::ByteBuffer innerSignature = signature;
            CaptureGroups captureGroups;

            TrimString(innerSignature);

            if (innerSignature.empty()) {
                return NotParameterType; // Empty string, invalid
            }

            if (RegexCapture(innerSignature, "^" PARAMETER_IDENTIFIER "[[:blank:]]*:?", captureGroups) &&
                !captureGroups[0].empty()) {

                innerSignature = innerSignature.substr(captureGroups[0].size());

                // If last char is ':', don't strip it from signature
                if (captureGroups[0].substr(captureGroups[0].size() - 1) == ":") {
                    innerSignature = ":" + innerSignature;
                }

                TrimString(innerSignature);
            }
            else {
                return NotParameterType;
            }

            // If contains only parameter name
            if (innerSignature.empty()) {
                return OldParameterType;
            }

            std::string firstChar = innerSignature.substr(0, 1);

            // If first char is ':' or `=`
            if (firstChar == ":" || firstChar == "=") {

                innerSignature = innerSignature.substr(1);
                TrimString(innerSignature);

                if (innerSignature.empty()) {
                    return NotParameterType; // No sample or default value
                }

                // Traverse over the value
                while (!innerSignature.empty()) {

                    std::string first = innerSignature.substr(0, 1);

                    if (first == "`") {
                        std::string escaped = RetrieveEscaped(innerSignature);

                        // If empty value is returned
                        if (escaped.empty()) {
                            innerSignature.clear();
                        }

                        TrimString(innerSignature);
                    }
                    else if (first == "(") {
                        break;
                    }
                    else {
                        innerSignature = innerSignature.substr(1);
                    }
                }

                return firstChar == ":" ? NewParameterType : OldParameterType;
            }

            if (innerSignature.substr(0, 1) == "(") {

                // We should use `matchBrackets` if the parameters are supported to be more complex
                size_t endOfAttributesPos = innerSignature.find_last_of(")");

                if (endOfAttributesPos == std::string::npos) {
                    return NotParameterType; // Expecting close of attributes
                }

                std::string attributes = innerSignature.substr(1, endOfAttributesPos);

                if (RegexMatch(attributes, "enum\\[[^][]+]")) {
                    return NewParameterType;
                }

                if (RegexMatch(attributes, "`")) {
                    return OldParameterType;
                }

                innerSignature = innerSignature.substr(endOfAttributesPos + 1);
                TrimString(innerSignature);

                if (innerSignature.empty()) {
                    return OldParameterType;
                }
            }

            if (innerSignature.substr(0, 1) == "-") {
                return NewParameterType;
            }

            if (innerSignature.substr(0, 3) == "...") {
                return OldParameterType;
            }

            return NotParameterType;
        }
    };

    /** Parameter Section Parser */
    typedef SectionParser<Parameter, ListSectionAdapter> ParameterParser;
}

#endif
