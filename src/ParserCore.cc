//
//  ParserCore.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
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
