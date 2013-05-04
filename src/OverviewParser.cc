//
//  OverviewParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "OverviewParser.h"
#include "BlockClassifier.h"
#include "RegexMatch.h"

using namespace snowcrash;

ParseSectionResult snowcrash::ParseOverview(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Blueprint &blueprint)
{
    Result result;
    Section currentSection = OverviewSection;
    BlockIterator currentBlock = begin;
    
    while ((currentSection = ClassifyBlock(*currentBlock, currentSection)) == OverviewSection &&
           currentBlock != end) {
        
        if (currentBlock == begin &&
            currentBlock->type == HeaderBlockType) {
            
            // Blueprint Name
            blueprint.name = currentBlock->content;
        }
        else {
            if (currentBlock == begin) {
                // WARN: No API name specified
                result.warnings.push_back(Warning("expected API name", 0, currentBlock->sourceMap));
            }
            
            if (currentBlock->type == HRuleBlockType) {
                
                // Section terminator, eat it and bail out
                ++currentBlock;
                break;
            }
            else {
                // Arbitrary markdown content, add to description
                blueprint.description += MapSourceData(sourceData, currentBlock->sourceMap);
            }
        }
        
        ++currentBlock;
        currentSection = ClassifyBlock(*currentBlock, currentSection);
    }
    
    return std::make_pair(result, currentBlock);
}
