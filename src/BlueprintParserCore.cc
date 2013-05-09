//
//  BlueprintParserCore.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/9/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "BlueprintParser.h"

using namespace snowcrash;

BlockIterator snowcrash::SkipToSectionEnd(const BlockIterator& begin,
                                          const BlockIterator& end,
                                          MarkdownBlockType sectionBegin,
                                          MarkdownBlockType sectionEnd)
{
    BlockIterator currentBlock = begin;
    if (currentBlock->type == sectionBegin) {
        int quoteLevel = 1;
        ++currentBlock;
        while (currentBlock != end && quoteLevel) {
            if (currentBlock->type == sectionBegin)
                ++quoteLevel;
            
            if (currentBlock->type == sectionEnd)
                --quoteLevel;
            
            if (quoteLevel)
                ++currentBlock;
        }
    }
    
    return currentBlock;
}
