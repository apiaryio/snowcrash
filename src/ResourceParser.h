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

    // Resource iterator in its containment group
    typedef Collection<Resource>::const_iterator ResourceIterator;
    
    // Finds a resource in resource group by its URI template
    inline ResourceIterator FindResource(const ResourceGroup& group,
                                         const Resource& resource) {
        
        return std::find_if(group.resources.begin(),
                            group.resources.end(),
                            std::bind2nd(MatchURI<Resource>(), resource));
    }

    // Resource iterator pair: its containment group and resource iterator itself
    typedef std::pair<Collection<ResourceGroup>::const_iterator, ResourceIterator> ResourceIteratorPair;
    
    // Finds a resource in blueprint by its URI template
    inline ResourceIteratorPair FindResource(const Blueprint& blueprint, const Resource& resource) {
        
        for (Collection<ResourceGroup>::const_iterator it = blueprint.resourceGroups.begin();
             it != blueprint.resourceGroups.end();
             ++it) {
            
            Collection<Resource>::const_iterator match = FindResource(*it, resource);
            if (match != it->resources.end())
                return std::make_pair(it, match);
        }
        
        return std::make_pair(blueprint.resourceGroups.end(), Collection<Resource>::iterator());
    }
    
    
    //
    // Classifier of internal list items, Resource context
    //
    template <>
    inline Section ClassifyInternaListBlock<Resource>(const BlockIterator& begin,
                                                      const BlockIterator& end) {
        if (HasHeaderSignature(begin, end))
            return HeadersSection;

        return UndefinedSection;
    }
    
    //
    // Block Classifier, Resource Context
    //
    template <>
    inline Section ClassifyBlock<Resource>(const BlockIterator& begin,
                                           const BlockIterator& end,
                                           const Section& context) {

        if (begin->type == HRuleBlockType)
            return TerminatorSection;
        
        if (context == TerminatorSection)
            return UndefinedSection;
        
        if (HasResourceSignature(*begin))
            return (context == UndefinedSection) ? ResourceSection : UndefinedSection;;
        
        if (HasMethodSignature(*begin))
            return MethodSection;
        
        Section listSection = ClassifyInternaListBlock<Resource>(begin, end);
        if (listSection != UndefinedSection)
            return listSection;
        
        // Unrecognized list item at this level
        if (begin->type == ListItemBlockBeginType)
            return ForeignSection;
        
        return (context == ResourceSection) ? context : UndefinedSection;
    }
        
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
                case TerminatorSection:
                    if (result.second != bounds.second)
                        ++result.second;
                    break;
                    
                case ResourceSection:
                    result = HandleResourceOverviewBlock(cur, bounds, sourceData, blueprint, resource);
                    break;
                    
                case HeadersSection:
                    result = HandleHeaders(cur, bounds.second, sourceData, blueprint, resource);
                    break;
                    
                case MethodSection:
                    result = HandleMethod(cur, bounds.second, sourceData, blueprint, resource);
                    break;
                    
                case UndefinedSection:
                    result.second = CloseListItemBlock(cur, bounds.second);
                    break;
                    
                case ForeignSection:
                    result = HandleForeignSection(cur, bounds);
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, cur->sourceMap);
                    break;
            }
            
            return result;
        }
        
        static ParseSectionResult HandleResourceOverviewBlock(const BlockIterator& cur,
                                                              const SectionBounds& bounds,
                                                              const SourceData& sourceData,
                                                              const Blueprint& blueprint,
                                                              Resource& resource) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
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
                    sectionCur = SkipToDescriptionListEnd<Resource>(sectionCur, bounds.second, result.first);
                }
                
                resource.description += MapSourceData(sourceData, sectionCur->sourceMap);
            }
            
            result.second = ++sectionCur;
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
                result.first.warnings.push_back(Warning("method `" +
                                                        method.method +
                                                        "` already defined for resource `" +
                                                        resource.uri +
                                                        "`",
                                                        0,
                                                        begin->sourceMap));
            }
            
            DeepCheckHeaderDuplicates(resource, method, begin->sourceMap, result.first);
            
            if (method.responses.empty()) {
                // WARN: method has no response
                result.first.warnings.push_back(Warning("no response defined for `" +
                                                        method.method +
                                                        " " +
                                                        resource.uri +
                                                        "`",
                                                        0,
                                                        begin->sourceMap));
            }
            
            resource.methods.push_back(method);
            return result;
        }
        
        static void DeepCheckHeaderDuplicates(const Resource& resource,
                                              const Method& method,
                                              const SourceDataBlock& sourceMap,
                                              Result& result) {
            
            CheckHeaderDuplicates(resource, method, sourceMap, result);
            for (Collection<Request>::const_iterator it = method.requests.begin();
                 it != method.requests.end();
                 ++it) {
                
                CheckHeaderDuplicates(resource, *it, sourceMap, result);
            }
            for (Collection<Response>::const_iterator it = method.responses.begin();
                 it != method.responses.end();
                 ++it) {
                
                CheckHeaderDuplicates(resource, *it, sourceMap, result);
            }
        }
    };
    
    typedef BlockParser<Resource, SectionParser<Resource> > ResourceParser;
}

#endif
