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

namespace snowcrash {

    // Markdown source data
    using SourceData = std::string;

    // Range of source data
    struct SourceDataRange {
        size_t location;
        size_t length;
    };
    using SourceDataBlock = std::vector<SourceDataRange>;
    void AppendSourceDataBlock(SourceDataBlock& destination, const SourceDataBlock& append);
    
    //
    // Markdown Block Type
    //
    enum class MarkdownBlockType : int {
        Undefined = 0,
        Code,
        Quote,
        HTML,
        Header,
        HRule,
        List,
        ListItem,
        Paragraph,
        Table,
        TableRow,
        TableCell
    };

    //
    // Markdown Block Element
    //
    struct MarkdownBlock {

        using Stack = std::vector<MarkdownBlock>;
        using Content = SourceData;
        using Data = int;
        
        MarkdownBlock()
        : type(MarkdownBlockType::Undefined), data(0) {}
        
        MarkdownBlock(MarkdownBlockType t, Content c = Content(), Data d = Data())
        : MarkdownBlock() { type = t; content = c; data = d; }
        
        MarkdownBlock(const MarkdownBlock& b) = default;
        
        MarkdownBlock(MarkdownBlock&& b) = default;
        
        virtual ~MarkdownBlock() = default;
        
        MarkdownBlock& operator=(const MarkdownBlock& b) = default;
        
        // Type of the Markdown Block
        MarkdownBlockType type;
        
        // Text content excluding nested blocks
        Content content;
        
        // Arbitrary data from parser
        Data data;
        
        // Nested blocks (Quote, List, ListItem Only)
        Stack blocks;
        
        // Position of this block in source module
        SourceDataBlock sourceMap;
    };

    // Return source data using from source and source map
    std::string MapSourceData(const SourceData& source, const SourceDataBlock& sourceMap);
        
#ifdef DEBUG
    // Prints markdown block recursively to stdout
    void printMarkdownBlock(const MarkdownBlock& block, unsigned int level = 0);
#endif

}

#endif
