//
//  ResourceParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "ResourceParser.h"
#include "BlockClassifier.h"
#include "RegexMatch.h"

using namespace snowcrash;

// Resource header regex
static const std::string ResourceHeaderRegex("^((" HTTP_METHODS ")[[:space:]]+)?/.*$");

bool snowcrash::HasResourceSignature(const MarkdownBlock& block)
{
    if (block.type != HeaderBlockType ||
        block.content.empty())
        return false;
    
    return RegexMatch(block.content, ResourceHeaderRegex);
}

// Finds a group in blueprint by name
ResourceMatch snowcrash::FindResource(const Blueprint& blueprint, const Resource& resource)
{
    for (Collection<ResourceGroup>::const_iterator it = blueprint.resourceGroups.begin(); it != blueprint.resourceGroups.end(); ++it) {
        Collection<Resource>::const_iterator match = std::find_if(it->resources.begin(),
                                                                  it->resources.end(),
                                                                  std::bind2nd(MatchURI<Resource>(), resource));
        if (match != it->resources.end()) {
            return std::make_pair(it, match);
        }
            
    }
    

    return std::make_pair(blueprint.resourceGroups.end(), Collection<Resource>::iterator());
}

ParseSectionResult snowcrash::ParseResource(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Resource &resource)
{
    Result result;
    Section currentSection = ResourceSection;
    BlockIterator currentBlock = begin;
    
    while ((currentSection = ClassifyBlock(*currentBlock, currentSection)) == ResourceSection &&
           currentBlock != end) {
        
        // URI
        if (currentBlock == begin) {
            resource.uri = currentBlock->content;
        }
        else {
            // Description
            resource.description += MapSourceData(sourceData, currentBlock->sourceMap);
        }
        
        // TODO: parameters, headers, methods
        
        ++currentBlock;
    }
    
    return std::make_pair(result, currentBlock);
}
