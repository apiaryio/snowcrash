//
//  ResourceParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_RESOURCEPARSER_H
#define SNOWCRASH_RESOURCEPARSER_H

#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "MethodParser.h"
#include "RegexMatch.h"

static const std::string ResourceHeaderRegex("^((" HTTP_METHODS ")[[:space:]]+)?/.*$");

namespace snowcrash {
    
    // Returns true if block has resource header signature, false otherwise
    inline bool HasResourceSignature(const MarkdownBlock& block) {

        if (block.type != HeaderBlockType || block.content.empty())
            return false;
        
        return RegexMatch(block.content, ResourceHeaderRegex);
    }

    // Resource match iterators
    typedef std::pair<Collection<ResourceGroup>::const_iterator, Collection<Resource>::const_iterator> ResourceMatch;

    // Finds a resource in blueprint by its URI template
    inline ResourceMatch FindResource(const Blueprint& blueprint, const Resource& resource) {
        
        for (Collection<ResourceGroup>::const_iterator it = blueprint.resourceGroups.begin();
             it != blueprint.resourceGroups.end();
             ++it) {
            
            Collection<Resource>::const_iterator match = std::find_if(it->resources.begin(),
                                                                      it->resources.end(),
                                                                      std::bind2nd(MatchURI<Resource>(), resource));
            if (match != it->resources.end())
                return std::make_pair(it, match);
        }
        
        return std::make_pair(blueprint.resourceGroups.end(), Collection<Resource>::iterator());
    }
    
    //
    // Block Classifier, Resource Context
    //
    template <>
    inline Section TClassifyBlock<Resource>(const BlockIterator& begin,
                                            const BlockIterator& end,
                                            const Section& context) {
        
        if (HasResourceSignature(*begin)) {
            return (context == UndefinedSection) ? ResourceSection : UndefinedSection;
        }
        else if (HasMethodSignature(*begin)) {
            return MethodSection;
        }
        
        return (context != ResourceSection) ? UndefinedSection : ResourceSection;
    }
    
    //
    // Resource Section Overview Parser
    //
    template<>
    struct SectionOverviewParser<Resource>  {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Resource& resource) {
            if (section != ResourceSection)
                return std::make_pair(Result(), cur);
            
            Result result;
            BlockIterator sectionCur(cur);
            if (cur->type == HeaderBlockType &&
                cur == bounds.first) {
                resource.uri = cur->content;
            }
            else {
                
                if (cur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (cur->type == ListBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, ListBlockBeginType, ListBlockEndType);
                }
                
                resource.description += MapSourceData(sourceData, sectionCur->sourceMap);
            }
            
            return std::make_pair(Result(), ++sectionCur);
        }
    };
    
    typedef BlockParser<Resource, SectionOverviewParser<Resource> > ResourceOverviewParser;
    
    //
    // Resource Section Parser
    //
    template<>
    struct SectionParser<Resource> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Resource& resource) {

            ParseSectionResult result = std::make_pair(Result(), cur);
            switch (section) {
                case ResourceSection:
                    result = ResourceOverviewParser::Parse(cur, bounds.second, sourceData, blueprint, resource);
                    break;
                    
                case MethodSection:
                    result = HandleMethod(cur, bounds.second, sourceData, blueprint, resource);
                    break;
                    
                case UndefinedSection:
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, cur->sourceMap);
                    break;
            }
            
            return result;
        }
        
        static ParseSectionResult HandleMethod(const BlockIterator& begin,
                                               const BlockIterator& end,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Resource& resource)
        {
            Method method;
            ParseSectionResult result = MethodParser::Parse(begin, end, sourceData, blueprint, method);
            if (result.first.error.code != Error::OK)
                return result;
            
            Collection<Method>::iterator duplicate = FindMethod(resource, method);
            if (duplicate != resource.methods.end()) {
                
                // WARN: duplicate method
                result.first.warnings.push_back(Warning("method '" +
                                                        method.method +
                                                        "' already defined for resource '" +
                                                        resource.uri +
                                                        "'",
                                                        0,
                                                        begin->sourceMap));
            }
            
            resource.methods.push_back(method);
            return result;
        }
    };
    
    typedef BlockParser<Resource, SectionParser<Resource> > ResourceParser;
}

#endif
