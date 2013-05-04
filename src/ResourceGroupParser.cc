//
//  ResourceGroupParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <algorithm>
#include "ResourceGroupParser.h"
#include "BlockClassifier.h"

using namespace snowcrash;

// Finds a group in blueprint by name
Collection<ResourceGroup>::iterator snowcrash::FindResourceGroup(Blueprint& blueprint, const ResourceGroup& group)
{
    Collection<ResourceGroup>::iterator match = std::find_if(blueprint.resourceGroups.begin(),
                                                             blueprint.resourceGroups.end(),
                                                             std::bind2nd(MatchName<ResourceGroup>(), group));
    return match;
}

// Parse Resource Group descending into Resources
ParseSectionResult snowcrash::ParseResourceGroup(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Blueprint &blueprint)
{
    Result result;
    Section currentSection = ResourceGroupSection;
    BlockIterator currentBlock = begin;
    ResourceGroup resourceGroup;
    Collection<ResourceGroup>::type::iterator duplicate = blueprint.resourceGroups.end();
    
    while ((currentSection = ClassifyBlock(*currentBlock, currentSection)) == ResourceGroupSection &&
           currentBlock != end) {
        
        // Name
        if (currentBlock == begin) {
            if (currentBlock->type == HeaderBlockType) {
                resourceGroup.name = currentBlock->content;
            }
            else {
                // WARN: No group name specified
                result.warnings.push_back(Warning("expected resources group name", 0, currentBlock->sourceMap));
                
                // Add as description
                resourceGroup.description += MapSourceData(sourceData, currentBlock->sourceMap);
            }
            
            // Check duplicate
            Collection<ResourceGroup>::type::iterator duplicate = FindResourceGroup(blueprint, resourceGroup);
            if (duplicate != blueprint.resourceGroups.end()) {
                // WARN: existing group
                result.warnings.push_back(Warning("group '" + resourceGroup.name + "' already exists", 0, currentBlock->sourceMap));
            }
        }
        else {
            // Description
            resourceGroup.description += MapSourceData(sourceData, currentBlock->sourceMap);
        }
        
        ++currentBlock;
    }
    
    if (duplicate != blueprint.resourceGroups.end()) {
        
        duplicate->description += resourceGroup.description;
    }
    else {
        
        blueprint.resourceGroups.push_back(resourceGroup); // FIXME: C++11 move
    }
    
    return std::make_pair(result, currentBlock);
}
