//
//  PayloadParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/7/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "PayloadParser.h"
#include "RegexMatch.h"

using namespace snowcrash;

// Request matching regex
static const std::string RequestRegex("^[Rr]equest([[:space:]]+([A-Za-z0-9_]|[[:space:]])*)?([[:space:]]\\([^\\)]*\\))?[[:space:]]*$");

// Response matching regex
static const std::string ResponseRegex("^[Rr]esponse([[:space:]]+([0-9_])*)?([[:space:]]\\([^\\)]*\\))?[[:space:]]*$");

PayloadSignature snowcrash::HasPayloadSignature(const MarkdownBlock& block)
{
    if (block.type != ListBlockType ||
        block.blocks.empty() ||
        block.blocks.front().type != ListItemBlockType)
        return NoPayloadSignature;
    
    MarkdownBlock::Stack::const_iterator item = block.blocks.begin();
    
    // In-place list (without child blocks)
    if (!item->content.empty() || item->blocks.empty())
        return NoPayloadSignature;
    
    // Sanity check, this should not happen
    if (item->blocks.front().type != ParagraphBlockType)
        return NoPayloadSignature;
    
    if (RegexMatch(item->blocks.front().content, RequestRegex))
        return RequestPayloadSignature;

    if (RegexMatch(item->blocks.front().content, ResponseRegex))
        return ResponsePayloadSignature;
    
    return NoPayloadSignature;
}

static Section ClassifyBlock(const MarkdownBlock& block, Section previousContext)
{
    // Exclusive terminator: Other Method Header, Parameters List, Headers List,
    // Request List, Response List and any other block after previous headers
    
//    if (block.type != HeaderBlockType &&
//        block.type != ListBlockType)
//        return (previousContext != MethodSection) ? UndefinedSection : MethodSection;
//    
//    if (block.type == HeaderBlockType && HasMethodSignature(block)) {
//        return (previousContext != MethodSection) ? MethodSection : UndefinedSection;
//    }
//    
//    if (block.type == ListBlockType) {
//        /// TODO:
//        return MethodSection;
//    }
//    
//    return (previousContext != MethodSection) ? UndefinedSection : MethodSection;
    return RequestSection;
}

ParseSectionResult snowcrash::ParsePayload(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Payload resource)
{
    Result result;
    Section currentSection = UndefinedSection;
    BlockIterator currentBlock = begin;
    
    while (currentBlock != end) {
        
        currentSection = ClassifyBlock(*currentBlock, currentSection);
        
        ParseSectionResult sectionResult;
        sectionResult.second = currentBlock;
        if (currentSection == RequestSection) {
            
            // TODO:
            //sectionResult = ParseResourceOverview(currentBlock, end, sourceData, resource);
        }
        else if (currentSection == ResponseSection) {
            
            // TODO:
            //sectionResult = ProcessMethod(currentBlock, end, sourceData, resource);
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
