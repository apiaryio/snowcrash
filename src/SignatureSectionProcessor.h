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

    /*
     * Forward Declarations
     */
    template<typename T>
    struct SignatureSectionProcessor;

    /**
     * \brief Signature Section Processor Base
     *
     * Defines default behaviours for Section Processor interface for Signature sections
     */
    template<typename T>
    struct SignatureSectionProcessorBase : public snowcrash::SectionProcessorBase<T> {

        static const char EscapeCharacter = '`';
        static const char ValuesDelimiter = ':';
        static const char ValueDelimiter = ',';
        static const char AttributesStartDelimiter = '(';
        static const char AttributesEndDelimiter = ')';
        static const char AttributeDelimiter = ',';
        static const char ContentDelimiter = '-';

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
            SignatureTraits signatureTraits = SignatureSectionProcessor<T>::signatureTraits();

            // Parse Signature
            Signature signature = SignatureSectionProcessor<T>::parseSignature(node, pd, signatureTraits, out);

            // Do section specific logic using the signature data
            SignatureSectionProcessor<T>::finalizeSignature(node, pd, signature, out);

            return ++MarkdownNodeIterator(node);
        };

        /**
         * \brief Parse section signature markdown node
         *
         * \param node Markdown node
         * \param pd Section Parser data
         * \param traits Signature traits of the section signature
         * \param out Parse result of the section
         *
         * \return Signature data
         */
        static Signature parseSignature(const MarkdownNodeIterator& node,
                                        snowcrash::SectionParserData& pd,
                                        const SignatureTraits& traits,
                                        const snowcrash::ParseResultRef<T>& out) {

            Signature signature;
            mdp::ByteBuffer subject;

            subject = snowcrash::GetFirstLine(node->text, signature.remainingContent);
            snowcrash::TrimString(subject);

            if (traits.identifierTrait &&
                !subject.empty()) {

                parseSignatureIdentifier(node, pd, out.report, subject, signature);
            }

            // Make sure values exist
            if (traits.valuesTrait &&
                !subject.empty() &&
                subject[0] != AttributesStartDelimiter) {

                // When subject starts with values, add a ':' for easier processing
                if (!traits.identifierTrait) {
                    subject = ValuesDelimiter + subject;
                }

                parseSignatureValues(node, pd, out.report, subject, signature);
            }

            if (traits.attributesTrait &&
                !subject.empty() &&
                subject[0] != ContentDelimiter) {

                parseSignatureAttributes(node, pd, out.report, subject, signature);
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
         */
        static void finalizeSignature(const MarkdownNodeIterator& node,
                                      snowcrash::SectionParserData& pd,
                                      const Signature& signature,
                                      const snowcrash::ParseResultRef<T>& out) {};

        /**
         * \brief Parse the identifier from the signature
         *
         * \param node Markdown node
         * \param pd Section Parser data
         * \param report Parse Report
         * \param subject String that needs to be parsed
         *                (which will be stripped of the parsed characters)
         * \param out Signature data structure
         */
        static void parseSignatureIdentifier(const MarkdownNodeIterator& node,
                                             snowcrash::SectionParserData& pd,
                                             snowcrash::Report& report,
                                             mdp::ByteBuffer& subject,
                                             Signature& out) {

            snowcrash::TrimString(subject);

            // `*` and `_` are markdown characters for emphasis
            std::string escapeCharacters = "*_`";
            bool isEnclosed = false;

            // If the identifier is enclosed, retrieve it
            if (escapeCharacters.find(subject[0]) != std::string::npos) {

                mdp::ByteBuffer escapedString = snowcrash::RetrieveEscaped(subject, 0);

                if (!escapedString.empty()) {
                    isEnclosed = true;
                    out.identifier = escapedString;
                }
            }

            snowcrash::TrimString(subject);

            // Look for next part of the signature section
            size_t end = subject.find_first_of(ValuesDelimiter);

            if (end == std::string::npos) {
                end = subject.find_first_of(AttributesStartDelimiter);

                if (end == std::string::npos) {
                    end = subject.find_first_of(ContentDelimiter);

                    if (end == std::string::npos) {
                        end = subject.length();
                    }
                }
            }

            // Retrieve the identifier if not done above
            if (end > 1) {
                if (isEnclosed) {

                    // WARN: Extraneous content after enclosed identifier
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(snowcrash::Warning("identifier not formatted correctly",
                                                                 snowcrash::FormattingWarning,
                                                                 sourceMap));
                } else {
                    out.identifier = subject.substr(0, end);
                    snowcrash::TrimString(out.identifier);
                }
            }

            if (out.identifier.empty()) {

                // WARN: Empty identifier
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(snowcrash::Warning("empty identifier",
                                                             snowcrash::EmptyDefinitionWarning,
                                                             sourceMap));
            }

            subject = subject.substr(end);
            snowcrash::TrimString(subject);
        };

        /**
         * \brief Parse the values from the signature
         *
         * \param node Markdown node
         * \param pd Section Parser data
         * \param report Parse Report
         * \param subject String that needs to be parsed
         *                (which will be stripped of the parsed characters)
         * \param out Signature data structure
         */
        static void parseSignatureValues(const MarkdownNodeIterator& node,
                                         snowcrash::SectionParserData& pd,
                                         snowcrash::Report& report,
                                         mdp::ByteBuffer& subject,
                                         Signature& out) {

            if (subject[0] == ValuesDelimiter) {
                subject = subject.substr(1);
                snowcrash::TrimString(subject);

                size_t i = 0;
                mdp::ByteBuffer value;

                // Traverse over the string
                while (i < subject.length()) {

                    if (subject[i] == EscapeCharacter) {

                        // If escaped string, retrieve it and cut it from subject
                        mdp::ByteBuffer escapedString = snowcrash::RetrieveEscaped(subject, i);

                        value += escapedString;
                        i = 0;
                    } else if (subject[i] == ValueDelimiter) {

                        // If found value delimiter, add the value and cut it from subject
                        subject = subject.substr(i + 1);
                        snowcrash::TrimString(subject);

                        snowcrash::TrimString(value);
                        out.values.push_back(value);

                        value = "";
                        i = 0;
                    } else if (subject[i] == AttributesStartDelimiter ||
                               subject[i] == ContentDelimiter) {

                        // If values section ends, cut it from subject
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
                    out.values.push_back(value);
                }

                // If the subject ended with the values, cut the last value from the subject
                if (i == subject.length()) {
                    subject = "";
                }

                if (out.values.empty()) {

                    // WARN: Empty values
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(snowcrash::Warning("empty values",
                                                                 snowcrash::EmptyDefinitionWarning,
                                                                 sourceMap));
                }

                snowcrash::TrimString(subject);
            }
        };

        /**
         * \brief Parse the attributes from the signature
         *
         * \param node Markdown node
         * \param pd Section Parser data
         * \param report Parse Report
         * \param subject String that needs to be parsed
         *                (which will be stripped of the parsed characters)
         * \param out Signature data structure
         */
        static void parseSignatureAttributes(const MarkdownNodeIterator& node,
                                             snowcrash::SectionParserData& pd,
                                             snowcrash::Report& report,
                                             mdp::ByteBuffer& subject,
                                             Signature& out) {

            if (subject[0] != AttributesStartDelimiter) {
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
                    subject = AttributesStartDelimiter + subject;
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
         *
         * \return String inside the given brackets. If not splitting by comma, append the brackets too
         */
        static mdp::ByteBuffer matchBrackets(mdp::ByteBuffer& subject,
                                             size_t begin,
                                             const char endBracket,
                                             const bool splitByAttribute = false) {

            size_t i = begin + 1;
            mdp::ByteBuffer returnString;

            // Append the beginning bracket
            if (!splitByAttribute) {
                returnString += subject[begin];
            }

            while (i < subject.length()) {

                if (subject[i] == EscapeCharacter) {

                    // If escaped string, retrieve it and cut it from subject
                    mdp::ByteBuffer escapedString = snowcrash::RetrieveEscaped(subject, i);

                    returnString += escapedString;
                    i = 0;
                } else if (subject[i] == '[') {

                    returnString += matchBrackets(subject, i, ']');
                    i = 0;
                } else if (subject[i] == '(') {

                    returnString += matchBrackets(subject, i, ')');
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

            return returnString;
        }
    };

    /**
     * Default Signature Section Processor
     */
    template<typename T>
    struct SignatureSectionProcessor : public SignatureSectionProcessorBase<T> {
    };
}

#endif
