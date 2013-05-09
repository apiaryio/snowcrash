//
//  ResourceParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <algorithm>
#include "ResourceParser.h"
#include "MethodParser.h"
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

ResourceMatch snowcrash::FindResource(const Blueprint& blueprint, const Resource& resource)
{
    for (Collection<ResourceGroup>::const_iterator it = blueprint.resourceGroups.begin(); it != blueprint.resourceGroups.end(); ++it) {
        Collection<Resource>::const_iterator match = std::find_if(it->resources.begin(),
                                                                  it->resources.end(),
                                                                  std::bind2nd(MatchURI<Resource>(), resource));
        if (match != it->resources.end())
            return std::make_pair(it, match);
    }
    
    return std::make_pair(blueprint.resourceGroups.end(), Collection<Resource>::iterator());
}

static Section ClassifyBlock(const MarkdownBlock& block, Section previousContext)
{
    // Exclusive terminator: Parameters List, Headers List,
    // Method Header, Resource Header, any other block after previous headers
    
    if (block.type != HeaderBlockType)
        return (previousContext != ResourceSection) ? UndefinedSection : ResourceSection;
    
    if (block.type == HeaderBlockType) {
        if (HasResourceSignature(block)) {
            return (previousContext != ResourceSection) ? ResourceSection : UndefinedSection;
        }
        else if (HasMethodSignature(block)) {
            return MethodSection;
        }
        
        return (previousContext != ResourceSection) ? UndefinedSection : ResourceSection;
    }
    
//    if (block.type == ListBlockType) {
//        /// TODO:
//        return ResourceSection;
//    }
    
    return UndefinedSection;
}

// Parse resource name and description
static ParseSectionResult ParseResourceOverview(const BlockIterator& begin,
                                                const BlockIterator& end,
                                                const SourceData& sourceData,
                                                Resource& resource)
{
    Result result;
    Section currentSection = UndefinedSection;
    BlockIterator currentBlock = begin;
    
    while (currentBlock != end &&
           (currentSection = ClassifyBlock(*currentBlock, currentSection)) == ResourceSection) {
        
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

// Parse & append method
static ParseSectionResult HandleMethod(const BlockIterator& begin,
                                       const BlockIterator& end,
                                       const SourceData& sourceData,
                                       Resource& resource)
{
    Method method;
    ParseSectionResult result = ParseMethod(begin, end, sourceData, method);
    
    if (result.first.error.code != Error::OK)
        return result;

    // TODO: find duplicate
    Collection<Method>::iterator duplicate = FindMethod(resource, method);
    if (duplicate != resource.methods.end()) {
        
        // WARN: duplicate method
        // Consider merge
        result.first.warnings.push_back(Warning("method '" + method.method + "' already defined for resource '" + resource.uri + "'", 0, begin->sourceMap));
    }

    // Insert resource
    resource.methods.push_back(method);
    
    return result;
}

ParseSectionResult snowcrash::ParseResource(const BlockIterator& begin,
                                            const BlockIterator& end,
                                            const SourceData& sourceData,
                                            Resource &resource)
{
    Result result;
    Section currentSection = UndefinedSection;
    BlockIterator currentBlock = begin;
    
    while (currentBlock != end) {
        
        currentSection = ClassifyBlock(*currentBlock, currentSection);
        ParseSectionResult sectionResult = std::make_pair(Result(), currentBlock);
        if (currentSection == ResourceSection) {
            
            sectionResult = ParseResourceOverview(currentBlock, end, sourceData, resource);
        }
        else if (currentSection == MethodSection) {

            sectionResult = HandleMethod(currentBlock, end, sourceData, resource);
        }
        else if (currentSection == UndefinedSection) {
            
            break;
        }
        else {
            
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
