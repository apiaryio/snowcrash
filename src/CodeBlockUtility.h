//
//  CodeBlockUtility.h
//  snowcrash
//
//  Created by Zdenek Nemec on 11/10/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_CODEBLOCKUTILITY_H
#define SNOWCRASH_CODEBLOCKUTILITY_H

#include <sstream>
#include "BlockUtility.h"

namespace snowcrash {
    
    /**
     *  Compute expected indentation level of a code block.
     *  \param  section A section to compute indentation level for
     *  \return Indentation level (number of tabs) for a block to be
     *  considered a pre-formatted code block in given section.
     */
    FORCEINLINE size_t CodeBlockIndentationLevel(const BlueprintSection& section)
    {
        if (!section.hasParent() ||
            section.parent().type == BlueprintSectionType ||
            section.parent().type == ResourceGroupSectionType ||
            section.parent().type == ResourceSectionType ||
            section.parent().type == ResourceMethodSectionType ||
            section.parent().type == ActionSectionType) {
            
            return 1;
        }
        else if (section.parent().type == RequestBodySectionType ||
                 section.parent().type == ResponseBodySectionType ||
                 section.parent().type == ObjectBodySectionType ||
                 section.parent().type == ModelBodySectionType) {
            
            return 2;
        }
        else {
            
            return 3;
        }
    }
    
    /**
     *  \brief Check code block for potential excessive indentation of a list item.
     *  \return True if code block does not contain potential recognized list, false otherwise.
     */
    template <class T>
    FORCEINLINE bool CheckCodeBlockListItem(const BlueprintSection& section,
                                            const BlockIterator& cur,
                                            const SourceData& sourceData,
                                            Result& result)
    {
        
        // Check for possible superfluous indentation of a recognized list items.
        std::string line = GetFirstLine(cur->content);
        TrimStringStart(line);
        
        // If line appears to be a Markdown list construct a dummy list item
        // with the first line of code block as its content.
        // Check the list item with respective internal classifier.
        if (line.empty() ||
            (line[0] != '-' && line[0] != '+' && line[0] != '*'))
            return true;
        
        // Skip leading Markdown list item mark
        std::string listItemContent = line.substr(1, std::string::npos);
        TrimStringStart(listItemContent);
        MarkdownBlock::Stack dummyList;
        
        dummyList.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
        dummyList.push_back(MarkdownBlock(ListItemBlockEndType, listItemContent, 0, MakeSourceDataBlock(0, 0)));
        
        SectionType type = ClassifyInternaListBlock<T>(dummyList.begin(), dummyList.end());
        if (type != UndefinedSectionType) {
            
            size_t level = CodeBlockIndentationLevel(section);
            --level;
            
            // WARN: Superfluous indentation
            std::stringstream ss;
            ss << "excessive indentation, ";
            ss << SectionName(type) << " ";
            if (level) {
                ss << "section is expected to be indented by just ";
                ss << level * 4 << " spaces or " << level << " tab";
                if (level > 1)
                    ss << "s";
            }
            else {
                ss << "section is not expected to be indented";
            }
            
            SourceCharactersBlock sourceBlock = CharacterMapForBlock(cur,
                                                                     cur,
                                                                     section.bounds,
                                                                     sourceData);
            result.warnings.push_back(Warning(ss.str(),
                                              IndentationWarning,
                                              sourceBlock));
            
            return false;
        }
        
        return true;
    }
    
    /**
     *  \brief  Parses given block as a preformatted code block.
     *  \param  section     Actual section being parsed.
     *  \param  cur         Cursor within the section boundaries.
     *  \param  parser      Parser instance.
     *  \param  action      An output data buffer.
     *  \param  sourceMap   An output source map buffer.
     *  \return A block parser section result, pointing at the last block parsed.
     */
    template <class T>
    FORCEINLINE ParseSectionResult ParsePreformattedBlock(const BlueprintSection& section,
                                                          const BlockIterator& cur,
                                                          BlueprintParserCore& parser,
                                                          SourceData& data,
                                                          SourceDataBlock& sourceMap) {
        
        ParseSectionResult result = std::make_pair(Result(), cur);
        BlockIterator sectionCur = cur;
        std::stringstream dataStream;
        
        if (sectionCur->type == CodeBlockType) {
            // Well formatted content, stream it up
            dataStream << sectionCur->content;
            
            // Check for excessive indentation
            CheckCodeBlockListItem<T>(section, sectionCur, parser.sourceData, result.first);
        }
        else {
            // Other blocks, process & warn
            if (sectionCur->type == QuoteBlockBeginType) {
                sectionCur = SkipToClosingBlock(sectionCur, section.bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
            }
            else if (sectionCur->type == ListBlockBeginType) {
                sectionCur = SkipToClosingBlock(sectionCur, section.bounds.second, ListBlockBeginType, ListBlockEndType);
            }
            else if (sectionCur->type == ListItemBlockBeginType) {
                sectionCur = SkipToClosingBlock(sectionCur, section.bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
            }
            
            if (!CheckCursor(section, sectionCur, parser.sourceData, result.first))
                return result;
            dataStream << MapSourceData(parser.sourceData, sectionCur->sourceMap);
            
            // WARN: Not a preformatted code block
            std::stringstream ss;
            
            // Build the warning message
            size_t level = CodeBlockIndentationLevel(section);
            if (section.type == DanglingBodySectionType ||
                section.type == DanglingSchemaSectionType) {
                
                ss << "dangling " << SectionName(section.type) << " asset, ";
                ss << "expected a pre-formatted code block, indent every of its line by ";
                ss << level * 4 << " spaces or " << level << " tabs";
            }
            else {
                
                ss << SectionName(section.type) << " asset ";
                ss << "is expected to be a pre-formatted code block, every of its line indented by exactly ";
                ss << level * 4 << " spaces or " << level << " tabs";
            }
            
            SourceCharactersBlock sourceBlock = CharacterMapForBlock(sectionCur, cur, section.bounds, parser.sourceData);
            result.first.warnings.push_back(Warning(ss.str(),
                                                    IndentationWarning,
                                                    sourceBlock));
        }
        
        data = dataStream.str();
        sourceMap = sectionCur->sourceMap;
        
        if (sectionCur != section.bounds.second)
            result.second = ++sectionCur;
        
        return result;
    }
}

#endif
