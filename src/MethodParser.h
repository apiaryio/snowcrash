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
#include "PayloadParser.h"

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
    inline Section TClassifyBlock<Method>(const BlockIterator& begin,
                                          const BlockIterator& end,
                                          const Section& context) {

        if (HasMethodSignature(*begin))
            return (context == UndefinedSection) ? MethodSection : UndefinedSection;
        
        PayloadSignature payload = GetPayloadSignature(begin, end);
        if (payload == RequestPayloadSignature)
            return RequestSection;
        else if (payload == ResponsePayloadSignature)
            return ResponseSection;
        
        return (context == MethodSection) ? MethodSection : UndefinedSection;
    }
    
    //
    // Resource Section Overview Parser
    //
    template<>
    struct SectionOverviewParser<Method>  {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Method& method) {
            if (section != MethodSection)
                return std::make_pair(Result(), cur);
            
            Result result;
            BlockIterator sectionCur(cur);
            if (cur->type == HeaderBlockType &&
                cur == bounds.first) {
                method.method = cur->content;
            }
            else {
                
                if (sectionCur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (sectionCur->type == ListBlockBeginType) {
                    sectionCur = SkipToListBlockEndChecking(sectionCur, bounds.second, result);
                }
                
                method.description += MapSourceData(sourceData, sectionCur->sourceMap);
            }
            
            return std::make_pair(result, ++sectionCur);
        }
        
        // Skips to block list' end as SkipToSectionEnd with ListBlockBeginType
        // Checks for recognized sections & warns, begin should be of ListBlockBeginType type
        static BlockIterator SkipToListBlockEndChecking(const BlockIterator& begin,
                                                        const BlockIterator& end,
                                                        Result& result) {
            BlockIterator cur(begin);
            if (++cur == end)
                return cur;
            
            while (cur != end &&
                   cur->type == ListItemBlockBeginType) {
                
                // Check payload signature
                PayloadSignature payload = GetPayloadSignature(cur, end);
                cur = SkipToSectionEnd(cur, end, ListItemBlockBeginType, ListItemBlockEndType);
                if (cur == end)
                    break;
                
                if (payload == RequestPayloadSignature) {
                    result.warnings.push_back(Warning("ignoring request in method description", 0, cur->sourceMap));
                }
                else if (payload == ResponsePayloadSignature) {
                    result.warnings.push_back(Warning("ignoring response in method description", 0, cur->sourceMap));
                }
                
                // TODO: Headers & Parameters check
                
                ++cur;
            }
            
            return cur;
        }
    };
    
    typedef BlockParser<Method, SectionOverviewParser<Method> > MethodOverviewParser;
    
    //
    // Method Section Parser
    //
    template<>
    struct SectionParser<Method> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Method& method) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            
            switch (section) {
                case MethodSection:
                    result = MethodOverviewParser::Parse(cur, bounds.second, sourceData, blueprint, method);
                    break;
                    
                case RequestSection:
                    result = HandleRequest(cur, bounds.second, sourceData, blueprint, method);
                    break;
                
                case ResponseSection:
                    result = HandleResponse(cur, bounds.second, sourceData, blueprint, method);
                    break;
                    
                case UndefinedSection:
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, cur->sourceMap);
                    break;
            }
            
            return result;
        }
        
        static ParseSectionResult HandleRequest(const BlockIterator& begin,
                                                const BlockIterator& end,
                                                const SourceData& sourceData,
                                                const Blueprint& blueprint,
                                                Method& method)
        {
            Request request;
            ParseSectionResult result = PayloadParser::Parse(begin, end, sourceData, blueprint, request);
            Collection<Response>::const_iterator duplicate = FindRequest(method, request);
            if (duplicate != method.requests.end()) {
                
                // WARN: duplicate request
                result.first.warnings.push_back(Warning("request '" +
                                                        request.name +
                                                        "' already defined for '" +
                                                        method.method +
                                                        "' method",
                                                        0,
                                                        begin->sourceMap));
            }
            
            method.requests.push_back(request);
            return result;
        }
        
        static ParseSectionResult HandleResponse(const BlockIterator& begin,
                                                 const BlockIterator& end,
                                                 const SourceData& sourceData,
                                                 const Blueprint& blueprint,
                                                 Method& method)
        {
            Response response;
            ParseSectionResult result = PayloadParser::Parse(begin, end, sourceData, blueprint, response);            
            Collection<Response>::const_iterator duplicate = FindResponse(method, response);
            if (duplicate != method.responses.end()) {

                // WARN: duplicate response
                result.first.warnings.push_back(Warning("response '" +
                                                        response.name +
                                                        "' already defined for '" +
                                                        method.method +
                                                        "' method",
                                                        0,
                                                        begin->sourceMap));
            }
            
            method.responses.push_back(response);
            return result;
        }
        
    };
    
    typedef BlockParser<Method, SectionParser<Method> > MethodParser;
}

#endif
