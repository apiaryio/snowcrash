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
#include "SourceAnnotation.h"
#include "Platform.h"

namespace snowcrash {
    
    /**
     *  \brief  Textual source data byte buffer. A markdown-formatted text.
     */
    typedef std::string SourceData;
    
    /**
     *  \brief  A byte range of data within the source data buffer.
     *
     *  Important note: All ranges are in bytes not characters.
     */
    struct SourceDataRange {
        size_t location;
        size_t length;
    };

    /**
     *  \brief  A block of source data bytes - a byte map.
     * 
     *  NOTE: The block does not have to be continuous.
     *  Data Blocks are in bytes not characters.
     */
    typedef std::vector<SourceDataRange> SourceDataBlock;
    
    /**
     *  \brief  Create a %SourceDataBlock with a range.
     *  \param  loc     A location in source data buffer.
     *  \param  len     Length of the range.
     */
    SourceDataBlock MakeSourceDataBlock(size_t loc, size_t len);
    
    /**
     *  \brief  Append %SourceDataBlock to existing block, merging continuous blocks.
     *  \param  destination A block to append to.
     *  \param  append      A block to be appended.
     */
    void AppendSourceDataBlock(SourceDataBlock& destination, const SourceDataBlock& append);
    
    /**
     *  \brief A generic pair of two blocks
     */
    typedef std::pair<SourceDataBlock, SourceDataBlock> SourceDataBlockPair;
    
    /**
     *  \brief  Splits %SourceDataBlock into two blocks.
     *  \param  block   A block to be split.
     *  \param  len     The length of first block after which the split occurs.
     */
    SourceDataBlockPair SplitSourceDataBlock(const SourceDataBlock& block, size_t len);
    
    /**
     *  \brief  Maps range of source data bytes into range of characters
     */
    SourceCharactersRange MapSourceDataRange(const SourceDataRange& range, const SourceData& data);
    
    /**
     *  \brief  Maps block of source data bytes into block of characters.
     *  \param  block   A %SourceDataBlock to map into %SourceCharactersBlock
     *  \param  data    A mapped source data.
     *  \returns A character-map constructed from given byte map
     */
    SourceCharactersBlock MapSourceDataBlock(const SourceDataBlock& block, const SourceData& data);
}

#endif
