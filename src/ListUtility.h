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
    
    // Skips to first closing list item block or list block if there is no additional list item
    // Appends result warnings if skipping other blocks
    inline BlockIterator SkipToListBlockEnd(const BlockIterator& begin,
                                            const BlockIterator& end,
                                            Result& result) {
        
        if (begin->type == ListBlockBeginType)
            return begin;
        
        BlockIterator cur = begin;
        int level = 0;
        while (cur != end) {
            
            if (cur->type == ListItemBlockEndType && level == 0)
                break;
            
            if (cur->type == ListBlockBeginType)
                ++level;
            else if (cur->type == ListBlockEndType)
                --level;
            
            if (cur->type != ListBlockBeginType &&
                cur->type != ListItemBlockBeginType &&
                cur->type != QuoteBlockBeginType)
                result.warnings.push_back(Warning("ignoring extraneous content", 0, cur->sourceMap));
            ++cur;
        }
        
        BlockIterator next(cur);
        ++next;
        if (next != end && next->type == ListBlockEndType)
            ++cur;  // eat closing list block
        
        return cur;
    }
    
    // Skips to AFTER current level's closing list block
    inline ParseSectionResult SkipAfterListBlockEnd(const BlockIterator& begin,
                                                    const BlockIterator& end) {
        Result result;
        BlockIterator cur(begin);
        if (cur->type == ListBlockEndType) {
            return std::make_pair(result, ++cur);
        }
        
        if (cur->type == ListItemBlockEndType) {
            cur = SkipToListBlockEnd(cur, end, result);
            return std::make_pair(result, ++cur);
        }
        
        return std::make_pair(result, begin);
    }
}

#endif
