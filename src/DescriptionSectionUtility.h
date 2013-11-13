//
//  DescriptionSectionUtility.h
//  snowcrash
//
//  Created by Zdenek Nemec on 11/10/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_DESCRIPTIONSECTIONUTILITY_H
#define SNOWCRASH_DESCRIPTIONSECTIONUTILITY_H

#include <sstream>
#include "MarkdownBlock.h"
#include "BlueprintParserCore.h"
#include "CodeBlockUtility.h"

namespace snowcrash {
    
    
    /**
     *  \brief  Skips to the end of the description list.
     *  \param  begin   Begin of the description list.
     *  \param  end     End of the block buffer.
     *  \param  descriptionMap  Output buffer containing skipped source map.
     *  \return An iterator pointing to the last block of a description list.
     *          Or, if one of description list items is recognized as a section, iterator
     *          pointing to this section.
     *
     *  This functions checks any skipped list items for a section signature.
     *  If a section signature is found this function returns the first block
     *  of the recognized section. If no signature is found this function returns
     *  the last (closing) block of the description list.
     */
    template <class T>
    static BlockIterator SkipToDescriptionListEnd(const BlockIterator& begin,
                                                  const BlockIterator& end,
                                                  SourceDataBlock& descriptionMap) {
        BlockIterator cur(begin);
        if (++cur == end)   // Skip leading ListBlockBeginType
            return cur;
        
        std::vector<SourceDataBlock> listItemMaps;
        SectionType listItemSection = UndefinedSectionType;
        BlockIterator recognizedCur = end;
        
        while (cur != end &&
               cur->type == ListItemBlockBeginType) {
            
            // Classify list item
            listItemSection = ClassifyInternaListBlock<T>(cur, end);
            if (listItemSection != UndefinedSectionType) {
                // Found a recognized section, record & skip to the end of the list.
                recognizedCur = cur;
                cur = SkipToClosingBlock(begin, end, ListBlockBeginType, ListBlockEndType);
            }
            else {
                // Skip one list item & take note of its source map
                cur = SkipToClosingBlock(cur, end, ListItemBlockBeginType, ListItemBlockEndType);
                listItemMaps.push_back(cur->sourceMap);
                
                if (cur != end)
                    ++cur;
            }
        }
        
        // Resolve
        if (listItemSection == UndefinedSectionType) {
            descriptionMap = cur->sourceMap;
            return cur;
        }
        else {
            // Resolve correct description source map
            descriptionMap.clear();
            if (!cur->sourceMap.empty() &&
                !listItemMaps.empty() &&
                !listItemMaps.front().empty()) {
                
                SourceDataRange r;
                r.location = cur->sourceMap.front().location;
                r.length = listItemMaps.front().front().location - r.location;
                descriptionMap.push_back(r);
                
                for (std::vector<SourceDataBlock>::iterator it = listItemMaps.begin();
                     it != listItemMaps.end();
                     ++it) {
                    
                    SourceDataRange gap;
                    gap.location = descriptionMap.back().location + descriptionMap.back().length;
                    gap.length = it->front().location - gap.location;
                    if (gap.length) {
                        SourceDataBlock gapBlock(1, gap);
                        AppendSourceDataBlock(descriptionMap, gapBlock);
                    }
                    
                    AppendSourceDataBlock(descriptionMap, *it);
                }
            }
            
            return recognizedCur;
        }
    }
    
    /**
     *  \brief  Process a description block retrieving its content.
     *  \param  section     A section its block is being processed.
     *  \param  cur         Cursor to the block to process.
     *  \param  sourceData  Source data stream.
     *  \param  output      Output object to APPEND retrieved description into.
     *  \return Standard parser section result poinitng at the last block parsed.
     */
    template <class T>
    FORCEINLINE ParseSectionResult ParseDescriptionBlock(const BlueprintSection& section,
                                                         const BlockIterator& cur,
                                                         const SourceData& sourceData,
                                                         T& output) {
        
        ParseSectionResult result = std::make_pair(Result(), cur);
        BlockIterator sectionCur(cur);
        
        if (sectionCur->type == QuoteBlockBeginType) {
            sectionCur = SkipToClosingBlock(sectionCur, section.bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
        }
        else if (sectionCur->type == ListBlockBeginType) {
            
            SourceDataBlock descriptionMap;
            sectionCur = SkipToDescriptionListEnd<T>(sectionCur, section.bounds.second, descriptionMap);

            if (sectionCur->type != ListBlockEndType) {
                // Found recognized lists in the list block
                if (!descriptionMap.empty())
                    output.description += MapSourceData(sourceData, descriptionMap);
                
                result.second = sectionCur;
                return result;
            }
        }
        else if (sectionCur->type == CodeBlockType) {
            // Check code block for potential excessive indentation of a list item
            CheckCodeBlockListItem<T>(section, sectionCur, sourceData, result.first);
        }
        else if (sectionCur->type == HeaderBlockType) {
            // Check headers for potential keywords
            CheckHeaderBlock<T>(section, sectionCur, sourceData, result.first);
        }
        
        if (!CheckCursor(section, sectionCur, sourceData, result.first))
            return result;

        output.description += MapSourceData(sourceData, sectionCur->sourceMap);
        result.second = ++sectionCur;
        
        return result;
    }
}

#endif
