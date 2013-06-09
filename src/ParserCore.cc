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
