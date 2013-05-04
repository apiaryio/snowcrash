//
//  MarkdownBlock.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/15/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <sstream>
#include "MarkdownBlock.h"

#ifdef DEBUG

#include <iostream>

#endif


std::string snowcrash::MapSourceData(const SourceData& source, const SourceDataBlock& sourceMap)
{
    if (source.empty())
        return std::string();
    
    size_t length = source.length();
    std::stringstream ss;
    for (SourceDataBlock::const_iterator it = sourceMap.begin(); it != sourceMap.end(); ++it) {
        
        if (it->location + it->length > length)
            return std::string();   // wrong map
        
        ss << source.substr(it->location, it->length);
    }
    
    return ss.str();
}

#ifdef DEBUG

using namespace snowcrash;

void snowcrash::printMarkdownBlock(const MarkdownBlock& block, unsigned int level)
{
    std::string indent;
    for (unsigned int i = 0; i < level; ++i) {
        indent += "  ";
    }

    std::cout << indent << "block: " << (int)block.type;
    if (block.content.length())
        std::cout << " source: '" << block.content << "'\n";
    else
        std::cout << std::endl;
    
    for (MarkdownBlock::Stack::const_iterator it = block.blocks.begin(); it != block.blocks.end(); ++it) {
        printMarkdownBlock(*it, level + 1);
    }
    
    if (level == 0)
        std::cout << std::endl;
}
#endif
