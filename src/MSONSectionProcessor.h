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

namespace mson {

    using mdp::MarkdownNode;
    using mdp::MarkdownNodes;
    using mdp::MarkdownNodeIterator;

    /*
     * Forward Declarations
     */
    template<typename T>
    struct SectionProcessor;

    /**
     * \brief  MSON Section Processor Base
     *
     * Defines default behaviours for Section Processor interface for MSON
     */
    template<typename T>
    struct SectionProcessorBase : public snowcrash::SectionProcessorBase<T> {

        /**
         * \brief Signature traits of the MSON section
         */
        static scpl::SignatureTraits signatureTraits() {

            scpl::SignatureTraits signatureTraits(0);
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

            return ++MarkdownNodeIterator(node);
        }
    };

    /**
     * Default Section Processor
     */
    template<typename T>
    struct SectionProcessor : public SectionProcessorBase<T> {
    };
}

#endif
