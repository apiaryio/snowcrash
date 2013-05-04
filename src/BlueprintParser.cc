//
//  BlueprintParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "BlueprintParser.h"
#include "BlockClassifier.h"
#include "OverviewParser.h"
#include "ResourceGroupParser.h"
#include "ResourceParser.h"

using namespace snowcrash;

// Top-level parse
void BlueprintParser::parse(const SourceData& sourceData, const MarkdownBlock& source, Result& result, Blueprint& blueprint)
{
    // Initial section
    Section currentSection = OverviewSection;
    
    // Iterate over top-level blocks & parse any sections recognized
    MarkdownBlock::Stack::const_iterator currentBlock = source.blocks.begin();
    while (currentBlock != source.blocks.end()) {

        currentSection = ClassifyBlock(*currentBlock, currentSection);

        ParseSectionResult sectionResult;
        if (currentSection == OverviewSection) {
            sectionResult = ParseOverview(currentBlock, source.blocks.end(), sourceData, blueprint);
        }
        else if (currentSection == ResourceGroupSection) {
            sectionResult = ParseResourceGroup(currentBlock, source.blocks.end(), sourceData, blueprint);
        }
        else if (currentSection == ResourceSection) {
            
            // TODO: Construct anonymous resource group and parse it from there
            sectionResult = ParseResource(currentBlock, source.blocks.end(), sourceData, blueprint);
        }
        
        // Append result error & warning data
        result += sectionResult.first;
        
        // Check error
        if (result.error.code != Error::OK) {
            break;
        }
        
        // Proceed to the next block
        currentSection = UndefinedSection;
        if (currentBlock != sectionResult.second) {
            currentBlock = sectionResult.second;
        }
        else {
            ++currentBlock;
        }
    }
}
