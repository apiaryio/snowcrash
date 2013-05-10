//
//  BlueprintParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_BLUEPRINTPARSER_H
#define SNOWCRASH_BLUEPRINTPARSER_H

#include <functional>
#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "ResourceParser.h"
#include "ResourceGroupParser.h"

namespace snowcrash {
    
    //
    // Block Classifier, Resource Context
    //
    template <>
    inline Section TClassifyBlock<Blueprint>(const MarkdownBlock& block, const Section& context) {
        
        if (block.type == HRuleBlockType)
            return TerminatorSection;
        
        if (HasResourceSignature(block))
            return ResourceGroupSection; // Treat Resource as anonymous resource group
        
        if (context == ResourceGroupSection ||
            context == TerminatorSection)
            return ResourceGroupSection;
        
        return BlueprintSection;
    }
    
    //
    // Blueprint Section Overview Parser
    //
    template<>
    struct SectionOverviewParser<Blueprint>  {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& begin,
                                               const BlockIterator& end,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Blueprint& output) {
            if (section != BlueprintSection)
                return std::make_pair(Result(), begin);
            
            Result result;
            BlockIterator cur(begin);
            if (begin->type == HeaderBlockType &&
                output.description.empty()) {
                output.name = begin->content;
            }
            else {
                if (output.name.empty() &&
                    output.description.empty()) {
                    
                    // WARN: No API name specified
                    result.warnings.push_back(Warning("expected API name",
                                                      0,
                                                      begin->sourceMap));
                }
                

                if (begin->type == QuoteBlockBeginType) {
                    cur = SkipToSectionEnd(begin, end, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (begin->type == ListBlockBeginType) {
                    cur = SkipToSectionEnd(begin, end, ListBlockBeginType, ListBlockEndType);
                }
                
                output.description += MapSourceData(sourceData, cur->sourceMap);
            }
            
            return std::make_pair(result, ++cur);
        }
    };
    
    typedef BlockParser<Blueprint, SectionOverviewParser<Blueprint> > BlueprintOverviewParser;
    
    //
    // Blueprint Section Parser
    //
    template<>
    struct SectionParser<Blueprint> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& begin,
                                               const BlockIterator& end,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Blueprint& output) {
            
            ParseSectionResult result = std::make_pair(Result(), begin);
            
            switch (section) {
                case TerminatorSection:
                    result.second = ++BlockIterator(begin);
                    break;
                    
                case BlueprintSection:
                    
                    result = BlueprintOverviewParser::Parse(begin, end, sourceData, blueprint, output);
                    break;
                    
                case ResourceGroupSection:
                    result = HandleResourceGroup(begin, end, sourceData, blueprint, output);
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, begin->sourceMap);
                    break;
            }
            
            return result;
        }
        
        static ParseSectionResult HandleResourceGroup(const BlockIterator& begin,
                                                      const BlockIterator& end,
                                                      const SourceData& sourceData,
                                                      const Blueprint& blueprint,
                                                      Blueprint& output)
        {
            ResourceGroup resourceGroup;
            ParseSectionResult result = ResourceGroupParser::Parse(begin, end, sourceData, blueprint, resourceGroup);
            if (result.first.error.code != Error::OK)
                return result;
            
            Collection<ResourceGroup>::const_iterator duplicate = FindResourceGroup(blueprint, resourceGroup);
            if (duplicate != blueprint.resourceGroups.end()) {
                
                // WARN: duplicate group
                result.first.warnings.push_back(Warning("group '" +
                                                        resourceGroup.name +
                                                        "' already exists",
                                                        0,
                                                        begin->sourceMap));
            }
            
            output.resourceGroups.push_back(resourceGroup); // FIXME: C++11 move
            return result;
        }
        
    };
    
    typedef BlockParser<Blueprint, SectionParser<Blueprint> > BlueprintParserInner;
    
    
    //
    // Blueprint Parser
    //
    class BlueprintParser {
    public:
        // Parse Markdown AST into API Blueprint AST
        static void Parse(const SourceData& sourceData,
                          const MarkdownBlock::Stack& source,
                          Result& result,
                          Blueprint& blueprint) {
            
            ParseSectionResult sectionResult = BlueprintParserInner::Parse(source.begin(),
                                                                           source.end(),
                                                                           sourceData,
                                                                           blueprint,
                                                                           blueprint);
            result += sectionResult.first;
        }
    };
}

#endif
