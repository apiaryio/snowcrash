//
//  MSONSectionProcessor.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/14/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONSECTIONPROCESSOR_H
#define SNOWCRASH_MSONSECTIONPROCESSOR_H

#include "SectionProcessor.h"

namespace scpl {

    using mdp::MarkdownNode;
    using mdp::MarkdownNodes;
    using mdp::MarkdownNodeIterator;

    /*
     * Forward Declarations
     */
    template<typename T>
    struct MSONSectionProcessor;

    /**
     * \brief  MSON Section Processor Base
     *
     * Defines default behaviours for Section Processor interface for MSON
     */
    template<typename T>
    struct MSONSectionProcessorBase : public snowcrash::SectionProcessorBase<T> {

        static const char EscapeCharacter = '`';
        static const char ValuesDelimiter = ':';
        static const char ValueDelimiter = ',';
        static const char AttributesDelimiter = '(';
        static const char ContentDelimiter = '-';

        /**
         * \brief Signature traits of the MSON section
         *
         * \return Signature traits given for MSON section
         */
        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(0);
            return signatureTraits;
        };

        /**
         * \brief Process MSON section signature markdown node (Default)
         */
        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     snowcrash::SectionParserData& pd,
                                                     snowcrash::SectionLayout& layout,
                                                     const snowcrash::ParseResultRef<T>& out) {

            // Get the signature traits of the section
            SignatureTraits signatureTraits = MSONSectionProcessor<T>::signatureTraits();

            // Parse Signature
            Signature signature = MSONSectionProcessor<T>::parseSignature(node, pd, signatureTraits, out);

            // Do section specific logic using the signature data
            MSONSectionProcessor<T>::finalizeSignature(node, pd, signature, out);

            return ++MarkdownNodeIterator(node);
        };

        /**
         * \brief Parse MSON section signature markdown node
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

            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);

            if (!subject.empty() &&
                subject[0] == ContentDelimiter) {

                // WARN: Signature is starting with a content
                out.report.warnings.push_back(snowcrash::Warning("signature should not start with a content",
                                                                 snowcrash::FormattingWarning,
                                                                 sourceMap));
            }

            if (traits.identifierTrait &&
                !subject.empty()) {

                parseSignatureIdentifier(sourceMap, out.report, subject, signature);
            }

            // Make sure values exist
            if (traits.valuesTrait &&
                !subject.empty() &&
                subject[0] != AttributesDelimiter) {

                // When subject starts with values, add a ':' for easier processing
                if (!traits.identifierTrait) {
                    subject = ValuesDelimiter + subject;
                }

                parseSignatureValues(sourceMap, out.report, subject, signature);
            }

            if (traits.attributesTrait &&
                !subject.empty() &&
                subject[0] != ContentDelimiter) {

                parseSignatureAttributes(sourceMap, out.report, subject, signature);
            }

            if (traits.contentTrait &&
                !subject.empty()) {

                parseSignatureContent(sourceMap, out.report, subject, signature);
            }

            return signature;
        };

        /**
         * \brief Use the signature data to do MSON section specific logic
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
         * \param sourceMap Source map of the node
         * \param report Parse Report
         * \param subject Signature of the section that needs to be parsed
         * \param out Signature data structure
         */
        static void parseSignatureIdentifier(const mdp::CharactersRangeSet sourceMap,
                                             snowcrash::Report& report,
                                             mdp::ByteBuffer& subject,
                                             Signature& out) {

            snowcrash::TrimString(subject);

            size_t end;
            bool isEnclosed = false;

            // If the identifier is enclosed, retrieve it
            if (subject[0] == '*' ||
                subject[0] == '_' ||
                subject[0] == EscapeCharacter) {

                mdp::ByteBuffer escapedString = retrieveEscaped(subject, 0, subject[0]);

                if (!escapedString.empty()) {
                    isEnclosed = true;
                    out.identifier = escapedString;
                }
            }

            snowcrash::TrimString(subject);

            // Look for next part of the signature section
            end = subject.find_first_of(ValuesDelimiter);

            if (end == std::string::npos) {
                end = subject.find_first_of(AttributesDelimiter);

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
                report.warnings.push_back(snowcrash::Warning("empty identifier",
                                                             snowcrash::EmptyDefinitionWarning,
                                                             sourceMap));
            }

            subject = subject.substr(end);
            snowcrash::TrimString(subject);
        };

        /**
         * \brief Parse the values from the signature
         */
        static void parseSignatureValues(const mdp::CharactersRangeSet sourceMap,
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
                        mdp::ByteBuffer escapedString = retrieveEscaped(subject, i);

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
                    } else if (subject[i] == AttributesDelimiter ||
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

                    // WARN: Empty identifier
                    report.warnings.push_back(snowcrash::Warning("empty values",
                                                                 snowcrash::EmptyDefinitionWarning,
                                                                 sourceMap));
                }

                snowcrash::TrimString(subject);
            }
        };

        /**
         * \brief Parse the attributes from the signature
         */
        static void parseSignatureAttributes(const mdp::CharactersRangeSet sourceMap,
                                             snowcrash::Report& report,
                                             mdp::ByteBuffer& subject,
                                             Signature& out) {

            if (subject[0] == AttributesDelimiter) {

                bool attributesNotFinished = true;

                // While there are attributes still left
                do {
                    // Retrieve attribute
                    mdp::ByteBuffer attribute = recursiveBracketMatch(subject, 0, ')', true);
                    size_t length = attribute.size();

                    // If the last char is not a comma, attributes are finished
                    if (attribute[length - 1] != ',') {
                        attributesNotFinished = false;
                    } else {
                        attribute = attribute.substr(0, length - 1);

                        // For easier processing
                        subject = "(" + subject;
                    }

                    snowcrash::TrimString(attribute);

                    if (!attribute.empty()) {
                        out.attributes.push_back(attribute);
                    }
                } while (attributesNotFinished);

                snowcrash::TrimString(subject);
            }
        };

        /**
         * \brief Find the matching bracket while doing the same for nested brackets and return
         *        the string until there while also cutting the signature that needs to be parsed
         *
         * \param subject The signature section that needs to be parsed
         * \param begin Character index representing the beginning of the bracket that needs to be matched
         * \param endBracket The type of bracket that needs to be matched
         * \param splitByComma If this is true, we need to return when we find a non-nested comma
         *
         * \return String inside the given brackets. If not splitting by comma, append the brackets too
         */
        static mdp::ByteBuffer recursiveBracketMatch(mdp::ByteBuffer& subject,
                                                     size_t begin,
                                                     const char endBracket,
                                                     const bool splitByComma = false) {

            size_t i = begin + 1;
            mdp::ByteBuffer returnString;

            // Append the beginning bracket
            if (!splitByComma) {
                returnString += subject[begin];
            }

            while (i < subject.length()) {

                if (subject[i] == EscapeCharacter) {

                    // If escaped string, retrieve it and cut it from subject
                    mdp::ByteBuffer escapedString = retrieveEscaped(subject, i);

                    returnString += escapedString;
                    i = 0;
                } else if (subject[i] == '[') {

                    returnString += recursiveBracketMatch(subject, i, ']');
                    i = 0;
                } else if (subject[i] == '(') {

                    returnString += recursiveBracketMatch(subject, i, ')');
                    i = 0;
                } else if (subject[i] == endBracket) {

                    // Append the ending bracket
                    if (!splitByComma) {
                        returnString += subject[i];
                    }

                    subject = subject.substr(i + 1);
                    break;
                } else if (splitByComma && subject[i] == ',') {

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

        /**
         * \brief Parse the content from the signature
         */
        static void parseSignatureContent(const mdp::CharactersRangeSet sourceMap,
                                          snowcrash::Report& report,
                                          mdp::ByteBuffer& subject,
                                          Signature& out) {

            if (subject[0] == ContentDelimiter) {
                subject = subject.substr(1);
                snowcrash::TrimString(subject);



                snowcrash::TrimString(subject);
            }
        };

        /**
         * \brief Retrieve strings enclosed by matching backticks
         *
         * \param subject Signature of the section that needs to be parsed
         * \param begin Character index representing the beginning of the escaped string
         * \param escapeChar Character used to escape the string
         *
         * \return Returns the escaped string, new subject will be from the end of the escaped string
         *
         * \example (begin = 1, escapeChar = "`", subject = "a```b```cd") ----> (return = "```b```", subject = "cd")
         */
        static mdp::ByteBuffer retrieveEscaped(mdp::ByteBuffer& subject,
                                               const size_t begin,
                                               const char escapeChar = EscapeCharacter) {

            size_t levels = 0, end;

            // Get the level of the backticks
            while (subject[levels + begin] == escapeChar) {
                levels++;
            }

            end = subject.substr(levels + begin).find(subject.substr(begin, levels));

            if (end == std::string::npos) {
                return "";
            }

            end = end + (2 * levels) + begin;

            mdp::ByteBuffer escapedString = subject.substr(begin, end - begin);
            subject = subject.substr(end);

            return escapedString;
        };
    };

    /**
     * Default MSON Section Processor
     */
    template<typename T>
    struct MSONSectionProcessor : public MSONSectionProcessorBase<T> {
    };
}

#endif
