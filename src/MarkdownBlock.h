//
//  MarkdownBlock.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/15/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_MARKDOWBLOCK_H
#define SNOWCRASH_MARKDOWBLOCK_H

#include <vector>
#include <string>
#include "ParserCore.h"

namespace snowcrash {
    
    //
    // Markdown Block Type
    //
    enum MarkdownBlockType {
        UndefinedBlockType = 0,
        CodeBlockType,
        QuoteBlockBeginType,
        QuoteBlockEndType,
        HTMLBlockType,
        HeaderBlockType,
        HRuleBlockType,
        ListBlockBeginType,
        ListBlockEndType,
        ListItemBlockBeginType,
        ListItemBlockEndType,
        ParagraphBlockType,
        TableBlockType,
        TableRowBlockType,
        TableCellBlockType
    };

    //
    // Markdown Block Element
    //
    struct MarkdownBlock {

        typedef std::vector<MarkdownBlock> Stack;
        typedef SourceData Content;
        typedef int Data;

        MarkdownBlock()
        : type(UndefinedBlockType), data(0) {}
        
        MarkdownBlock(MarkdownBlockType t, Content c = Content(), Data d = Data(), SourceDataBlock map = SourceDataBlock())
        { type = t; content = c; data = d; sourceMap = map; }
        
        MarkdownBlock(const MarkdownBlock& b)
        { type = b.type; content = b.content; data = b.data; sourceMap = b.sourceMap; }
        
        virtual ~MarkdownBlock() {}
        
        MarkdownBlock& operator=(const MarkdownBlock& b)
        { type = b.type; content = b.content; data = b.data; sourceMap = b.sourceMap; return *this; }
        
        // Type of the Markdown Block
        MarkdownBlockType type;
        
        // Text content excluding nested blocks
        Content content;
        
        // Arbitrary data from parser
        Data data;
        
        // Position of this block in source module
        SourceDataBlock sourceMap;
    };

    // Return source data using from source and source map
    std::string MapSourceData(const SourceData& source, const SourceDataBlock& sourceMap);
        
#ifdef DEBUG
    // Prints markdown block recursively to stdout
    void printMarkdown(const MarkdownBlock::Stack& markdown, unsigned int level = 0);
#endif

}

#endif
