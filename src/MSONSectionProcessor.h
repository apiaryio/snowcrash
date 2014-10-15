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

            if (traits.identifierTrait) {
                parseSignatureIdentifier(sourceMap, out.report, subject, signature);
            }

            if (traits.valuesTrait) {
                parseSignatureValues(sourceMap, out.report, subject, signature);
            }

            if (traits.attributesTrait) {
                parseSignatureAttributes(sourceMap, out.report, subject, signature);
            }

            if (traits.contentTrait) {
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
         */
        static void parseSignatureIdentifier(const mdp::CharactersRangeSet sourceMap,
                                             snowcrash::Report& report,
                                             mdp::ByteBuffer& subject,
                                             Signature& out) {

            size_t end;
            bool isEnclosed = false;

            // If the identifier is enclosed, retrieve it
            if (subject[0] == '*' || subject[0] == '_') {
                end = subject.substr(1).find(subject[0]);

                if (end != std::string::npos) {
                    isEnclosed = true;
                    end = end + 2;

                    out.identifier = subject.substr(0, end);
                }
            } else if (subject[0] == '`') {
                int levels = 1;

                while (subject[levels] == '`') {
                    levels++;
                }

                end = subject.substr(levels).find(subject.substr(0, levels));

                if (end != std::string::npos) {
                    isEnclosed = true;
                    end = end + (2 * levels);

                    out.identifier = subject.substr(0, end);
                }
            }

            // Remove the enclosed identifier part
            if (isEnclosed) {
                subject = subject.substr(end);
                snowcrash::TrimString(subject);
            }

            end = subject.find_first_of(':');

            if (end == std::string::npos) {
                end = subject.find_first_of('(');

                if (end == std::string::npos) {
                    end = subject.find_first_of('-');

                    if (end == std::string::npos) {
                        end = subject.length();
                    }
                }
            }

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

            // TODO: Logic
        };

        /**
         * \brief Parse the attributes from the signature
         */
        static void parseSignatureAttributes(const mdp::CharactersRangeSet sourceMap,
                                             snowcrash::Report& report,
                                             mdp::ByteBuffer& subject,
                                             Signature& out) {

            // TODO: Logic
        };

        /**
         * \brief Parse the content from the signature
         */
        static void parseSignatureContent(const mdp::CharactersRangeSet sourceMap,
                                          snowcrash::Report& report,
                                          mdp::ByteBuffer& subject,
                                          Signature& out) {

            // TODO: Logic
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
