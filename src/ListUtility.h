//
//  ListUtility.h
//  snowcrash
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

    // Return name block of list item; that is either FirstContentBlock() or
    // matching closing item block for inline items
    FORCEINLINE BlockIterator ListItemNameBlock(const BlockIterator& begin,
                                                const BlockIterator& end) {
        
        BlockIterator cur = FirstContentBlock(begin, end);
        if (cur == end ||
            cur->type != ListBlockBeginType)
            return cur;
        
        // Inline list block
        cur = SkipToSectionEnd(cur, end, ListBlockBeginType, ListBlockEndType);
        if (cur != end)
            return ++cur;
        
        return cur;
    }
    
    // Eats closing elements of a list / list item block
    FORCEINLINE BlockIterator CloseListItemBlock(const BlockIterator& begin,
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
    
    /**
     *  \brief  Skips consecutive closing elements of a list or a list item.
     *  \param  begin   The begin of a list or a list item.
     *  \param  end     End of markdown block buffer.
     *  \return An iterator pointing AFTER the last closing list or list item block.
     */
    FORCEINLINE BlockIterator CloseList(const BlockIterator& begin,
                                        const BlockIterator& end) {
        BlockIterator cur = begin;
        while (cur != end &&
               (cur->type == ListItemBlockEndType || cur->type == ListBlockEndType)) {
            cur = CloseListItemBlock(cur, end);
        }
        return cur;
    }
    
    // Generic parser handler to warn & skip foreign blocks
    FORCEINLINE ParseSectionResult HandleForeignSection(const BlockIterator& cur,
                                                        const SectionBounds& bounds) {

        ParseSectionResult result = std::make_pair(Result(), cur);
        if (cur->type == ListItemBlockBeginType) {

            result.second = SkipToSectionEnd(cur, bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
            result.first.warnings.push_back(Warning("ignoring unrecognized list item",
                                                    IgnoringWarning,
                                                    result.second->sourceMap));
            result.second = CloseListItemBlock(result.second, bounds.second);
        }
        else if (cur->type == ListBlockBeginType) {

            result.second = SkipToSectionEnd(cur, bounds.second, ListBlockBeginType, ListBlockEndType);
            result.first.warnings.push_back(Warning("ignoring unrecognized list",
                                                    IgnoringWarning,
                                                    result.second->sourceMap));
            result.second = CloseListItemBlock(result.second, bounds.second);
        }
        else {
            if (cur->type == QuoteBlockBeginType) {
                result.second = SkipToSectionEnd(cur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
            }
            else {
                ++result.second;
            }
            result.first.warnings.push_back(Warning("ignoring unrecognized block, check indentation",
                                                    IgnoringWarning,
                                                    result.second->sourceMap));
        }

        return result;
    }
    
    /**
     *  \brief  Skip to the end of a description list.
     *  \param  begin   Begin of the description list.
     *  \param  end     End of the block buffer.
     *  \param  descriptionMap  Source map from the start
     *  of the list to the list end or recognized section list item.
     *  \return An iterator pointing to the last block of a description list.
     *  Or, if one of description list items is recognized as a section, iterator
     *  pointing to this section.
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
        Section listItemSection = UndefinedSection;
        BlockIterator recognizedCur = end;
        
        while (cur != end &&
               cur->type == ListItemBlockBeginType) {
            
            // Classify list item
            listItemSection = ClassifyInternaListBlock<T>(cur, end);
            if (listItemSection != UndefinedSection) {
                // Found a recognized section, record & skip to the end of the list.
                recognizedCur = cur;
                cur = SkipToSectionEnd(begin, end, ListBlockBeginType, ListBlockEndType);
            }
            else {
                // Skip one list item & take note of its source map
                cur = SkipToSectionEnd(cur, end, ListItemBlockBeginType, ListItemBlockEndType);
                listItemMaps.push_back(cur->sourceMap);
                
                if (cur != end)
                    ++cur;
            }
        }
        
        // Resolve
        if (listItemSection == UndefinedSection) {
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
    
    // Extracts first line of a list item content - signature
    FORCEINLINE SourceData GetListItemSignature(const BlockIterator& cur,
                                                const BlockIterator& end,
                                                SourceData& remainingContent) {

        BlockIterator sectionCur = ListItemNameBlock(cur, end);
        if (sectionCur == end)
            return SourceData();
        
        ContentParts content = ExtractFirstLine(*sectionCur);
        if (content.empty() ||
            content.front().empty())
            return SourceData();
        
        if (content.size() == 2)
            remainingContent = content[1];
        
        return content[0];
    }
    
    
    
    // Parse preformatted source data from block(s) of a list item block
    FORCEINLINE ParseSectionResult ParseListPreformattedBlock(const Section& section,
                                                              const BlockIterator& cur,
                                                              const SectionBounds& bounds,
                                                              BlueprintParserCore& parser,
                                                              SourceData& data,
                                                              SourceDataBlock& sourceMap) {
        
        static const std::string FormattingWarningMesssage = "content is expected to be preformatted code block";
        
        ParseSectionResult result = std::make_pair(Result(), cur);
        BlockIterator sectionCur = cur;
        std::stringstream dataStream;
        
        if (sectionCur == bounds.first) {
            // Process first block of list, throw away first line - signature
            SourceData content;
            SourceData signature = GetListItemSignature(cur, bounds.second, content);
            
            // Retrieve any extra lines after signature
            if (!content.empty()) {
                dataStream << content;
                
                // WARN: not a preformatted code block
                BlockIterator nameBlock = ListItemNameBlock(sectionCur, bounds.second);
                std::stringstream ss;
                ss << SectionName(section) << " " << FormattingWarningMesssage;
                result.first.warnings.push_back(Warning(ss.str(),
                                                        FormattingWarning,
                                                        nameBlock->sourceMap));
            }
            
            sectionCur = FirstContentBlock(cur, bounds.second);
        }
        else if (sectionCur->type == CodeBlockType) {

            dataStream << sectionCur->content; // well formatted content, stream it up
        }
        else {
            // Other blocks, process them but warn
            if (sectionCur->type == QuoteBlockBeginType) {
                sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
            }
            else if (sectionCur->type == ListBlockBeginType) {
                sectionCur = SkipToSectionEnd(sectionCur, bounds.second, ListBlockBeginType, ListBlockEndType);
            }
            else if (sectionCur->type == ListItemBlockBeginType) {
                sectionCur = SkipToSectionEnd(sectionCur, bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
            }
            
            if (!CheckCursor(sectionCur, bounds, cur, result.first))
                return result;
            dataStream << MapSourceData(parser.sourceData, sectionCur->sourceMap);
            
            // WARN: not a preformatted code block
            std::stringstream ss;
            ss << SectionName(section) << " " << FormattingWarningMesssage;
            result.first.warnings.push_back(Warning(ss.str(),
                                                    FormattingWarning,
                                                    sectionCur->sourceMap));
        }
        
        data = dataStream.str();
        sourceMap = sectionCur->sourceMap;
        
        if (sectionCur != bounds.second)
            result.second = ++sectionCur;
        
        return result;
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
        
        BlockIterator sectionEnd = SkipToSectionEnd(sectionBegin,
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
