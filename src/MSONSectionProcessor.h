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

            if (traits.identifierTrait) {
                parseIdentifier(subject, signature);
            }

            // TODO: Other traits

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
        static void parseIdentifier(const mdp::ByteBuffer& subject,
                                    const Signature& out) {

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
