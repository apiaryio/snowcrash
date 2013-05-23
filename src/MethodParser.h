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
#include "HeaderParser.h"

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
    // Classifier of internal list items, Payload context
    //
    template <>
    inline Section ClassifyInternaListBlock<Method>(const BlockIterator& begin,
                                                    const BlockIterator& end) {
        
        if (HasHeaderSignature(begin, end))
            return HeadersSection;
        
        PayloadSignature payload = GetPayloadSignature(begin, end);
        if (payload == RequestPayloadSignature)
            return RequestSection;
        else if (payload == ResponsePayloadSignature)
            return ResponseSection;
        
        return UndefinedSection;
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
        
        if (HasResourceSignature(*begin))
            return UndefinedSection;

        if (HasMethodSignature(*begin))
            return (context == UndefinedSection) ? MethodSection : UndefinedSection;
        
        Section listSection = ClassifyInternaListBlock<Method>(begin, end);
        if (listSection != UndefinedSection)
            return listSection;
        
        // Unrecognized list item at this level
        if (begin->type == ListItemBlockBeginType)
            return ForeignSection;
        
        return (context == MethodSection) ? MethodSection : UndefinedSection;
    }
    
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
                    
                case HeadersSection:
                    result = HandleHeaders(cur, bounds.second, sourceData, blueprint, method);
                    break;
                    
                case RequestSection:
                case ResponseSection:
                    result = HandlePayload(section, cur, bounds.second, sourceData, blueprint, method);
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
                    sectionCur = SkipToDescriptionListEnd<Method>(sectionCur, bounds.second, result.first);
                }
                
                method.description += MapSourceData(sourceData, sectionCur->sourceMap);
            }
            
            result.second = ++sectionCur;
            return result;
        }
        
        static ParseSectionResult HandlePayload(const Section &section,
                                                const BlockIterator& begin,
                                                const BlockIterator& end,
                                                const SourceData& sourceData,
                                                const Blueprint& blueprint,
                                                Method& method)
        {
            Payload payload;
            ParseSectionResult result = PayloadParser::Parse(begin, end, sourceData, blueprint, payload);
            if (result.first.error.code != Error::OK)
                return result;
            
            if (IsPayloadDuplicate(section, payload, method)) {
                // WARN: duplicate payload
                std::stringstream ss;
                ss << SectionName(section) << " payload `" << payload.name << "`";
                ss << " already defined for `" << method.method << "` method";
                
                result.first.warnings.push_back(Warning(ss.str(),
                                                        0,
                                                        begin->sourceMap));
                
            }
            
            if (section == RequestSection)
                method.requests.push_back(payload);
            else if (section == ResponseSection)
                method.responses.push_back(payload);
            
            return result;
        }
        
        // Checks whether given section payload has duplicate.
        // Returns true when a duplicate is found, false otherwise.
        static bool IsPayloadDuplicate(const Section& section, const Payload& payload, Method& method) {
            
            if (section == RequestSection) {
                Collection<Request>::const_iterator duplicate = FindRequest(method, payload);
                return duplicate != method.requests.end();
            }
            else if (section == ResponseSection) {
                Collection<Response>::const_iterator duplicate = FindResponse(method, payload);
                return duplicate != method.responses.end();
            }

            return false;
        }
    };
    
    typedef BlockParser<Method, SectionParser<Method> > MethodParser;
}

#endif
