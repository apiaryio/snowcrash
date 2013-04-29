//
//  BlueprintParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <regex>
#include "BlueprintParser.h"

using namespace snowcrash;

// Parse sections
enum class Section : int {
    Undefined,
    Overview,
    ResourceGroup,
    Resource
};

// Parser iterator
using BlockStackIterator = MarkdownBlock::Stack::const_iterator;

// Parsing sub routine result
using ParseSectionResult = std::pair<Result, BlockStackIterator>;

// HTTP Methods
static const std::string HTTPMethods = "GET|POST|PUT|DELETE|OPTIONS|PATCH|PROPPATCH|LOCK|UNLOCK|COPY|MOVE|MKCOL";

// Method header regex
static const std::regex MethodHeaderRegex("^(" + HTTPMethods + R"()\s*$)");

// Resource header regex
static const std::regex ResourceHeaderRegex("^((" + HTTPMethods + R"()\s+)?\/(.*?)\s*$)");

// Returns true if block represents a resource header, false otherwise
bool IsResourceHeader(const BlockStackIterator& block)
{
    if (block->type != MarkdownBlockType::Header ||
        block->content.empty())
        return false;
    
    return std::regex_match(block->content, ResourceHeaderRegex);
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
                result.warnings.emplace_back("expected API name", 0, currentBlock->sourceMap);
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
    
    return std::make_pair(result, currentBlock);
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
                result.warnings.emplace_back("expected resources group name", 0, currentBlock->sourceMap);
            }
            
            resourceGroup.description += MapSourceData(sourceData, currentBlock->sourceMap);
        }
        
        ++currentBlock;
        currentSection = BlockSection(currentBlock, currentSection);
    }
    
    blueprint.resourceGroups.emplace_back(std::move(resourceGroup));
    return std::make_pair(result, currentBlock);
}

// Parse Resource
ParseSectionResult ParseResource(const BlockStackIterator& begin, const BlockStackIterator& end, const SourceData& sourceData, Blueprint &blueprint)
{
    Result result;
    auto currentSection = Section::ResourceGroup;
    BlockStackIterator currentBlock = begin;


    return std::make_pair(result, currentBlock);
}

// Top-level parse
void BlueprintParser::parse(const SourceData& sourceData, const MarkdownBlock& source, const ParseHandler& callback)
{
    Blueprint blueprint;
    Result result;
    
    // Initial section
    auto currentSection = Section::Overview;
    
    // Iterate over top-level blocks & parse any sections recognized
    auto currentBlock = std::begin(source.blocks);
    while (currentBlock != std::end(source.blocks)) {

        currentSection = BlockSection(currentBlock, currentSection);

        ParseSectionResult sectionResult;
        if (currentSection == Section::Overview) {
            sectionResult = ParseOverview(currentBlock, std::end(source.blocks), sourceData, blueprint);
        }
        else if (currentSection == Section::ResourceGroup) {
            sectionResult = ParseResourceGroup(currentBlock, std::end(source.blocks), sourceData, blueprint);
        }
        else if (currentSection == Section::Resource) {
            sectionResult = ParseResource(currentBlock, std::end(source.blocks), sourceData, blueprint);
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
