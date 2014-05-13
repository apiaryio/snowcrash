//
//  BlockUtility.h
//  snowcrash
//
//  Created by Zdenek Nemec on 11/11/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_BLOCKUTILITY_H
#define SNOWCRASH_BLOCKUTILITY_H

#include "BlueprintSection.h"
#include "StringUtility.h"

namespace snowcrash {
    
    /**
     *  \brief  Skip to the matching closing block.
     *  \param  begin   Begin of the section inside a block buffer.
     *  \param  end     End of the block buffer.
     *  \param  sectionBegin    A %MarkdownBlockType of the beginning.
     *  \param  sectionEnd      A %MarkdownBlockType of the end.
     *  \return An iterator pointing to the end of the section.
     *
     *  Advances iterator from the begin of a section to the end
     *  of a section at the same nesting level.
     */
    FORCEINLINE BlockIterator SkipToClosingBlock(const BlockIterator& begin,
                                                 const BlockIterator& end,
                                                 MarkdownBlockType sectionBegin,
                                                 MarkdownBlockType sectionEnd) {
        
        BlockIterator currentBlock = begin;
        if (currentBlock->type == sectionBegin) {
            int level = 1;
            ++currentBlock;
            while (currentBlock != end && level) {
                if (currentBlock->type == sectionBegin)
                    ++level;
                
                if (currentBlock->type == sectionEnd)
                    --level;
                
                if (level)
                    ++currentBlock;
            }
        }
        
        return currentBlock;
    }
    
    /**
     *  \brief  Parse one line of raw `key:value` data.
     *  \param  line    A line to parse.
     *  \param  keyValuePair    The output buffer to place the parsed data into.
     *  \return True on success, false otherwise.
     */
    FORCEINLINE bool KeyValueFromLine(const std::string& line,
                                      KeyValuePair& keyValuePair) {
        
        std::vector<std::string> rawMetadata = SplitOnFirst(line, ':');
        if (rawMetadata.size() != 2)
            return false;
        
        keyValuePair = std::make_pair(rawMetadata[0], rawMetadata[1]);
        TrimString(keyValuePair.first);
        TrimString(keyValuePair.second);
        
        return (!keyValuePair.first.empty() && !keyValuePair.second.empty());
    }
    
    /**
     *  \brief  Construct unexpected block error message for given block.
     *  \param  section A section being parsed.
     *  \param  cue     Cursor to the unexpected markdown block.
     *  \param  sourceData  Source data byte buffer.
     *  \return An Error with description relevant to the type of the unexpected block.
     *
     *  This function 
     */
    FORCEINLINE Error UnexpectedBlockError(const BlueprintSection& section,
                                           const BlockIterator& cur,
                                           const SourceData& sourceData) {
        
        
        std::stringstream ss;
        ss << "unexpected " << BlockName(cur->type) << " block";
        
        if (section.hasParent()) {
            
            switch (section.parent().type) {

                // Top-level section
                case UndefinedSectionType:
                    ss << ", expected a group, resource or an action definition, e.g. ";
                    ss << "'# Group <name>', '# <resource name> [<URI>]' or '# <HTTP method> <URI>'";
                    break;

                // TODO: other sections
                    
                default:
                    break;
            }
        }
        
        return Error(ss.str(),
                     BusinessError,
                     MapSourceDataBlock(cur->sourceMap, sourceData));
    }
    
    
    /**
     *  \brief  Retrieves source code character map for a markdown block.
     *  \param  cur         A block to retrieve the characters map for.
     *  \param  fallback    Alternative block if %cur map source does not exists.
     *  \param  bounds      Boundaries of the %cur and %fallback container.
     *  \param  sourceData  Source data to map.
     *  \returns            Character map for given markdown block or its alternative block,
     *                      if exists. Empty source character map otherwise.
     */
    FORCEINLINE SourceCharactersBlock CharacterMapForBlock(const BlockIterator& cur,
                                                           const BlockIterator& fallback,
                                                           const SectionBounds& bounds,
                                                           const SourceData& sourceData)
    {
        // Try to use cursor's source map
        if (cur != bounds.second &&
            !cur->sourceMap.empty()) {
            return MapSourceDataBlock(cur->sourceMap, sourceData);
        }
        
        // Fallback to alternative (previous) block
        if (fallback != bounds.second &&
            !fallback->sourceMap.empty()) {
            return MapSourceDataBlock(fallback->sourceMap, sourceData);
        }
        
        return SourceCharactersBlock();
    }
    
    /**
     *  \brief Checks cursor validity within its container.
     *  \param section   A section to check against.
     *  \param cur  An iterator to be checked.
     *  \param sourceData   Source data byte buffer.
     *  \param result   Error result output, an error object is added in case of failed check.
     *  \returns True if cursor appears to be valid, false otherwise.
     */
    FORCEINLINE bool CheckCursor(const BlueprintSection& section,
                                 const BlockIterator& cur,
                                 const SourceData& sourceData,
                                 Result& result) {
        if (cur != section.bounds.second)
            return true;
        
        if (!section.hasParent() ||
            section.parent().bounds.first->sourceMap.empty())
            return false;
        
        // ERR: Sanity check
        SourceCharactersBlock sourceBlock = CharacterMapForBlock(section.parent().bounds.first,
                                                                 section.parent().bounds.second,
                                                                 section.parent().bounds,
                                                                 sourceData);
        result.error = Error("unexpected markdown closure",
                             ApplicationError,
                             sourceBlock);
        return false;
    }
    
    /**
     *  \brief Check header block for an potential ambiguous signature
     *  \return True if a header does not contain potential ambiguous signature, false otherwise.
     */
    template <class T>
    FORCEINLINE bool CheckHeaderBlock(const BlueprintSection& section,
                                      const BlockIterator& cur,
                                      const SourceData& sourceData,
                                      Result& result) {
        
        return false;
    }
}


#endif
