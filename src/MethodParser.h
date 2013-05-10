//
//  MethodParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_METHODPARSER_H
#define SNOWCRASH_METHODPARSER_H

#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "RegexMatch.h"

static const std::string MethodHeaderRegex("^(" HTTP_METHODS ")[[:space:]]*$");

namespace snowcrash {
    
    // Returns true if block has HTTP Method signature, false otherwise
    inline bool HasMethodSignature(const MarkdownBlock& block) {
        if (block.type != HeaderBlockType ||
            block.content.empty())
            return false;
        
        return RegexMatch(block.content, MethodHeaderRegex);
    }
    
    // Finds a method inside resource
    inline Collection<Method>::iterator FindMethod(Resource& resource, const Method& method) {
        return std::find_if(resource.methods.begin(),
                            resource.methods.end(),
                            std::bind2nd(MatchMethod<Method>(), method));
    }
        
    //
    // Block Classifier, Method Context
    //
    template <>
    inline Section TClassifyBlock<Method>(const MarkdownBlock& block, const Section& context) {

        if (block.type != HeaderBlockType)
            return (context != MethodSection) ? UndefinedSection : MethodSection;
        
        if (block.type == HeaderBlockType && HasMethodSignature(block))
            return (context != MethodSection) ? MethodSection : UndefinedSection;
        
        return (context != MethodSection) ? UndefinedSection : MethodSection;
    }
    
    //
    // Resource Section Overview Parser
    //
    template<>
    struct SectionOverviewParser<Method>  {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& begin,
                                               const BlockIterator& end,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Method& method) {
            if (section != MethodSection)
                return std::make_pair(Result(), begin);
            
            if (begin->type == HeaderBlockType &&
                method.description.empty()) {
                method.method = begin->content;
            }
            else {
                method.description += MapSourceData(sourceData, begin->sourceMap);
            }
            
            return std::make_pair(Result(), ++BlockIterator(begin));
        }
    };
    
    typedef BlockParser<Method, SectionOverviewParser<Method> > MethodOverviewParser;
    
    //
    // Method Section Parser
    //
    template<>
    struct SectionParser<Method> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& begin,
                                               const BlockIterator& end,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Method& method) {
            
            ParseSectionResult result = std::make_pair(Result(), begin);
            switch (section) {
                case MethodSection:
                    
                    result = MethodOverviewParser::Parse(begin, end, sourceData, blueprint, method);
                    break;
                    
                case UndefinedSection:
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, begin->sourceMap);
                    break;
            }
            
            return result;
        }
    };
    
    typedef BlockParser<Method, SectionParser<Method> > MethodParser;
}

#endif
