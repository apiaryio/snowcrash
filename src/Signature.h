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
     *  \return True if the node has a recognized keyword signature, false otherwise
     */
    extern bool HasSectionKeywordSignature(const mdp::MarkdownNodeIterator& node);

    /**
     *  \brief Recognize the type of section given the first line from a code block
     *  \param subject  The first line that needs to be recognized
     *  \return SectionType Type of the section if the line contains a keyword
     */
    extern SectionType RecognizeCodeBlockFirstLine(const mdp::ByteBuffer& subject);
}

#endif
