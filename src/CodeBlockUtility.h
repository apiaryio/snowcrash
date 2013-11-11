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
#include "MarkdownBlock.h"
#include "BlueprintParserCore.h"

namespace snowcrash {
    
    /**
     *  Compute expected indentation level of a code block.
     *  \param  section A section to compute indentation level for
     *  \return Indentation level (number of tabs) for a block to be
     *  considered a pre-formatted code block in given section.
     */
    FORCEINLINE size_t CodeBlockIndentationLevel(const BlueprintSection& section)
    {
        if (!section.hasParent()) {
            
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
     *  \brief  Parses given block as a preformatted code block.
     *  \param  section     Actual section being parsed.
     *  \param  cur         Cursor within the section boundaries.
     *  \param  parser      Parser instance.
     *  \param  action      An output data buffer.
     *  \param  sourceMap   An output source map buffer.
     *  \return A block parser section result, pointing at the last block parsed.
     */
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
        }
        else {
            // Other blocks, process them & warn
            if (sectionCur->type == QuoteBlockBeginType) {
                sectionCur = SkipToSectionEnd(sectionCur, section.bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
            }
            else if (sectionCur->type == ListBlockBeginType) {
                sectionCur = SkipToSectionEnd(sectionCur, section.bounds.second, ListBlockBeginType, ListBlockEndType);
            }
            else if (sectionCur->type == ListItemBlockBeginType) {
                sectionCur = SkipToSectionEnd(sectionCur, section.bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
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
                
                ss << "dangling " << SectionName(section.type) << ", ";
                ss << "expected a pre-formatted code block, indent every of its line by ";
                ss << level * 4 << " spaces or " << level << " tabs";
            }
            else {
                
                ss << SectionName(section.type) << " ";
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
