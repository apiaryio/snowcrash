//
//  MarkdownBlock.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/15/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MARKDOWBLOCK_H
#define SNOWCRASH_MARKDOWBLOCK_H

#include <vector>
#include <string>
#include "ParserCore.h"

namespace snowcrash {
    
    /** Type of a Markdown Block */
    enum MarkdownBlockType {
        UndefinedBlockType = 0,
        CodeBlockType = 1,
        QuoteBlockBeginType = 2,
        QuoteBlockEndType = 3,
        HTMLBlockType = 4,
        HeaderBlockType = 5,
        HRuleBlockType = 6,
        ListBlockBeginType = 7,
        ListBlockEndType = 8,
        ListItemBlockBeginType = 9,
        ListItemBlockEndType = 10,
        ParagraphBlockType = 11,
        TableBlockType = 12,
        TableRowBlockType = 13,
        TableCellBlockType = 14
    };
    
    /**
     *  \return Name of the markdown block.
     */
    std::string BlockName(const MarkdownBlockType& blockType);

    /**
     *  Markdown Block Element
     */
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

    /**
     *  \brief  Maps block of source data bytes into source string.
     *  \param  block   A %SourceDataBlock to map into %SourceCharactersBlock
     *  \param  data    A mapped source data.
     *  \returns A string concatenated from the source string using the byte map.
     */
    std::string MapSourceData(const SourceData& source, const SourceDataBlock& sourceMap);
        
#ifdef DEBUG
    // Prints markdown block recursively to stdout
    void printMarkdown(const MarkdownBlock::Stack& markdown, unsigned int level = 0);
#endif

}

#endif
