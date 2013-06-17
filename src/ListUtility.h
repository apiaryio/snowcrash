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
    
    // Return first list / list item content block block (paragraph)
    // This is the first block inside the list / list item or the list item's closing block 
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

    // Return name block of list item; that is either FirstContentBlock() or
    // matching closing item block for inline items
    inline BlockIterator ListItemNameBlock(const BlockIterator& begin,
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
    inline BlockIterator CloseListItemBlock(const BlockIterator& begin,
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
    
    // Generic parser handler to warn & skip foreign blocks
    inline ParseSectionResult HandleForeignSection(const BlockIterator& cur,
                                                   const SectionBounds& bounds) {

        ParseSectionResult result = std::make_pair(Result(), cur);
        if (cur->type == ListItemBlockBeginType) {

            result.second = SkipToSectionEnd(cur, bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
            result.first.warnings.push_back(Warning("ignoring unrecognized list item",
                                                    0,
                                                    result.second->sourceMap));
            result.second = CloseListItemBlock(result.second, bounds.second);
        }
        else if (cur->type == ListBlockBeginType) {

            result.second = SkipToSectionEnd(cur, bounds.second, ListBlockBeginType, ListBlockEndType);
            result.first.warnings.push_back(Warning("ignoring unrecognized list",
                                                    0,
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
                                                    0,
                                                    result.second->sourceMap));
        }

        return result;
    }
    
    // Skips to the end of a list in description, checks
    template <class T>
    static BlockIterator SkipToDescriptionListEnd(const BlockIterator& begin,
                                                  const BlockIterator& end,
                                                  Result& result) {
        BlockIterator cur(begin);
        if (++cur == end)
            return cur;
        
        while (cur != end &&
               cur->type == ListItemBlockBeginType) {
            
            Section listSection = ClassifyInternaListBlock<T>(cur, end);
            cur = SkipToSectionEnd(cur, end, ListItemBlockBeginType, ListItemBlockEndType);
            
            if (listSection != UndefinedSection) {
                // WARN: skipping section in description
                std::stringstream ss;
                ss << "ignoring " << SectionName(listSection);
                ss << " in description, description should not end with list";
                result.warnings.push_back(Warning(ss.str(),
                                                  0,
                                                  (cur != end) ? cur->sourceMap : MakeSourceDataBlock(0,0)));
            }
            if (cur != end)
                ++cur;
        }
        
        return cur;
    }
    
    // Extracts first line of a list item content - signature
    inline SourceData GetListItemSignature(const BlockIterator& cur,
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
    inline ParseSectionResult ParseListPreformattedBlock(const Section& section,
                                                         const BlockIterator& cur,
                                                         const SectionBounds& bounds,
                                                         BlueprintParserCore& parser,
                                                         SourceData& data,
                                                         SourceDataBlock& sourceMap) {
        
        static const std::string FormattingWarning = "content is expected to be preformatted code block";
        
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
                std::stringstream ss;
                ss << SectionName(section) << " " << FormattingWarning;
                result.first.warnings.push_back(Warning(ss.str(),
                                                        0,
                                                        sectionCur->sourceMap));
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
            
            if (!CheckCursor(sectionCur, bounds, cur, result.first))
                return result;
            dataStream << MapSourceData(parser.sourceData, sectionCur->sourceMap);
            
            // WARN: not a preformatted code block
            std::stringstream ss;
            ss << SectionName(section) << " " << FormattingWarning;
            result.first.warnings.push_back(Warning(ss.str(),
                                                    0,
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
    inline bool HasNestedListBlock(const BlockIterator& begin, const BlockIterator& end) {

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
