//
//  BlueprintParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "BlueprintParser.h"

using namespace snowcrash;

enum class Section : int {
    Undefined,
    Overview,
    ResourceGroup,
    Resource
};

using BlockStackIterator = MarkdownBlock::Stack::const_iterator;
using ParseSectionResult = std::pair<Result, BlockStackIterator>;

// Returns true if block represents a resource header, false otherwise
bool IsResourceHeader(const BlockStackIterator& block)
{
    // TODO:
    return false;
}

// Returns section for given block and context
Section BlockSection(const BlockStackIterator& block, const Section& context)
{   
    if (context == Section::Overview) {

        // We are in Overview section
        // inclusive terminator: HR
        // exclusive terminator: Resource Header
        
        if (block->type != MarkdownBlockType::Header)
            return Section::Overview;
        
        if (IsResourceHeader(block))
            return Section::Resource;
        else
            return Section::Overview;
    }
    else if (context == Section::Undefined) {

        // We are in an undefined top-level section
        // Entering after parsed top level section. Which implies
        // only expected sections are Resource or Resource group.
        
        if (block->type != MarkdownBlockType::Header)
            return Section::ResourceGroup;
        
        if (IsResourceHeader(block))
            return Section::Resource;
        else
            return Section::ResourceGroup;
    }
    else if (context == Section::ResourceGroup) {
        
        // We are in Resource Group section
        // exclusive terminator: Resource Header
        
        if (block->type != MarkdownBlockType::Header)
            return Section::ResourceGroup;
        
        if (IsResourceHeader(block))
            return Section::Resource;
        else
            return Section::ResourceGroup;
    }
    
    return Section::Undefined;
}

// Parse Overview section
ParseSectionResult ParseOverview(const BlockStackIterator& begin, const BlockStackIterator& end, const SourceData& sourceData, Blueprint &blueprint)
{
    Result result;
    auto currentSection = Section::Overview;
    BlockStackIterator currentBlock = begin;

    while (currentSection == Section::Overview &&
           currentBlock != end) {
        
        if (currentBlock == begin &&
            currentBlock->type == MarkdownBlockType::Header) {

            // Blueprint Name
            blueprint.name = currentBlock->content;
        }
        else {
            if (currentBlock == begin) {
                // WARN: No API name specified
                result.warnings.emplace_back("No API name specified", 0, currentBlock->sourceMap);
            }
             
            if (currentBlock->type == MarkdownBlockType::HRule) {
                
                // Section terminator, eat it and bail out
                ++currentBlock;
                break;
            }
            else {
                // Arbitrary markdown content, add to description
                blueprint.description += MapSourceData(sourceData, currentBlock->sourceMap);
            }
        }

        ++currentBlock;
        currentSection = BlockSection(currentBlock, currentSection);
    }
    
    return std::make_pair(Result(), currentBlock);
}

// Parse Resource Group descending into Resources
ParseSectionResult ParseResourceGroup(const BlockStackIterator& begin, const BlockStackIterator& end, const SourceData& sourceData, Blueprint &blueprint)
{
    Result result;    
    auto currentSection = Section::ResourceGroup;
    BlockStackIterator currentBlock = begin;
    ResourceGroup resourceGroup;
    
    while (currentSection == Section::ResourceGroup &&
           currentBlock != end) {
        
        if (currentBlock == begin &&
            currentBlock->type == MarkdownBlockType::Header) {
            
            resourceGroup.name = currentBlock->content;
        }
        else {
            
            if (currentBlock == begin) {
                // WARN: No group name specified
                result.warnings.emplace_back("Unnamed Resource Group", 0, currentBlock->sourceMap);
            }
            
            resourceGroup.description += MapSourceData(sourceData, currentBlock->sourceMap);
        }
        
        ++currentBlock;
        currentSection = BlockSection(currentBlock, currentSection);
    }
    
    blueprint.resourceGroups.emplace_back(std::move(resourceGroup));
    return std::make_pair(Result(), currentBlock);
}


void BlueprintParser::parse(const SourceData& sourceData, const MarkdownBlock& source, const ParseHandler& callback)
{
    Blueprint blueprint;
    Result result;
    
    // Initial section
    auto currentSection = Section::Overview;
    
    // Iterate over top-level blocks & parse any sections recognized
    auto currentBlock = source.blocks.begin();
    while (currentBlock != source.blocks.end()) {

        ParseSectionResult sectionResult;
        currentSection = BlockSection(currentBlock, currentSection);
        
        if (currentSection == Section::Overview) {
            sectionResult = ParseOverview(currentBlock, source.blocks.end(), sourceData, blueprint);
        }
        if (currentSection == Section::ResourceGroup) {
            sectionResult = ParseResourceGroup(currentBlock, source.blocks.end(), sourceData, blueprint);
        }
        
        // Append result error & warning data
        result += sectionResult.first;
        
        // Check error
        if (result.error.code != Error::OK) {
            break;
        }
        
        // Proceed to the next block
        currentSection = Section::Undefined;
        if (currentBlock != sectionResult.second) {
            currentBlock = sectionResult.second;
        }
        else {
            ++currentBlock;
        }
    }
    
    if (callback)
        callback(result, blueprint);
}
