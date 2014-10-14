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
         */
        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(0);
            return signatureTraits;
        };

        /**
         * \brief Process MSON section signature markdown node
         */
        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     snowcrash::SectionParserData& pd,
                                                     snowcrash::SectionLayout& layout,
                                                     const snowcrash::ParseResultRef<T>& out) {

            Signature signature;

            mdp::ByteBuffer subject;
            subject = snowcrash::GetFirstLine(node->text, signature.remainingContent);

            snowcrash::TrimString(subject);

            // Get the signature traits of the section
            SignatureTraits signatureTraits = MSONSectionProcessor<T>::signatureTraits();

            // Do section specific logic using the signature data
            MSONSectionProcessor<T>::finalizeSignature(node, pd, signature, out);

            return ++MarkdownNodeIterator(node);
        };

        /**
         * \brief Use the signature data to do MSON section specific logic
         *
         * \param signature Signature data
         */
        static void finalizeSignature(const MarkdownNodeIterator& node,
                                      snowcrash::SectionParserData& pd,
                                      const Signature& signature,
                                      const snowcrash::ParseResultRef<T>& out) {};
    };

    /**
     * Default MSON Section Processor
     */
    template<typename T>
    struct MSONSectionProcessor : public MSONSectionProcessorBase<T> {
    };
}

#endif
