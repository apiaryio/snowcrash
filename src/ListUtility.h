//
//  ListUtility.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/12/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_LISTUTILITY_H
#define SNOWCRASH_LISTUTILITY_H

#include <vector>
#include <string>
#include "MarkdownBlock.h"
#include "BlueprintParserCore.h"

namespace snowcrash {
    
    // Pair of content parts
    typedef std::vector<MarkdownBlock::Content> ContentParts;
        
    // Extract first line from block content
    // ContentParts[0] contains first line of block content
    // ContentParts[1] contains the rest of block content
    inline ContentParts ExtractFirstLine(const MarkdownBlock& block) {
        
        ContentParts result;
        if (block.content.empty())
            return result;
        
        std::string::size_type pos = block.content.find("\n");
        if (pos == std::string::npos) {
            result.push_back(block.content);
            return result;
        }
        
        result.push_back(block.content.substr(0, pos + 1));
        result.push_back(block.content.substr(pos + 1, std::string::npos));
        return result;
    }
    
    // Skips list / list item blocks to first content block (paragraph)
    inline BlockIterator FirstContentBlock(const BlockIterator& begin,
                                           const BlockIterator& end) {
        
        BlockIterator cur = begin;
        if (cur->type == ListBlockBeginType)
            if (++cur == end)
                return end;
        
        if (cur->type == ListItemBlockBeginType)
            if (++cur == end)
                return end;
        
        return cur;
    }
    
    inline BlockIterator CloseListItemBlock(const BlockIterator& begin,
                                            const BlockIterator& end) {
        
        BlockIterator cur = begin;
        if (cur != end &&
            cur->type == ListItemBlockEndType) {
            ++cur; // eat list item end
        }
        
        if (cur != end &&
            cur->type == ListBlockEndType) {
            ++cur; // eat list end
        }
        
        return cur;
    }
}

#endif
