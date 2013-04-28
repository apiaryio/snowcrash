//
//  MarkdownBlock.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/15/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "MarkdownBlock.h"

#include <sstream>

#ifdef DEBUG
#include <iostream>
#endif

using namespace snowcrash;

void snowcrash::AppendSourceDataBlock(SourceDataBlock& destination, const SourceDataBlock& append)
{
    if (append.empty())
        return;
    
    if (destination.empty() ||
        append.front().location != destination.back().location + destination.back().length) {
        destination.insert(std::end(destination), std::begin(append), std::end(append));
    }
    else {
        // merge
        destination.back().length += append.front().length;
        
        if (append.size() > 1) {
            destination.insert(std::end(destination), ++std::begin(append), std::end(append));
        }
    }
}

std::string snowcrash::MapSourceData(const SourceData& source, const SourceDataBlock& sourceMap)
{
    if (source.empty())
        return std::string();
    
    auto length = source.length();
    std::stringstream ss;
    for (auto it = std::begin(sourceMap); it != std::end(sourceMap); ++it) {
        
        if (it->location + it->length > length)
            return std::string();   // wrong map
        
        ss << source.substr(it->location, it->length);
    }
    
    return ss.str();
}

#ifdef DEBUG
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
    
    for (auto it = std::begin(block.blocks); it != std::end(block.blocks); ++it) {
        printMarkdownBlock(*it, level + 1);
    }
    
    if (level == 0)
        std::cout << std::endl;
}
#endif
