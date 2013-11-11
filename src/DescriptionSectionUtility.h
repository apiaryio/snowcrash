//
//  DescriptionSectionUtility.h
//  snowcrash
//
//  Created by Zdenek Nemec on 11/10/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_DESCRIPTIONSECTIONUTILITY_H
#define SNOWCRASH_DESCRIPTIONSECTIONUTILITY_H

#include "MarkdownBlock.h"
#include "BlueprintParserCore.h"

namespace snowcrash {
    
    /**
     *  \brief  Process a description block retrieving its content.
     *  \param  section     A section its block is being processed.
     *  \param  cur         Cursor to the block to process.
     *  \param  sourceData  Source data stream.
     *  \param  output      Output object to APPEND retrieved description into.
     *  \return Standard parser section result poinitng at the last block parsed.
     */
    template <class T>
    FORCEINLINE ParseSectionResult ParserDescriptionBlock(const BlueprintSection& section,
                                                          const BlockIterator& cur,
                                                          const SourceData& sourceData,
                                                          T& output) {
        
        ParseSectionResult result = std::make_pair(Result(), cur);
        BlockIterator sectionCur(cur);
        
        if (sectionCur->type == QuoteBlockBeginType) {
            sectionCur = SkipToSectionEnd(sectionCur, section.bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
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
        
        if (!CheckCursor(section, sectionCur, sourceData, result.first))
            return result;

        output.description += MapSourceData(sourceData, sectionCur->sourceMap);
        result.second = ++sectionCur;
        
        return result;
    }
}

#endif
