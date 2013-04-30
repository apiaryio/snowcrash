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
    ResourceGroupTerminating,
    Resource,
    ResourceTerminating,
    Method,
    MethodTerminating
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

// Returns true if block represents a HTTP Method header, false otherwise
bool IsMethodHeader(const BlockStackIterator& block)
{
    if (block->type != MarkdownBlockType::Header ||
        block->content.empty())
        return false;
    
    return std::regex_match(block->content, MethodHeaderRegex);
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
    else if (context == Section::Resource) {
        
        // We are in Resource section
        // exclusive terminator: Parameters List, Headers List, Method Header, Resource Header
        
        if (block->type != MarkdownBlockType::Header &&
            block->type != MarkdownBlockType::List)
            return Section::Resource;
        
        if (block->type == MarkdownBlockType::Header) {
            if (IsResourceHeader(block)) {
                return Section::ResourceTerminating;
            }
            else if (IsMethodHeader(block)) {
                return Section::Method;
            }
            
            return Section::Resource;
        }
        
        if (block->type == MarkdownBlockType::List) {
            /// TODO:
            return Section::Resource;
        }
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

// Finds a group in blueprint by name
auto FindResourceGroup(Blueprint& blueprint, const Name& name) -> Collection<ResourceGroup>::iterator
{
    auto group = std::find_if(std::begin(blueprint.resourceGroups),
                              std::end(blueprint.resourceGroups),
                              [=](const ResourceGroup& rg) { return rg.name == name; });
    
    return group;
}

// Parse Resource Group descending into Resources
ParseSectionResult ParseResourceGroup(const BlockStackIterator& begin, const BlockStackIterator& end, const SourceData& sourceData, Blueprint &blueprint)
{
    Result result;    
    auto currentSection = Section::ResourceGroup;
    BlockStackIterator currentBlock = begin;
    ResourceGroup resourceGroup;
    auto duplicate = std::end(blueprint.resourceGroups);
    
    while (currentSection == Section::ResourceGroup &&
           currentBlock != end) {
        
        // Name
        if (currentBlock == begin) {
            if (currentBlock->type == MarkdownBlockType::Header) {
                resourceGroup.name = currentBlock->content;
            }
            else {
                // WARN: No group name specified
                result.warnings.emplace_back("expected resources group name", 0, currentBlock->sourceMap);
                
                // Add as description
                resourceGroup.description += MapSourceData(sourceData, currentBlock->sourceMap);
            }
            
            // Check duplicate
            auto duplicate = FindResourceGroup(blueprint, resourceGroup.name);
            if (duplicate != std::end(blueprint.resourceGroups)) {
                // WARN: existing group
                result.warnings.emplace_back("group '" + resourceGroup.name + "' already exists", 0, currentBlock->sourceMap);
            }
        }
        else {
            // Description
            resourceGroup.description += MapSourceData(sourceData, currentBlock->sourceMap);
        }
        
        ++currentBlock;
        currentSection = BlockSection(currentBlock, currentSection);
    }
    
    if (duplicate != std::end(blueprint.resourceGroups)) {
        
        duplicate->description += resourceGroup.description;
    }
    else {
        
        blueprint.resourceGroups.emplace_back(std::move(resourceGroup));
    }
    
    return std::make_pair(result, currentBlock);
}

// Parse Resource
ParseSectionResult ParseResource(const BlockStackIterator& begin, const BlockStackIterator& end, const SourceData& sourceData, Blueprint &blueprint)
{
    Result result;
    auto currentSection = Section::Resource;
    BlockStackIterator currentBlock = begin;
    
    while (currentSection == Section::Resource &&
           currentBlock != end) {
        
        // TODO:
        // Collect URI
        // Collect description
        // Construct method ? 

        ++currentBlock;
        currentSection = BlockSection(currentBlock, currentSection);
    }

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
            
            // TODO: Construct anonymous resource group and parse it from there
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
