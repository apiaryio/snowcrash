//
//  ListBlockUtility.h
//  snowcrash
//
//  All the cool kids from ListUtility.h live here.
//
//  Created by Zdenek Nemec on 11/11/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_LISTBLOCKUTILITY_H
#define SNOWCRASH_LISTBLOCKUTILITY_H

#include "ListUtility.h"

namespace snowcrash {
    
    /**
     *  \brief  Return a first non-signature content block of a list(item).
     *  \param  begin    Begin of the block buffer to examine.
     *  \param  end      End of the block buffer.
     *  \return First non-signature content block or begin.
     *
     *  Returns first block with the actual content of a list or list item.
     */
    FORCEINLINE BlockIterator ContentBlock(const BlockIterator& begin,
                                           const BlockIterator& end) {
        
        BlockIterator cur = begin;
        if (cur->type == ListBlockBeginType) {
            if (++cur == end)
                return end;
            
            return cur;
        }
        
        if (cur->type == ListItemBlockBeginType) {
            if (++cur == end)
                return end;
            
            if (cur->type == ListItemBlockEndType)
                return begin;
            
            if (cur->type == ParagraphBlockType)
                if (++cur == end)
                    return end;
        }
        
        return cur;
    }

    /**
     * \brief   Skips first list item signature block.
     *
     *  This function effectively returns the first content block of
     *  a list item that is not a signature block. If no such a block exists.
     *  a block following the list item is returned. If a list is provided it
     *  uses its first list item.
     */
    FORCEINLINE BlockIterator SkipSignatureBlock(const BlockIterator& begin,
                                                 const BlockIterator& end) {
        
        BlockIterator cur = begin;
        
        // Skip to fist list item if appropriate
        if (cur->type == ListBlockBeginType) {
            cur = ContentBlock(cur, end);
        }
        
        // Skip to first list item content
        if (cur->type == ListItemBlockBeginType) {
            BlockIterator firstContent = ContentBlock(cur, end);
            if (cur != firstContent) {
                cur = firstContent;
            }
            else {
                // No content, just move to the next block
                ++cur;
            }
        }
        
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
        cur = SkipToClosingBlock(cur, end, ListBlockBeginType, ListBlockEndType);
        if (cur != end)
            return ++cur;
        
        return cur;
    }
    
    /**
     *  \brief  Eats closing blocks of a list / list item block.
     *  \param  begin   Cursor to a list or list item closing block.
     *  \param  end     End of a block buffer.
     *  \return Block AFTER closed list / list item.
     *
     *  Skips over subsequent ListItemBlockEndType and ListBlockEndType returning the fist
     *  other block after.
     */
    FORCEINLINE BlockIterator CloseList(const BlockIterator& begin,
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
     *  \brief  Skips COMPLETE consecutive (nested) closing elements of a list or a list item.
     *  \param  begin   The begin of a list or a list item.
     *  \param  end     End of markdown block buffer.
     *  \return An iterator pointing AFTER the last closing list / list item block.
     */
    FORCEINLINE BlockIterator CloseNestedList(const BlockIterator& begin,
                                              const BlockIterator& end) {
        BlockIterator cur = begin;
        while (cur != end &&
               (cur->type == ListItemBlockEndType || cur->type == ListBlockEndType)) {
            cur = CloseList(cur, end);
        }
        return cur;
    }
    
    /**
     *  \brief  Extract the first line of a list item content - its signature
     *  \param  cur  The begining of the list item to get its signature.
     *  \param  end  The begining of a block buffer.
     *  \param  remainingContent Any additonal content after the first line of signature.
     *  \return First line of the list item signature.
     */
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
    
    /**
     *  \brief Check List Item signature for an addtional content and issue a warning.
     *  \param section  Current section to be checked.
     *  \param cur      The begining of the list item to check.
     *  \param bounds   Bounds within the block buffer.
     *  \param sourceData   Source data byte buffer.
     *  \param placeHint    A string explaining the possible place of failure. Might be empty.
     *  \param expectedHint A string defining expected content. Might be empty.
     *  \param result   Result to append the possible warning into.
     *  \return True if signagure contains no additional content, false otherwise.
     */
    FORCEINLINE bool CheckSignatureAdditionalContent(const BlueprintSection& section,
                                                     const BlockIterator& cur,
                                                     const SourceData& sourceData,
                                                     const std::string& placeHint,
                                                     const std::string& expectedHint,
                                                     Result& result)
    {
        SourceData remainingContent;
        SourceData signature = GetListItemSignature(cur, section.bounds.second, remainingContent);
        
        if (!remainingContent.empty()) {
            // WARN: Superfluous content in signature
            
            std::stringstream ss;
            ss << "ignoring additional content";
            
            if (!placeHint.empty())
                ss << " after " << placeHint;
            
            if (!expectedHint.empty())
                ss << ", expected " << expectedHint;
            
            BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
            SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, sourceData);
            result.warnings.push_back(Warning(ss.str(),
                                              IgnoringWarning,
                                              sourceBlock));
        }
        
        return remainingContent.empty();
    }
    
    /**
     *  \brief  Parses list (item) block as a preformatted code block.
     *  \param  section     Actual section being parsed.
     *  \param  cur         Cursor within the section boundaries.
     *  \param  parser      Parser instance.
     *  \param  action      An output data buffer.
     *  \param  sourceMap   An output source map buffer.
     *  \return A block parser section result, pointing AFTER the last block parsed.
     */
    template <class T>
    FORCEINLINE ParseSectionResult ParseListPreformattedBlock(const BlueprintSection& section,
                                                              const BlockIterator& cur,
                                                              BlueprintParserCore& parser,
                                                              SourceData& data,
                                                              SourceDataBlock& sourceMap) {
        
        ParseSectionResult result = std::make_pair(Result(), cur);
        BlockIterator sectionCur = cur;
        
        if (sectionCur != section.bounds.first) {
            // Parse subsequent blocks as standalone pre blocks.
            return ParsePreformattedBlock<T>(section, sectionCur, parser, data, sourceMap);
        }
        
        // Parse first block of list, throwing away its first line (signature)
        SourceData content;
        SourceData signature = GetListItemSignature(cur, section.bounds.second, content);
        
        // Retrieve any extra lines after signature & warn
        if (!content.empty()) {
            
            data = content;
            
            // WARN: not a preformatted code block
            std::stringstream ss;
            
            size_t level = CodeBlockIndentationLevel(section);
            ss << SectionName(section.type) << " asset ";
            ss << "is expected to be a pre-formatted code block, separate it by a newline and ";
            ss << "indent every of its line by ";
            ss << level * 4 << " spaces or " << level << " tabs";
            
            BlockIterator nameBlock = ListItemNameBlock(sectionCur, section.bounds.second);
            SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, parser.sourceData);
            result.first.warnings.push_back(Warning(ss.str(),
                                                    IndentationWarning,
                                                    sourceBlock));
        }
        
        sectionCur = FirstContentBlock(cur, section.bounds.second);
        sourceMap = sectionCur->sourceMap;
        
        if (sectionCur != section.bounds.second)
            result.second = ++sectionCur;
        
        return result;
    }
}

#endif
