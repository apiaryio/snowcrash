//
//  ListUtility.h
//  snowcrash
//
//  Deprecation Warning:    The content of this file should be
//                          eventually replaced by the ListBlockUtility.h.
//
//  Created by Zdenek Nemec on 5/12/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_LISTUTILITY_H
#define SNOWCRASH_LISTUTILITY_H

#include <vector>
#include <string>
#include <sstream>
#include "MarkdownBlock.h"
#include "BlueprintParserCore.h"
#include "CodeBlockUtility.h"

namespace snowcrash {
    
    // Pair of content parts
    typedef std::vector<MarkdownBlock::Content> ContentParts;
        
    // Extract first line from block content
    // ContentParts[0] contains first line of block content
    // ContentParts[1] contains the rest of block content
    FORCEINLINE ContentParts ExtractFirstLine(const MarkdownBlock& block) {
        
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
    
    // Return first list / list item content block block (paragraph)
    // This is the first block inside the list / list item or the list item's closing block
    // DEPRECATED - use ContentBlock() or SkipSignatureBlock() instead
    FORCEINLINE BlockIterator FirstContentBlock(const BlockIterator& begin,
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

    
    // Returns true if list item (begin) contains nested list block false otherwise
    // Look ahead. 
    FORCEINLINE bool HasNestedListBlock(const BlockIterator& begin,
                                        const BlockIterator& end) {

        BlockIterator sectionBegin = begin;
        if (sectionBegin == end)
            return false;
        
        if (sectionBegin->type == ListBlockBeginType &&
            ++sectionBegin == end)
            return false;

        if (sectionBegin->type != ListItemBlockBeginType)
            return false;
        
        BlockIterator sectionEnd = SkipToClosingBlock(sectionBegin,
                                                    end,
                                                    ListItemBlockBeginType,
                                                    ListItemBlockEndType);
        for (BlockIterator it = sectionBegin; it != sectionEnd; ++it) {
            if (it->type == ListBlockBeginType)
                return true;
        }
        
        return false;
    }
}

#endif
