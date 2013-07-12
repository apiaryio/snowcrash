//
//  ParserCore.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_PARSERCORE_H
#define SNOWCRASH_PARSERCORE_H

#include <string>
#include <vector>
#include "Platform.h"

namespace snowcrash {
    
    /**
     *  \brief  Textual source data. A markdown-formatted text.
     */
    typedef std::string SourceData;
    
    /**
     *  \brief  A range of data within the source data buffer.
     */
    struct SourceDataRange {
        size_t location;
        size_t length;
    };

    /**
     *  \brief  A block of source data.
     * 
     *  NOTE: The block does not have to be continuous.
     */
    typedef std::vector<SourceDataRange> SourceDataBlock;
    
    /**
     *  \brief  Create a %SourceDataBlock with a range.
     *  \param  loc     A location in source data buffer.
     *  \param  len     Length of the range.
     *
     *  NOTE: A binding does not need to wrap this function.
     */
    SourceDataBlock MakeSourceDataBlock(size_t loc, size_t len);
    
    /**
     *  \brief  Append %SourceDataBlock to existign block, merging continuous blocks.
     *  \param  destination A block to append to.
     *  \param  append      A block to be appended.
     *
     *  NOTE: A binding does not need to wrap this function.
     */
    void AppendSourceDataBlock(SourceDataBlock& destination, const SourceDataBlock& append);
    
    /**
     *  \brief A generic pair of two blocks
     *
     *  NOTE: A binding does not need to wrap this data type.
     */
    typedef std::pair<SourceDataBlock, SourceDataBlock> SourceDataBlockPair;
    
    /**
     *  \brief  Splits %SourceDataBlock into two blocks.
     *  \param  block   A block to be split.
     *  \param  len     The length of first block after which the split occurs.
     *
     *  NOTE: A binding does not need to wrap this function.
     */
    SourceDataBlockPair SplitSourceDataBlock(const SourceDataBlock& block, size_t len);
}

#endif
