//
//  BlueprintParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

//#include <regex>
#include "BlueprintParser.h"
//#include <boost/regex.hpp>

using namespace snowcrash;

// Parse sections
enum Section {
    UndefinedSection,
    OverviewSection,
    ResourceGroupSection,
    ResourceGroupTerminatingSection,
    ResourceSection,
    ResourceTerminatingSection,
    MethodSection,
    MethodTerminatingSection
};

// Parser iterator
typedef MarkdownBlock::Stack::const_iterator BlockStackIterator;

// Parsing sub routine result
typedef std::pair<Result, BlockStackIterator> ParseSectionResult;

// HTTP Methods
static const std::string HTTPMethods = "GET|POST|PUT|DELETE|OPTIONS|PATCH|PROPPATCH|LOCK|UNLOCK|COPY|MOVE|MKCOL";

// Method header regex
//static const std::regex MethodHeaderRegex("^(" + HTTPMethods + R"()\s*$)");

// Resource header regex
//static const std::regex ResourceHeaderRegex("^((" + HTTPMethods + R"()\s+)?\/(.*?)\s*$)");

// Returns true if block represents a resource header, false otherwise
bool IsResourceHeader(const BlockStackIterator& block)
{
    if (block->type != HeaderBlockType ||
        block->content.empty())
        return false;
    
//    return std::regex_match(block->content, ResourceHeaderRegex);
    return false;
}

// Returns true if block represents a HTTP Method header, false otherwise
bool IsMethodHeader(const BlockStackIterator& block)
{
    if (block->type != HeaderBlockType ||
        block->content.empty())
        return false;
    
//    return std::regex_match(block->content, MethodHeaderRegex);
    return false;
}

// Returns section for given block and context
Section BlockSection(const BlockStackIterator& block, const Section& context)
{   
    if (context == OverviewSection) {

        // We are in Overview section
        // inclusive terminator: HR
        // exclusive terminator: Resource Header
        
        if (block->type != HeaderBlockType)
            return OverviewSection;
        
        if (IsResourceHeader(block))
            return ResourceSection;
        else
            return OverviewSection;
    }
    else if (context == UndefinedSection) {

        // We are in an undefined top-level section
        // Entering after parsed top level section. Which implies
        // only expected sections are Resource or Resource group.
        
        if (block->type != HeaderBlockType)
            return ResourceGroupSection;
        
        if (IsResourceHeader(block))
            return ResourceSection;
        else
            return ResourceGroupSection;
    }
    else if (context == ResourceGroupSection) {
        
        // We are in Resource Group section
        // exclusive terminator: Resource Header
        
        if (block->type != HeaderBlockType)
            return ResourceGroupSection;
        
        if (IsResourceHeader(block))
            return ResourceSection;
        else
            return ResourceGroupSection;
    }
    else if (context == ResourceSection) {
        
        // We are in Resource section
        // exclusive terminator: Parameters List, Headers List, Method Header, Resource Header
        
        if (block->type != HeaderBlockType &&
            block->type != ListBlockType)
            return ResourceSection;
        
        if (block->type == HeaderBlockType) {
            if (IsResourceHeader(block)) {
                return ResourceTerminatingSection;
            }
            else if (IsMethodHeader(block)) {
                return MethodSection;
            }
            
            return ResourceSection;
        }
        
        if (block->type == ListBlockType) {
            /// TODO:
            return ResourceSection;
        }
    }
    
    return UndefinedSection;
}

// Parse Overview section
ParseSectionResult ParseOverview(const BlockStackIterator& begin, const BlockStackIterator& end, const SourceData& sourceData, Blueprint &blueprint)
{
    Result result;
    Section currentSection = OverviewSection;
    BlockStackIterator currentBlock = begin;

    while (currentSection == OverviewSection &&
           currentBlock != end) {
        
        if (currentBlock == begin &&
            currentBlock->type == HeaderBlockType) {

            // Blueprint Name
            blueprint.name = currentBlock->content;
        }
        else {
            if (currentBlock == begin) {
                // WARN: No API name specified
                result.warnings.push_back(Warning("expected API name", 0, currentBlock->sourceMap));
            }
             
            if (currentBlock->type == HRuleBlockType) {
                
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
Collection<ResourceGroup>::type::iterator FindResourceGroup(Blueprint& blueprint, const Name& name)
{
    Collection<ResourceGroup>::type::iterator group = blueprint.resourceGroups.end();
    
    // TODO:
//    = std::find_if(blueprint.resourceGroups.begin(),
//                              blueprint.resourceGroups.end(),
//                              [=](const ResourceGroup& rg) { return rg.name == name; });
    
    return group;
}

// Parse Resource Group descending into Resources
ParseSectionResult ParseResourceGroup(const BlockStackIterator& begin, const BlockStackIterator& end, const SourceData& sourceData, Blueprint &blueprint)
{
    Result result;
    Section currentSection = ResourceGroupSection;
    BlockStackIterator currentBlock = begin;
    ResourceGroup resourceGroup;
    Collection<ResourceGroup>::type::iterator duplicate = blueprint.resourceGroups.end();
    
    while (currentSection == ResourceGroupSection &&
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
            Collection<ResourceGroup>::type::iterator duplicate = FindResourceGroup(blueprint, resourceGroup.name);
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
        currentSection = BlockSection(currentBlock, currentSection);
    }
    
    if (duplicate != blueprint.resourceGroups.end()) {
        
        duplicate->description += resourceGroup.description;
    }
    else {
        
        blueprint.resourceGroups.push_back(resourceGroup); // FIXME: C++11 move
    }
    
    return std::make_pair(result, currentBlock);
}

// Parse Resource
ParseSectionResult ParseResource(const BlockStackIterator& begin, const BlockStackIterator& end, const SourceData& sourceData, Blueprint &blueprint)
{
    Result result;
    Section currentSection = ResourceSection;
    BlockStackIterator currentBlock = begin;
    
    while (currentSection == ResourceSection &&
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
void BlueprintParser::parse(const SourceData& sourceData, const MarkdownBlock& source, Result& result, Blueprint& blueprint)
{
    //Blueprint blueprint;
    //Result result;
    
    // Initial section
    Section currentSection = OverviewSection;
    
    // Iterate over top-level blocks & parse any sections recognized
    MarkdownBlock::Stack::const_iterator currentBlock = source.blocks.begin();
    while (currentBlock != source.blocks.end()) {

        currentSection = BlockSection(currentBlock, currentSection);

        ParseSectionResult sectionResult;
        if (currentSection == OverviewSection) {
            sectionResult = ParseOverview(currentBlock, source.blocks.end(), sourceData, blueprint);
        }
        else if (currentSection == ResourceGroupSection) {
            sectionResult = ParseResourceGroup(currentBlock, source.blocks.end(), sourceData, blueprint);
        }
        else if (currentSection == ResourceSection) {
            
            // TODO: Construct anonymous resource group and parse it from there
            sectionResult = ParseResource(currentBlock, source.blocks.end(), sourceData, blueprint);
        }
        
        // Append result error & warning data
        result += sectionResult.first;
        
        // Check error
        if (result.error.code != Error::OK) {
            break;
        }
        
        // Proceed to the next block
        currentSection = UndefinedSection;
        if (currentBlock != sectionResult.second) {
            currentBlock = sectionResult.second;
        }
        else {
            ++currentBlock;
        }
    }
}
