//
//  SectionUtility.h
//  snowcrash
//
//  Created by Zdenek Nemec on 11/11/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SECTIONUTILITY_H
#define SNOWCRASH_SECTIONUTILITY_H

#include "BlockUtility.h"
#include "ListBlockUtility.h"
#include "CodeBlockUtility.h"

namespace snowcrash {
    
    /**
     *  Compute expected indentation of a section
     */
    FORCEINLINE size_t SectionIndentationLevel(const BlueprintSection& section)
    {
        switch (section.type) {
            case ParameterValuesSectionType:
                return 2;
                
            case BodySectionType:
            case SchemaSectionType:
            case HeadersSectionType:    // NOTE: Headers can be potentially both level 0 and 1
                return 1;

            default:
                return 0;
        }
    }
    
    /**
     *  \brief  Report & skip blocks in a foreign section.
     *  \param  section     Section including the foreign section.
     *  \param  cur         A cursor pointing to the beigin of the foreign section.
     *  \param  hint        An optional string containing expected hint.
     *  \return Standard section parse result pointing AFTER the last block parsed.
     */
    template <class T>
    FORCEINLINE ParseSectionResult HandleForeignSection(const BlueprintSection& section,
                                                        const BlockIterator& cur,
                                                        const SourceData& sourceData,
                                                        const std::string& hint = std::string()) {
        
        ParseSectionResult result = std::make_pair(Result(), cur);
        BlockIterator sectionCur = cur;
        
        std::stringstream ss;
        SourceCharactersBlock sourceBlock;
        
        // Handle list / list item separately
        if (sectionCur->type == ListItemBlockBeginType ||
            sectionCur->type == ListBlockBeginType) {
            
            // Skip the list / list item
            if (sectionCur->type == ListBlockBeginType) {
                sectionCur = SkipToClosingBlock(sectionCur, section.bounds.second, ListBlockBeginType, ListBlockEndType);
            }
            else {
                sectionCur = SkipToClosingBlock(sectionCur, section.bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
            }
            SectionType type = ClassifyChildrenListBlock<T>(cur, sectionCur);
            
            // Assemble the message
            ss << "ignoring ";
            ss << ((type != UndefinedSectionType) ? SectionName(type) : "unrecognized");
            ss << " list";
            if (cur->type == ListItemBlockBeginType)
                ss << " item";
            
            if (type != UndefinedSectionType &&
                section.hasParent()) {
                BlueprintSection recognizedSection(type, std::make_pair(cur, sectionCur), section.parent());
                size_t level = SectionIndentationLevel(recognizedSection);
                
                if (level) {
                    ss << ", " << SectionName(type) << " section is expected to be indented by ";
                    ss << level * 4 << " spaces or " << level << " tab";
                }
            }
            
            sourceBlock = CharacterMapForBlock(sectionCur, cur, section.bounds, sourceData);
            sectionCur = CloseList(sectionCur, section.bounds.second);
        }
        else if (sectionCur->type == QuoteBlockBeginType) {
            
            // Skip quote or just one block
            ss << "ignoring unrecognized quote";
            sectionCur = SkipToClosingBlock(sectionCur, section.bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
            sourceBlock = CharacterMapForBlock(sectionCur, cur, section.bounds, sourceData);
            
            if (!CheckCursor(section, sectionCur, sourceData, result.first))
                return result;
            
            ++sectionCur;
        }
        else {
            // Skip just one block
            ss << "ignoring unrecognized block";
            sourceBlock = CharacterMapForBlock(sectionCur, cur, section.bounds, sourceData);
            
            ++sectionCur;
        }
        
        // WARN: Ignoring unrecognized block
        if (!hint.empty())
            ss << ", expected: " << hint;
        result.first.warnings.push_back(Warning(ss.str(),
                                                IgnoringWarning,
                                                sourceBlock));
        result.second = sectionCur;
        return result;
    }
}

#endif
