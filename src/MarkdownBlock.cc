//
//  MarkdownBlock.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/15/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
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

#include "Serialize.h"

using namespace snowcrash;

static std::string BlockTypeToString(MarkdownBlockType type) {
    switch (type) {
        case UndefinedBlockType:
            return "UndefinedBlockType";
            
        case CodeBlockType:
            return "CodeBlockType";
            
        case QuoteBlockBeginType:
            return "QuoteBlockBeginType";
            
        case QuoteBlockEndType:
            return "QuoteBlockEndType";
            
        case HTMLBlockType:
            return "HTMLBlockType";
            
        case HeaderBlockType:
            return "HeaderBlockType";
            
        case HRuleBlockType:
            return "HRuleBlockType";
            
        case ListBlockBeginType:
            return "ListBlockBeginType";
            
        case ListBlockEndType:
            return "ListBlockEndType";
            
        case ListItemBlockBeginType:
            return "ListItemBlockBeginType";
            
        case ListItemBlockEndType:
            return "ListItemBlockEndType";
            
        case ParagraphBlockType:
            return "ParagraphBlockType";
            
        case TableBlockType:
            return "TableBlockType";
            
        case TableRowBlockType:
            return "TableRowBlockType";
            
        case TableCellBlockType:
            return "TableCellBlockType";
            
        default:
            return "n/a";
    }
}

void snowcrash::printMarkdown(const MarkdownBlock::Stack& markdown, unsigned int level)
{
    std::string indent;
    for (unsigned int i = 0; i < level; ++i) {
        indent += "  ";
    }

    for (MarkdownBlock::Stack::const_iterator block = markdown.begin(); block != markdown.end(); ++block) {
    
        std::cout << indent << BlockTypeToString(block->type);
        if (block->content.length()) {
            std::cout << ", content: '" << EscapeNewlines(block->content) << "'";
        }
        
        if (!block->sourceMap.empty()) {
            for (SourceDataBlock::const_iterator sourceMap = block->sourceMap.begin();
                 sourceMap != block->sourceMap.end();
                 ++sourceMap) {
                
                std::cout << ((sourceMap == block->sourceMap.begin()) ? ", :" : ";");
                std::cout << sourceMap->location << ":" << sourceMap->length;
            }
        }
        
        std::cout << std::endl;
    }
    
    if (level == 0)
        std::cout << std::endl;    
}

#endif
