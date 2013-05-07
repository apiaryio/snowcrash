//
//  MethodParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <algorithm>
#include "MethodParser.h"
#include "RegexMatch.h"

using namespace snowcrash;

// Method header regex
static const std::string MethodHeaderRegex("^(" HTTP_METHODS ")[[:space:]]*$");

bool snowcrash::HasMethodSignature(const MarkdownBlock& block)
{
    if (block.type != HeaderBlockType ||
        block.content.empty())
        return false;
    
    return RegexMatch(block.content, MethodHeaderRegex);
}

Collection<Method>::iterator snowcrash::FindMethod(Resource& resource, const Method& method)
{
    Collection<Method>::iterator match = std::find_if(resource.methods.begin(),
                                                      resource.methods.end(),
                                                      std::bind2nd(MatchMethod<Method>(), method));
    
    return match;
}

static Section ClassifyBlock(const MarkdownBlock& block, Section previousContext)
{
    // Exclusive terminator: Other Method Header, Parameters List, Headers List,
    // Request List, Response List and any other block after previous headers
    
    if (block.type != HeaderBlockType &&
        block.type != ListBlockType)
        return (previousContext != MethodSection) ? UndefinedSection : MethodSection;

    if (block.type == HeaderBlockType && HasMethodSignature(block)) {
        return (previousContext != MethodSection) ? MethodSection : UndefinedSection;
    }

    if (block.type == ListBlockType) {
        /// TODO:
        return MethodSection;
    }
    
    return (previousContext != MethodSection) ? UndefinedSection : MethodSection;
}

// Parse method and description
static ParseSectionResult ParseMethodOverview(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Method& method)
{
    Result result;
    Section currentSection = UndefinedSection;
    BlockIterator currentBlock = begin;
    
    while (currentBlock != end &&
           (currentSection = ClassifyBlock(*currentBlock, currentSection)) == MethodSection) {
        
        // Method
        if (currentBlock == begin) {
            method.method= currentBlock->content;
        }
        else {
            // Description
            method.description += MapSourceData(sourceData, currentBlock->sourceMap);
        }
        
        // TODO: parameters, headers
        
        ++currentBlock;
    }
    
    return std::make_pair(result, currentBlock);
}

ParseSectionResult snowcrash::ParseMethod(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Method& method)
{
    Result result;
    Section currentSection = UndefinedSection;
    BlockIterator currentBlock = begin;
    
    while (currentBlock != end) {
        
        currentSection = ClassifyBlock(*currentBlock, currentSection);
        
        ParseSectionResult sectionResult;
        sectionResult.second = currentBlock;
        if (currentSection == MethodSection) {
            
            sectionResult = ParseMethodOverview(currentBlock, end, sourceData, method);
        }
        else if (currentSection == UndefinedSection) {
            
            break; // Pass control to caller
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
