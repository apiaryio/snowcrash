//
//  ParserCore.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "ParserCore.h"

using namespace snowcrash;

SourceDataBlock snowcrash::MakeSourceDataBlock(size_t loc, size_t len)
{
    SourceDataRange r;
    r.location = loc;
    r.length = len;
    return SourceDataBlock(1, r);
}

void snowcrash::AppendSourceDataBlock(SourceDataBlock& destination, const SourceDataBlock& append)
{
    if (append.empty())
        return;
    
    if (destination.empty() ||
        append.front().location != destination.back().location + destination.back().length) {
        destination.insert(destination.end(), append.begin(), append.end());
    }
    else {
        // merge
        destination.back().length += append.front().length;
        
        if (append.size() > 1) {
            destination.insert(destination.end(), ++append.begin(), append.end());
        }
    }
}

SourceDataBlockPair snowcrash::SplitSourceDataBlock(const SourceDataBlock& block, size_t len)
{
    if (block.empty())
        return std::make_pair(SourceDataBlock(), SourceDataBlock());

    SourceDataBlock first;
    SourceDataBlock second;
    
    size_t remain = len;
    for (SourceDataBlock::const_iterator it = block.begin(); it != block.end(); ++it) {
        
        if (remain == 0) {
            second.push_back(*it);
            continue;
        }
        
        if (remain >= it->length) {
            first.push_back(*it);
            remain -= it->length;
            continue;
        }
        
        if (remain < it->length) {
            SourceDataRange left = *it;
            left.length = remain;
            first.push_back(left);
            
            SourceDataRange right = *it;
            right.location += remain;
            right.length -= remain;
            second.push_back(right);
        }
    }
    
    return std::make_pair(first, second);
}

#define UTF8_CHAR_LEN( byte ) (( 0xE5000000 >> (( byte >> 3 ) & 0x1e )) & 3 ) + 1

/** \returns Number of UTF8 characters in byte buffer */
static size_t strnlen_utf8(const char* s, size_t len)
{
    if (!s || !len)
        return 0;
    
    size_t i = 0, j = 0;
	while (s[i] && i < len) {
        i += UTF8_CHAR_LEN(s[i]);
        j++;
	}
	return j;
}

SourceCharactersRange snowcrash::MapSourceDataRange(const SourceDataRange& range, const SourceData& data)
{
    if (data.empty())
        return SourceCharactersRange();
    
    size_t charLocation = 0;
    if (range.location > 0)
        charLocation = strnlen_utf8(data.c_str(), range.location);
    
    size_t charLength = 0;
    if (range.length > 0)
        charLength = strnlen_utf8(data.c_str() + range.location, range.length);
    
    SourceCharactersRange characterRange = {charLocation, charLength };
    return characterRange;
}

SourceCharactersBlock snowcrash::MapSourceDataBlock(const SourceDataBlock& block, const SourceData& data)
{
    SourceCharactersBlock characterMap;
    
    for (SourceDataBlock::const_iterator it = block.begin(); it != block.end(); ++it) {
        SourceCharactersRange characterRange = MapSourceDataRange(*it, data);
        characterMap.push_back(characterRange);
    }
        
    return characterMap;
}
