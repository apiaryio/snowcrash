//
//  Signature.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/14/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SIGNATURE_H
#define SNOWCRASH_SIGNATURE_H

#include "MarkdownNode.h"
#include "Section.h"

namespace snowcrash {

    /**
     *  \brief Query whether a node has keyword-defined signature.
     *  \param node     A Markdown AST node to check.
     *  \return Type of the node if it has a recognized keyword signature, UndefinedType otherwise
     */
    extern SectionType SectionKeywordSignature(const mdp::MarkdownNodeIterator& node);

    /**
     *  \brief Recognize the type of section given the first line from a code block
     *  \param subject  The first line that needs to be recognized
     *  \return SectionType Type of the section if the line contains a keyword
     */
    extern SectionType RecognizeCodeBlockFirstLine(const mdp::ByteBuffer& subject);
}

namespace scpl {

    /**
     * \brief Signature data after parsing section using signature traits
     */
    struct Signature {

        mdp::ByteBuffer identifier;              // Signature Identifier
        mdp::ByteBuffer value;                   // Signature Value (unparsed values)

        std::vector<mdp::ByteBuffer> values;     // Signature Values
        std::vector<mdp::ByteBuffer> attributes; // Signature Attributes

        mdp::ByteBuffer content;          // Signature content before newline character
        mdp::ByteBuffer remainingContent; // Signature content after newline character
    };

    /**
     * \brief Traits characteristics of a section signature
     *
     * Traits describe identifier defined or value/type defined signatures.
     */
    struct SignatureTraits {

        enum Trait {
            IdentifierTrait = (1 << 0), // Expect an identifier in the signature
            ValuesTrait     = (1 << 1), // Expect a (list of) value in the signature
            AttributesTrait = (1 << 2), // Expect a list of attributes in the signature
            ContentTrait    = (1 << 3)  // Expect inline description in the signature
        };

        typedef unsigned int Traits;

        const bool identifierTrait;
        const bool valuesTrait;
        const bool attributesTrait;
        const bool contentTrait;

        SignatureTraits(Traits traits_ = 0)
        :
        identifierTrait(traits_ & IdentifierTrait),
        valuesTrait(traits_ & ValuesTrait),
        attributesTrait(traits_ & AttributesTrait),
        contentTrait(traits_ & ContentTrait)
        {}
    };
}

#endif
