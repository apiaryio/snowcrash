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
    inline Section ClassifyBlock<Method>(const BlockIterator& begin,
                                         const BlockIterator& end,
                                         const Section& context) {
        
        if (begin->type == HRuleBlockType)
            return TerminatorSection;
        
        if (context == TerminatorSection)
            return UndefinedSection;

        if (HasMethodSignature(*begin))
            return (context == UndefinedSection) ? MethodSection : UndefinedSection;

        PayloadSignature payload = GetPayloadSignature(begin, end);
        if (payload == RequestPayloadSignature)
            return RequestSection;
        else if (payload == ResponsePayloadSignature)
            return ResponseSection;
        
        // Unrecognized list item at this level
        if (begin->type == ListItemBlockBeginType)
            return ForeignSection;
        
        return (context == MethodSection) ? MethodSection : UndefinedSection;
    }
    
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
                case TerminatorSection:
                    if (result.second != bounds.second)
                        ++result.second;
                    break;
                    
                case MethodSection:
                    result = HandleMethodOverviewBlock(cur, bounds, sourceData, blueprint, method);
                    break;
                    
                case RequestSection:
                    result = HandleRequest(cur, bounds.second, sourceData, blueprint, method);
                    break;
                
                case ResponseSection:
                    result = HandleResponse(cur, bounds.second, sourceData, blueprint, method);
                    break;
                    
                case ForeignSection:
                    result = HandleForeignSection(cur, bounds);
                    break;
                    
                case UndefinedSection:
                    result.second = CloseListItemBlock(cur, bounds.second);
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, cur->sourceMap);
                    break;
            }
            
            return result;
        }
        
        static ParseSectionResult HandleMethodOverviewBlock(const BlockIterator& cur,
                                                            const SectionBounds& bounds,
                                                            const SourceData& sourceData,
                                                            const Blueprint& blueprint,
                                                            Method& method) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
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
                    sectionCur = SkipToListBlockEndChecking(sectionCur, bounds.second, result.first);
                }
                
                method.description += MapSourceData(sourceData, sectionCur->sourceMap);
            }
            
            result.second = ++sectionCur;
            return result;
        }
        
        // Skips to block list end as in SkipToSectionEnd() with ListBlockBeginType.
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
                
                if (payload == RequestPayloadSignature) {
                    result.warnings.push_back(Warning("ignoring request in description, description should not end with list",
                                                      0,
                                                      (cur != end) ? cur->sourceMap : MakeSourceDataBlock(0, 0)));
                }
                else if (payload == ResponsePayloadSignature) {
                    result.warnings.push_back(Warning("ignoring response in description, description should not end with list",
                                                      0,
                                                      (cur != end) ? cur->sourceMap : MakeSourceDataBlock(0, 0)));
                }
                
                // TODO: Headers & Parameters check
                
                
                if (cur != end)
                    ++cur;
            }
            
            return cur;
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
