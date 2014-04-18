//
//  MarkdownParser.h
//  markdownparser
//
//  Created by Zdenek Nemec on 4/18/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef MARKDOWNPARSER_MARKDOWNPARSER_H
#define MARKDOWNPARSER_MARKDOWNPARSER_H

#include "ByteBuffer.h"
#include "ASTNode.h"

namespace mdp {
    
    /** 
     *  GitHub-flavored Markdown Parser
     */
    class MarkdownParser {
    public:
        MarkdownParser();
        MarkdownParser(const MarkdownParser&);
        MarkdownParser& operator=(const MarkdownParser&);
        
        /**
         *  \brief Parse source buffer
         *
         *  \param source   Markdown source data to be parsed
         *  \param ast      Parsed AST (root node)
         */
        void parse(const ByteBuffer& source, ASTNode& ast);
    };
}

#endif
