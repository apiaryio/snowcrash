//
//  ResourceGroupParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <algorithm>
#include "ResourceGroupParser.h"
#include "ResourceParser.h"

using namespace snowcrash;

Collection<ResourceGroup>::iterator snowcrash::FindResourceGroup(Blueprint& blueprint, const ResourceGroup& group)
{
    Collection<ResourceGroup>::iterator match = std::find_if(blueprint.resourceGroups.begin(),
                                                             blueprint.resourceGroups.end(),
                                                             std::bind2nd(MatchName<ResourceGroup>(), group));
    return match;
}

static Section ClassifyBlock(const MarkdownBlock& block, Section previousContext)
{
    // We are in Resource Group section
    // exclusive terminator: Other Resource Group, Resource Header
    
    if (HasResourceSignature(block))
        return ResourceSection;
    else if (previousContext == ResourceSection)
        return UndefinedSection;
    else
        return ResourceGroupSection;
}

// Parse group's name and description
static ParseSectionResult ParseResourceGroupOverview(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, ResourceGroup& group)
{
    Result result;
    Section currentSection = UndefinedSection;
    BlockIterator currentBlock = begin;
    
    while (currentBlock != end &&
           (currentSection = ClassifyBlock(*currentBlock, currentSection)) == ResourceGroupSection) {

        // Name
        if (currentBlock == begin) {
            if (currentBlock->type == HeaderBlockType) {
                group.name = currentBlock->content;
            }
            else {
                // WARN: No group name specified
                result.warnings.push_back(Warning("expected resources group name", 0, currentBlock->sourceMap));
                
                // Add as description
                group.description += MapSourceData(sourceData, currentBlock->sourceMap);
            }
        }
        else {
            // Description
            group.description += MapSourceData(sourceData, currentBlock->sourceMap);
        }
        
        ++currentBlock;
    }
    
    return std::make_pair(result, currentBlock);
}

// Parse & append resource
static ParseSectionResult ProcessResource(const BlockIterator& begin,
                                          const BlockIterator& end,
                                          const SourceData& sourceData,
                                          const Blueprint& blueprint,
                                          ResourceGroup& group)
{
    Resource resource;
    ParseSectionResult result = ParseResource(begin, end, sourceData, resource);
    
    if (result.first.error.code != Error::OK)
        return result;
    
    ResourceMatch duplicate = FindResource(blueprint, resource);
    if (duplicate.first != blueprint.resourceGroups.end()) {

        // WARN: duplicate resource
        // Just warn, consider merge or error
        result.first.warnings.push_back(Warning("resource '" + resource.uri + "' already exists", 0, begin->sourceMap));
    }
    
    // Insert resource
    group.resources.push_back(resource);
    
    return result;
}

ParseSectionResult snowcrash::ParseResourceGroup(const BlockIterator& begin,
                                                 const BlockIterator& end,
                                                 const SourceData& sourceData,
                                                 const Blueprint& blueprint,
                                                 ResourceGroup& group)

{
    Result result;
    Section currentSection = UndefinedSection;
    BlockIterator currentBlock = begin;
    
    while (currentBlock != end) {
        
        currentSection = ClassifyBlock(*currentBlock, currentSection);
        
        ParseSectionResult sectionResult;
        sectionResult.second = currentBlock;
        if (currentSection == ResourceGroupSection) {

            sectionResult = ParseResourceGroupOverview(currentBlock, end, sourceData, group);
        }
        else if (currentSection == ResourceSection) {

            sectionResult = ProcessResource(currentBlock, end, sourceData, blueprint, group);
        }
        else {

            // Sanity check
            result.error = Error("unexpected block", 1, currentBlock->sourceMap);
            break;
        }
        
        result += sectionResult.first;
        if (result.error.code != Error::OK)
            break;

        if (sectionResult.second != currentBlock)
            currentBlock = sectionResult.second;
        else
            ++currentBlock;
    }

    return std::make_pair(result, currentBlock);
}
