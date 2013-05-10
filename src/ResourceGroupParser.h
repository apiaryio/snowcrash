//
//  ResourceGroupParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_RESOURCEGROUPPARSER_H
#define SNOWCRASH_RESOURCEGROUPPARSER_H

#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "ResourceParser.h"

namespace snowcrash {
    
    // Finds a group in blueprint by name
    inline Collection<ResourceGroup>::const_iterator FindResourceGroup(const Blueprint& blueprint,
                                                                       const ResourceGroup& group) {

        return std::find_if(blueprint.resourceGroups.begin(),
                            blueprint.resourceGroups.end(),
                            std::bind2nd(MatchName<ResourceGroup>(), group));
    }
    
    // Parse Resource Group descending into Resources
//    ParseSectionResult ParseResourceGroup(const BlockIterator& begin,
//                                          const BlockIterator& end,
//                                          const SourceData& sourceData,
//                                          const Blueprint& blueprint,
//                                          ResourceGroup& group);
    
    //
    // Block Classifier, Resource Group Context
    //
    template <>
    inline Section TClassifyBlock<ResourceGroup>(const MarkdownBlock& block, const Section& context) {
        
        if (HasResourceSignature(block))
            return ResourceSection;
        else if (context == ResourceSection)
            return UndefinedSection;
        else
            return ResourceGroupSection;
    }
    
    //
    // Resource Group Section Overview Parser
    //
    template<>
    struct SectionOverviewParser<ResourceGroup>  {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& begin,
                                               const BlockIterator& end,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               ResourceGroup& group) {
            if (section != ResourceGroupSection)
                return std::make_pair(Result(), begin);
            
            Result result;
            BlockIterator cur(begin);
            if (begin->type == HeaderBlockType &&
                group.description.empty()) {
                group.name = begin->content;
            }
            else {
                if (group.name.empty() &&
                    group.description.empty()) {
                    
                    // WARN: No API name specified
                    result.warnings.push_back(Warning("expected resources group name",
                                                      0,
                                                      begin->sourceMap));
                }
                
                
                if (begin->type == QuoteBlockBeginType) {
                    cur = SkipToSectionEnd(begin, end, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (begin->type == ListBlockBeginType) {
                    cur = SkipToSectionEnd(begin, end, ListBlockBeginType, ListBlockEndType);
                }
                
                group.description += MapSourceData(sourceData, cur->sourceMap);
            }
            
            return std::make_pair(result, ++cur);
        }
    };
    
    typedef BlockParser<ResourceGroup, SectionOverviewParser<ResourceGroup> > ResourceGroupOverviewParser;    
    
    //
    // Resource Group Section Parser
    //
    template<>
    struct SectionParser<ResourceGroup> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& begin,
                                               const BlockIterator& end,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               ResourceGroup& group) {
            
            ParseSectionResult result = std::make_pair(Result(), begin);
            switch (section) {
                case ResourceGroupSection:
                    
                    result = ResourceGroupOverviewParser::Parse(begin, end, sourceData, blueprint, group);
                    break;
                    
                case ResourceSection:

                    result = HandleResource(begin, end, sourceData, blueprint, group);
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, begin->sourceMap);
                    break;
            }
            
            return result;
        }
        
        static ParseSectionResult HandleResource(const BlockIterator& begin,
                                                 const BlockIterator& end,
                                                 const SourceData& sourceData,
                                                 const Blueprint& blueprint,
                                                 ResourceGroup& group)
        {
            Resource resource;
            ParseSectionResult result = ResourceParser::Parse(begin, end, sourceData, blueprint, resource);
            if (result.first.error.code != Error::OK)
                return result;
            
            ResourceMatch duplicate = FindResource(blueprint, resource);
            if (duplicate.first != blueprint.resourceGroups.end()) {
                
                // WARN: duplicate resource
                result.first.warnings.push_back(Warning("resource '" +
                                                        resource.uri +
                                                        "' already exists",
                                                        0,
                                                        begin->sourceMap));
            }
            
            group.resources.push_back(resource); // FIXME: C++11 move
            return result;
        }
    };
    
    typedef BlockParser<ResourceGroup, SectionParser<ResourceGroup> > ResourceGroupParser;
}

#endif