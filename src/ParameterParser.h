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
#define PARAMETER_IDENTIFIER "`?(([[:alnum:]_.-])*|(%[A-Fa-f0-9]{2})*)+`?"

/** Lead in and out for comma separated values regex */
#define CSV_LEADINOUT "[[:blank:]]*,?[[:blank:]]*"

namespace snowcrash {

    /* We only allow at the maximum 3 attributes for old syntax parameters */
    const size_t OLD_SYNTAX_MAX_ATTRIBUTES = 3;

    /* We only allow at the maximum 2 attributes for new syntax parameters */
    const size_t NEW_SYNTAX_MAX_ATTRIBUTES = 2;

    /** Parameter description delimiter */
    const std::string DescriptionIdentifier = "...";

    /** Parameter Required matching regex */
    const char* const ParameterRequiredRegex = "^[[:blank:]]*[Rr]equired[[:blank:]]*$";

    /** Parameter Optional matching regex */
    const char* const ParameterOptionalRegex = "^[[:blank:]]*[Oo]ptional[[:blank:]]*$";

    /** Additonal Parameter Traits Example matching regex */
    const char* const AdditionalTraitsExampleRegex = "`([^`]*)`";

    /** Additonal Parameter Traits Use matching regex */
    const char* const AdditionalTraitsUseRegex = "([Oo]ptional|[Rr]equired)";

    /** Parameter Values matching regex */
    const char* const ParameterValuesRegex = "^[[:blank:]]*[Vv]alues[[:blank:]]*$";

    /** Values expected content */
    const char* const ExpectedValuesContent = "nested list of possible parameter values, one element per list item e.g. '`value`'";

    /** Additional Traits warning for old syntax */
    const char* const OldSyntaxAdditionalTraitsWarning = ", expected '([required | optional], [<type>], [`<example value>`])', e.g. '(optional, string, `Hello World`)'";

    /** Additional Traits warning for new syntax */
    const char* const NewSyntaxAdditionalTraitsWarning = ", expected '([required | optional], [<type> | enum[<type>])', e.g. '(optional, string)'";

    /* Type wrapped by enum matching regex */
    const char* const EnumRegex = "^enum\\[([^][]+)]$";

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
    struct SectionProcessor<Parameter> : public SignatureSectionProcessorBase<Parameter> {

        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(SignatureTraits::IdentifierTrait |
                                            SignatureTraits::ValuesTrait |
                                            SignatureTraits::AttributesTrait |
                                            SignatureTraits::ContentTrait,
                                            Delimiters('=', snowcrash::DescriptionIdentifier));

            return signatureTraits;
        }

        static MarkdownNodeIterator finalizeSignature(const MarkdownNodeIterator& node,
                                                      SectionParserData& pd,
                                                      const Signature& signature,
                                                      const ParseResultRef<Parameter>& out) {

            out.node.name = signature.identifier;
            out.node.description = signature.content;
            out.node.defaultValue = signature.value;

            if (!signature.remainingContent.empty()) {
                out.node.description += "\n" + signature.remainingContent + "\n";
            }

            SectionProcessor<Parameter>::parseAttributes(node, pd, signature.attributes, out);

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

            return ++MarkdownNodeIterator(node);
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<Parameter>& out) {

            checkDefaultAndRequiredClash<Parameter>(node, pd, out);
            checkExampleAndDefaultValue<Parameter>(node, pd, out);
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

                        if (RegexMatch(itSubject, ParameterValuesRegex)) {
                            return ParameterSectionType;
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

        template<typename T>
        static void parseAttributes(const mdp::MarkdownNodeIterator& node,
                                    SectionParserData& pd,
                                    const std::vector<mdp::ByteBuffer>& attributes,
                                    const ParseResultRef<T>& out,
                                    const bool oldSyntax = true) {

            out.node.use = UndefinedParameterUse;
            size_t max = oldSyntax ? OLD_SYNTAX_MAX_ATTRIBUTES : NEW_SYNTAX_MAX_ATTRIBUTES;

            if (attributes.size() > max) {
                return warnAboutAdditionalTraits(node, pd, out, oldSyntax);
            }

            bool definedUse = false;

            // Traverse over parameter's traits
            for (size_t i = 0; i < attributes.size(); i++) {
                CaptureGroups captureGroups;

                if (RegexMatch(attributes[i], ParameterOptionalRegex) && !definedUse) {
                    out.node.use = OptionalParameterUse;
                    definedUse = true;
                }
                else if (RegexMatch(attributes[i], ParameterRequiredRegex) && !definedUse) {
                    out.node.use = RequiredParameterUse;
                    definedUse = true;
                }
                else if (oldSyntax &&
                         RegexCapture(attributes[i], AdditionalTraitsExampleRegex, captureGroups) &&
                         captureGroups.size() > 1) {

                    out.node.exampleValue = captureGroups[1];
                }
                else {
                    if (!out.node.type.empty()) {
                        return warnAboutAdditionalTraits(node, pd, out, oldSyntax);
                    }

                    out.node.type = attributes[i];
                }
            }

            // For new syntax, Retrieve the type which is wrapped by enum[]
            if (!oldSyntax && !out.node.type.empty()) {
                std::string typeInsideEnum = RegexCaptureFirst(out.node.type, EnumRegex);
                TrimString(typeInsideEnum);

                if (!typeInsideEnum.empty()) {
                    out.node.type = typeInsideEnum;
                }
            }

            if (pd.exportSourceMap()) {
                if (!out.node.type.empty()) {
                    out.sourceMap.type.sourceMap = node->sourceMap;
                }

                if (definedUse) {
                    out.sourceMap.use.sourceMap = node->sourceMap;
                }

                if (oldSyntax && !out.node.exampleValue.empty()) {
                    out.sourceMap.exampleValue.sourceMap = node->sourceMap;
                }
            }
        }

        template<typename T>
        static void warnAboutAdditionalTraits(const mdp::MarkdownNodeIterator& node,
                                              SectionParserData& pd,
                                              const ParseResultRef<T>& out,
                                              bool oldSyntax) {

            // WARN: Additional parameters traits warning
            std::stringstream ss;
            ss << "unable to parse additional parameter traits";
            ss << (oldSyntax ? OldSyntaxAdditionalTraitsWarning : NewSyntaxAdditionalTraitsWarning);

            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
            out.report.warnings.push_back(Warning(ss.str(),
                                                  FormattingWarning,
                                                  sourceMap));

            out.node.type.clear();
            out.node.use = UndefinedParameterUse;

            if (pd.exportSourceMap()) {
                out.sourceMap.type.sourceMap.clear();
                out.sourceMap.use.sourceMap.clear();
            }

            // Clear example value for old syntax
            if (oldSyntax) {
                out.node.exampleValue.clear();

                if (pd.exportSourceMap()) {
                    out.sourceMap.exampleValue.sourceMap.clear();
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

            if ((out.node.exampleValue.empty() && out.node.defaultValue.empty()) ||
                out.node.values.empty()) {

                return;
            }

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

            if (RegexCapture(innerSignature, "^" PARAMETER_IDENTIFIER "[[:blank:]]*", captureGroups) &&
                !captureGroups[0].empty()) {

                innerSignature = innerSignature.substr(captureGroups[0].size());
                TrimString(innerSignature);
            }
            else {
                return NotParameterType;
            }

            // If contains only parameter name
            if (innerSignature.empty()) {
                return NewParameterType;
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

                mdp::ByteBuffer inner = matchBrackets(innerSignature, 0, ')');

                if (inner.length() == 2 || inner == innerSignature) {
                    return NotParameterType; // Expecting close of attributes
                }

                std::string attributes = inner.substr(1, inner.length()-1);

                if (RegexMatch(attributes, "enum\\[[^][]+]")) {
                    return NewParameterType;
                }

                if (RegexMatch(attributes, "`")) {
                    return OldParameterType;
                }

                TrimString(innerSignature);

                if (innerSignature.empty()) {
                    return NewParameterType;
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
