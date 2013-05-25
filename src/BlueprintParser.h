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
#include <sstream>
#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "ResourceParser.h"
#include "ResourceGroupParser.h"

namespace snowcrash {
    
    //
    // Block Classifier, Resource Context
    //
    template <>
    inline Section ClassifyBlock<Blueprint>(const BlockIterator& begin,
                                            const BlockIterator& end,
                                            const Section& context) {
        
        if (begin->type == HRuleBlockType)
            return TerminatorSection;
        
        if (HasResourceSignature(*begin))
            return ResourceGroupSection; // Treat Resource as anonymous resource group
        
        if (context == ResourceGroupSection ||
            context == TerminatorSection)
            return ResourceGroupSection;
        
        return BlueprintSection;
    }
    
    //
    // Blueprint Section Parser
    //
    template<>
    struct SectionParser<Blueprint> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Blueprint& output) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            
            switch (section) {
                case TerminatorSection:
                    result.second = ++BlockIterator(cur);
                    break;
                    
                case BlueprintSection:
                    result = HandleBlueprintOverviewBlock(cur, bounds, sourceData, blueprint, output);
                    break;
                    
                case ResourceGroupSection:
                    result = HandleResourceGroup(cur, bounds.second, sourceData, blueprint, output);
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, cur->sourceMap);
                    break;
            }
            
            return result;
        }

        static ParseSectionResult HandleBlueprintOverviewBlock(const BlockIterator& cur,
                                                               const SectionBounds& bounds,
                                                               const SourceData& sourceData,
                                                               const Blueprint& blueprint,
                                                               Blueprint& output) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur(cur);
            if (sectionCur->type == HeaderBlockType &&
                sectionCur == bounds.first) {
                output.name = cur->content;
            }
            else {
                if (sectionCur == bounds.first) {
                    
                    // WARN: No API name specified
                    result.first.warnings.push_back(Warning("expected API name, e.g. `# <API Name>`",
                                                            0,
                                                            cur->sourceMap));
                }
                

                if (sectionCur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(cur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (sectionCur->type == ListBlockBeginType) {
                    sectionCur = SkipToSectionEnd(cur, bounds.second, ListBlockBeginType, ListBlockEndType);
                }
                
                output.description += MapSourceData(sourceData, sectionCur->sourceMap);
            }
            
            result.second = ++sectionCur;
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
                std::stringstream ss;
                if (resourceGroup.name.empty()) {
                    ss << "anonymous group";
                }
                else {
                    ss << "group `" << resourceGroup.name << "`";
                }
                ss << " is already defined";
                
                result.first.warnings.push_back(Warning(ss.str(), 0, begin->sourceMap));
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
