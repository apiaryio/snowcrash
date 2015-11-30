//
//  SignatureSectionProcessor.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/14/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SIGNATURESECTIONPROCESSOR_H
#define SNOWCRASH_SIGNATURESECTIONPROCESSOR_H

#include "SectionProcessor.h"

namespace scpl {

    using mdp::MarkdownNode;
    using mdp::MarkdownNodes;
    using mdp::MarkdownNodeIterator;

    const char EscapeCharacter = '`';
    const char ValuesDelimiter = ':';
    const char ValueDelimiter = ',';
    const char AttributesBeginDelimiter = '(';
    const char AttributesEndDelimiter = ')';
    const char AttributeDelimiter = ',';
    const char ContentDelimiter = '-';

    /**
     * \brief Signature Section Processor Base
     *
     * Defines default behaviours for Section Processor interface for Signature sections
     */
    template<typename T>
    struct SignatureSectionProcessorBase : public snowcrash::SectionProcessorBase<T> {

        /**
         * \brief Signature traits of the section
         *
         * \return Signature traits given for section
         */
        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits;
            return signatureTraits;
        };

        /**
         * \brief Process section signature markdown node (Default)
         */
        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     snowcrash::SectionParserData& pd,
                                                     snowcrash::SectionLayout& layout,
                                                     const snowcrash::ParseResultRef<T>& out) {

            // Get the signature traits of the section
            SignatureTraits signatureTraits = snowcrash::SectionProcessor<T>::signatureTraits();

            // Parse Signature
            Signature signature = snowcrash::SectionProcessor<T>::parseSignature(node, pd, signatureTraits, out.report);

            // Do section specific logic using the signature data
            return snowcrash::SectionProcessor<T>::finalizeSignature(node, pd, signature, out);
        };

        /**
         * \brief Parse section signature markdown node
         *
         * \param node Markdown node
         * \param pd Section Parser data
         * \param traits Signature traits of the section signature
         * \param report Parse result report of the section
         * \param subjectOrig The string that needs to be parsed
         *
         * \return Signature data
         */
        static Signature parseSignature(const MarkdownNodeIterator& node,
                                        snowcrash::SectionParserData& pd,
                                        const SignatureTraits& traits,
                                        snowcrash::Report& report,
                                        const mdp::ByteBuffer& subjectOrig = "") {

            Signature signature;
            mdp::ByteBuffer subject = subjectOrig;

            if (subject.empty()) {

                subject = snowcrash::GetFirstLine(node->text, signature.remainingContent);
                snowcrash::TrimString(subject);
            }

            if (traits.identifierTrait &&
                !subject.empty()) {

                parseSignatureIdentifier(traits, report, subject, signature);

                if (signature.identifier.empty()) {

                    // WARN: Empty identifier
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                    report.warnings.push_back(snowcrash::Warning("no identifier specified",
                                                                 snowcrash::EmptyDefinitionWarning,
                                                                 sourceMap));
                }
            }

            // Make sure values exist
            if (traits.valuesTrait &&
                !subject.empty() &&
                subject[0] != AttributesBeginDelimiter) {

                // When subject starts with values, add a ':' for easier processing
                if (!traits.identifierTrait) {
                    subject = ValuesDelimiter + subject;
                }

                if (subject[0] == ValuesDelimiter) {

                    parseSignatureValues(traits, report, subject, signature);

                    if (signature.values.empty()) {

                        // WARN: Empty values
                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                        report.warnings.push_back(snowcrash::Warning("no value(s) specified",
                                                                     snowcrash::EmptyDefinitionWarning,
                                                                     sourceMap));
                    }
                }
            }

            if (traits.attributesTrait &&
                !subject.empty() &&
                subject[0] != ContentDelimiter) {

                parseSignatureAttributes(report, subject, signature);
            }

            if (traits.contentTrait &&
                !subject.empty() &&
                subject[0] == ContentDelimiter) {

                subject = subject.substr(1);
                snowcrash::TrimString(subject);

                signature.content = subject;
            }

            return signature;
        };

        /**
         * \brief Use the signature data to do section specific logic
         *
         * \param node Markdown node
         * \param pd Section Parser data
         * \param signature Signature data
         * \param out Parse result of the section
         *
         * \return Result of process operation
         */
        static MarkdownNodeIterator finalizeSignature(const MarkdownNodeIterator& node,
                                                      snowcrash::SectionParserData& pd,
                                                      const Signature& signature,
                                                      const snowcrash::ParseResultRef<T>& out) {

            return ++MarkdownNodeIterator(node);
        };

        /**
         * \brief Parse the identifier from the signature
         *
         * \param traits Signature traits of the section signature
         * \param report Parse Report
         * \param subject String that needs to be parsed
         *                (which will be stripped of the parsed characters)
         * \param out Signature data structure
         */
        static void parseSignatureIdentifier(const SignatureTraits& traits,
                                             snowcrash::Report& report,
                                             mdp::ByteBuffer& subject,
                                             Signature& out) {

            snowcrash::TrimString(subject);

            // `*` and `_` are markdown characters for emphasis
            std::string escapeCharacters = "*_`";

            size_t i = 0;
            mdp::ByteBuffer identifier;

            // Traverse over the string
            while (i < subject.length()) {

                if (escapeCharacters.find(subject[i]) != std::string::npos) {

                    // If escaped string, retrieve it and strip it from the subject
                    mdp::ByteBuffer escapedString = snowcrash::RetrieveEscaped(subject, i);

                    if (!escapedString.empty()) {
                        identifier += escapedString;
                        i = 0;
                    } else {
                        identifier += subject[i];
                        i++;
                    }
                } else if ((traits.valuesTrait && subject[i] == ValuesDelimiter) ||
                           (traits.attributesTrait && subject[i] == AttributesBeginDelimiter) ||
                           (traits.contentTrait && subject[i] == ContentDelimiter)) {

                    // If identifier ends, strip it from the subject
                    subject = subject.substr(i);
                    i = 0;
                    break;
                } else {

                    identifier += subject[i];
                    i++;
                }
            }

            // Assign the identifier
            snowcrash::TrimString(identifier);

            if (!identifier.empty()) {
                out.identifier = snowcrash::StripBackticks(identifier);
            }

            // If the subject ended with the identifier, strip it from the subject
            if (i == subject.length()) {
                subject = "";
            }

            snowcrash::TrimString(subject);
        };

        /**
         * \brief Parse the values from the signature
         *
         * \param traits Signature traits of the section signature
         * \param report Parse Report
         * \param subject String that needs to be parsed
         *                (which will be stripped of the parsed characters)
         * \param out Signature data structure
         */
        static void parseSignatureValues(const SignatureTraits& traits,
                                         snowcrash::Report& report,
                                         mdp::ByteBuffer& subject,
                                         Signature& out) {

            // Remove leading delimiter
            subject = subject.substr(1);
            snowcrash::TrimString(subject);

            size_t i = 0;
            mdp::ByteBuffer value;

            out.value = subject;

            // Traverse over the string
            while (i < subject.length()) {

                if (subject[i] == EscapeCharacter) {

                    // If escaped string, retrieve it and strip it from subject
                    mdp::ByteBuffer escapedString = snowcrash::RetrieveEscaped(subject, i);

                    if (!escapedString.empty()) {
                        value += escapedString;
                        i = 0;
                    } else {
                        value += subject[i];
                        i++;
                    }
                } else if (subject[i] == ValueDelimiter) {

                    // If found value delimiter, add the value and strip it from subject
                    subject = subject.substr(i + 1);
                    snowcrash::TrimString(subject);

                    snowcrash::TrimString(value);
                    out.values.push_back(snowcrash::StripBackticks(value));

                    value = "";
                    i = 0;
                } else if ((traits.attributesTrait && subject[i] == AttributesBeginDelimiter) ||
                           (traits.contentTrait && subject[i] == ContentDelimiter)) {

                    // If values section ends, strip it from subject
                    subject = subject.substr(i);
                    i = 0;
                    break;
                } else {

                    value += subject[i];
                    i++;
                }
            }

            // Add the value at the end of values section if present
            snowcrash::TrimString(value);

            if (!value.empty()) {
                out.values.push_back(snowcrash::StripBackticks(value));
            }

            // If the subject ended with the values, strip the last value from the subject
            if (i == subject.length()) {
                subject = "";
            }

            snowcrash::TrimString(subject);

            // Fill signature value with the string which was stripped from subject
            out.value = out.value.substr(0, out.value.length() - subject.length());

            snowcrash::TrimString(out.value);
            out.value = snowcrash::StripBackticks(out.value);
        };

        /**
         * \brief Parse the attributes from the signature
         *
         * \param report Parse Report
         * \param subject String that needs to be parsed
         *                (which will be stripped of the parsed characters)
         * \param out Signature data structure
         */
        static void parseSignatureAttributes(snowcrash::Report& report,
                                             mdp::ByteBuffer& subject,
                                             Signature& out) {

            if (subject[0] != AttributesBeginDelimiter) {
                return;
            }

            bool attributesNotFinished = true;

            // While there are attributes still left
            while (attributesNotFinished) {

                // Retrieve attribute
                mdp::ByteBuffer attribute = matchBrackets(subject, 0, AttributesEndDelimiter, true);
                size_t length = attribute.size();

                // If the last char is not an attribute delimiter, attributes are finished
                if (attribute[length - 1] != AttributeDelimiter) {
                    attributesNotFinished = false;
                } else {
                    attribute = attribute.substr(0, length - 1);

                    // For easier processing
                    subject = AttributesBeginDelimiter + subject;
                }

                snowcrash::TrimString(attribute);

                if (!attribute.empty()) {
                    out.attributes.push_back(attribute);
                }
            };

            snowcrash::TrimString(subject);
        };

        /**
         * \brief Find the matching bracket while ignoring any nested brackets and return the string
         *        enclosed by them, while also stripping the string that needs to be parsed
         *
         * \param subject The string that needs to be parsed
         * \param begin Character index representing the beginning of the bracket that needs to be matched
         * \param endBracket The type of bracket that needs to be matched
         * \param splitByAttribute If this is true, we need to return when we find a top-level attribute delimiter
         * \param clearAtEnd If this is true, the string will be cleared at the end of parsing
         *
         * \return String inside the given brackets. If not splitting by comma, append the brackets too
         */
        static mdp::ByteBuffer matchBrackets(mdp::ByteBuffer& subject,
                                             size_t begin,
                                             const char endBracket,
                                             const bool splitByAttribute = false,
                                             const bool clearAtEnd = false) {

            size_t i = begin + 1;
            mdp::ByteBuffer returnString;

            // Append the beginning bracket
            if (!splitByAttribute) {
                returnString += subject[begin];
            }

            while (i < subject.length()) {

                if (subject[i] == EscapeCharacter) {

                    // If escaped string, retrieve it and strip it from subject
                    mdp::ByteBuffer escapedString = snowcrash::RetrieveEscaped(subject, i);

                    if (!escapedString.empty()) {
                        returnString += escapedString;
                        i = 0;
                    } else {
                        returnString += subject[i];
                        i++;
                    }
                } else if (subject[i] == '[') {

                    returnString += matchBrackets(subject, i, ']', false, true);
                    i = 0;
                } else if (subject[i] == '(') {

                    returnString += matchBrackets(subject, i, ')', false, true);
                    i = 0;
                } else if (subject[i] == endBracket) {

                    // Append the ending bracket
                    if (!splitByAttribute) {
                        returnString += subject[i];
                    }

                    subject = subject.substr(i + 1);
                    break;
                } else if (splitByAttribute && subject[i] == AttributeDelimiter) {

                    // Return when encountering comma
                    returnString += subject[i];
                    subject = subject.substr(i + 1);
                    break;
                } else {

                    returnString += subject[i];
                    i++;
                }
            }

            if (i == subject.length() && clearAtEnd) {
                subject.clear();
            }

            return returnString;
        }
    };
}

#endif
