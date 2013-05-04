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

ParseSectionResult snowcrash::ParseResource(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Blueprint &blueprint)
{
    Result result;
    Section currentSection = ResourceSection;
    BlockIterator currentBlock = begin;
    
    while (currentSection == ResourceSection &&
           currentBlock != end) {
        
        // TODO:
        // Collect URI
        // Collect description
        // Construct method ?
        
        ++currentBlock;
        currentSection = ClassifyBlock(*currentBlock, currentSection);
    }
    
    return std::make_pair(result, currentBlock);
}
