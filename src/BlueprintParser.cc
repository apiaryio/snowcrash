//
//  BlueprintParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "BlueprintParser.h"
#include "ResourceGroupParser.h"
#include "ResourceParser.h"

using namespace snowcrash;

static Section ClassifyBlock(const MarkdownBlock& block, Section previousContext)
{
    if (block.type == HRuleBlockType)
        return TerminatorSection;
    
    if (HasResourceSignature(block))
        return ResourceGroupSection; // Treat Resource as anonymous resource group
    
    if (previousContext == BlueprintSection)
        return BlueprintSection;
    
    if (previousContext == ResourceGroupSection ||
        previousContext == TerminatorSection)
        return ResourceGroupSection;
    
    return BlueprintSection;
}

// Parse overview
static ParseSectionResult ParseBlueprintOverview(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Blueprint& blueprint)
{
    Result result;
    Section currentSection = UndefinedSection;
    BlockIterator currentBlock = begin;
    
    while (currentBlock != end &&
           (currentSection = ClassifyBlock(*currentBlock, currentSection)) == BlueprintSection) {
        
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
            else {
                // Arbitrary markdown content, add to description
                blueprint.description += MapSourceData(sourceData, currentBlock->sourceMap);
            }
        }
        
        ++currentBlock;
    }
    
    return std::make_pair(result, currentBlock);
}

// Parse & append resource group
static ParseSectionResult ProcessResourceGroup(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Blueprint& blueprint)
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
    Section currentSection = UndefinedSection;
    
    // Iterate over top-level blocks & parse any sections recognized
    MarkdownBlock::Stack::const_iterator currentBlock = source.blocks.begin();
    while (currentBlock != source.blocks.end()) {

        currentSection = ClassifyBlock(*currentBlock, currentSection);
        if (currentSection == TerminatorSection) {
            ++currentBlock;
            continue;
        }
            
        ParseSectionResult sectionResult;
        sectionResult.second = currentBlock;
        if (currentSection == BlueprintSection) {
            
            sectionResult = ParseBlueprintOverview(currentBlock, source.blocks.end(), sourceData, blueprint);
        }
        else if (currentSection == ResourceGroupSection) {
            
            // Parse resource group and its resources
            // Top-level resource is treated as anonymous group
            sectionResult = ProcessResourceGroup(currentBlock, source.blocks.end(), sourceData, blueprint);
        }
        else {
            
            // Sanity check
            result.error = Error("unexpected block", 1, currentBlock->sourceMap);
            break;
        }
        
        // Append result error & warning data
        result += sectionResult.first;
        if (result.error.code != Error::OK)
            break;
        
        // Proceed to the next block
        if (sectionResult.second != currentBlock)
            currentBlock = sectionResult.second;
        else
            ++currentBlock;
    }
}
