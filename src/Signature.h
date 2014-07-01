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

namespace snowcrash {
    
    /** 
     *  \brief Query whether a node has keyword-defined signature.
     *  \param node     A Markdown AST node to check.
     *  \return True if the node has a recognized keyword signature, false otherwise
     */
    extern bool HasSectionKeywordSignature(const mdp::MarkdownNodeIterator& node);
}

#endif
