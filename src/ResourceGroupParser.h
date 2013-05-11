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
        
    //
    // Block Classifier, Resource Group Context
    //
    template <>
    inline Section TClassifyBlock<ResourceGroup>(const BlockIterator& begin,
                                                 const BlockIterator& end,
                                                 const Section& context) {
        
        if (HasResourceSignature(*begin))
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
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               ResourceGroup& group) {
            if (section != ResourceGroupSection)
                return std::make_pair(Result(), cur);
            
            Result result;
            BlockIterator sectionCur(cur);
            if (sectionCur->type == HeaderBlockType &&
                sectionCur == bounds.first) {
                group.name = sectionCur->content;
            }
            else {
                if (sectionCur == bounds.first) {
                    
                    // WARN: No API name specified
                    result.warnings.push_back(Warning("expected resources group name",
                                                      0,
                                                      sectionCur->sourceMap));
                }
                
                
                if (cur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (cur->type == ListBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, ListBlockBeginType, ListBlockEndType);
                }
                
                group.description += MapSourceData(sourceData, sectionCur->sourceMap);
            }
            
            return std::make_pair(result, ++sectionCur);
        }
    };
    
    typedef BlockParser<ResourceGroup, SectionOverviewParser<ResourceGroup> > ResourceGroupOverviewParser;    
    
    //
    // Resource Group Section Parser
    //
    template<>
    struct SectionParser<ResourceGroup> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               ResourceGroup& group) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            switch (section) {
                case ResourceGroupSection:
                    
                    result = ResourceGroupOverviewParser::Parse(cur, bounds.second, sourceData, blueprint, group);
                    break;
                    
                case ResourceSection:

                    result = HandleResource(cur, bounds.second, sourceData, blueprint, group);
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, cur->sourceMap);
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