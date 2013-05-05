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

static ParseSectionResult processResourceGroup(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Blueprint &blueprint)
{
    ResourceGroup resourceGroup;
    ParseSectionResult result;
    result = ParseResourceGroup(begin, end, sourceData, blueprint, resourceGroup);
    
    // Check for duplicate & emplace group into blueprint
    if (result.first.error.code != Error::OK)
        return result;
        
    Collection<ResourceGroup>::iterator duplicate = FindResourceGroup(blueprint, resourceGroup);
    if (duplicate != blueprint.resourceGroups.end()) {
        
        // WARN: duplicate group
        result.first.warnings.push_back(Warning("group '" + resourceGroup.name + "' already exists", 0, begin->sourceMap));
        duplicate->description += resourceGroup.description;
    }
    else {
        
        blueprint.resourceGroups.push_back(resourceGroup); // FIXME: C++11 move
    }
    
    return result;
}

// Top-level parse, start here
void BlueprintParser::parse(const SourceData& sourceData, const MarkdownBlock& source, Result& result, Blueprint& blueprint)
{
    // Initial section
    Section currentSection = OverviewSection;
    
    // Iterate over top-level blocks & parse any sections recognized
    MarkdownBlock::Stack::const_iterator currentBlock = source.blocks.begin();
    while (currentBlock != source.blocks.end()) {

        currentSection = ClassifyBlock(*currentBlock, currentSection);

        ParseSectionResult sectionResult;
        sectionResult.second = currentBlock;
        
        if (currentSection == OverviewSection) {
            
            sectionResult = ParseOverview(currentBlock, source.blocks.end(), sourceData, blueprint);
        }
        else if (currentSection == ResourceGroupSection ||
                 currentSection == ResourceSection) {
            
            // Parse resource group and its resources
            // Top-level resource is treated as anonymous group
            sectionResult = processResourceGroup(currentBlock, source.blocks.end(), sourceData, blueprint);
        }
        else {
            
            // Sanity check
            result.error = Error("unexpected block", 1, currentBlock->sourceMap);
            break;
        }
        
        // Append result error & warning data
        result += sectionResult.first;
        
        // Check error
        if (result.error.code != Error::OK) {
            break;
        }
        
        // Proceed to the next block
        currentSection = UndefinedSection;
        if (sectionResult.second != currentBlock) {
            currentBlock = sectionResult.second;
        }
        else {
            ++currentBlock;
        }
    }
}
