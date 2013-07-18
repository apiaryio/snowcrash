//
//  MethodParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_METHODPARSER_H
#define SNOWCRASH_METHODPARSER_H

#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "RegexMatch.h"
#include "PayloadParser.h"
#include "HeaderParser.h"
#include "HTTP.h"

static const std::string MethodHeaderRegex("^(" HTTP_METHODS ")[ \\t]*(" URI_TEMPLATE ")?$");
static const std::string NamedMethodHeaderRegex("^([^\\[]*)\\[(" HTTP_METHODS ")]$");

namespace snowcrash {
    
    // Method signature
    enum MethodSignature {
        UndefinedMethodSignature,
        NoMethodSignature,
        MethodMethodSignature,      // # GET
        MethodURIMethodSignature,   // # GET /uri
        NamedMethodSignature        // # My Method [GET]
    };
    
    // Query method signature
    FORCEINLINE MethodSignature GetMethodSignature(const MarkdownBlock& block,
                                                   Name& name,
                                                   HTTPMethod& method) {
        if (block.type != HeaderBlockType ||
            block.content.empty())
            return NoMethodSignature;
        
        CaptureGroups captureGroups;
        if (RegexCapture(block.content, MethodHeaderRegex, captureGroups, 3)) {
            // Nameless method
            method = captureGroups[1];
            URITemplate uri = captureGroups[2];
            return (uri.empty()) ? MethodMethodSignature : MethodURIMethodSignature;
        }
        else if (RegexCapture(block.content, NamedMethodHeaderRegex, captureGroups, 3)) {
            // Named method
            name = captureGroups[1];
            TrimString(name);
            method = captureGroups[2];
            return NamedMethodSignature;
        }
        
        return NoMethodSignature;
    }

    // Returns true if block has HTTP Method signature, false otherwise
    FORCEINLINE bool HasMethodSignature(const MarkdownBlock& block) {
        
        if (block.type != HeaderBlockType ||
            block.content.empty())
            return false;
        
        Name name;
        HTTPMethod method;
        return GetMethodSignature(block, name, method) != NoMethodSignature;
    }
    
    // Finds a method inside resource
    FORCEINLINE Collection<Method>::iterator FindMethod(Resource& resource,
                                                        const Method& method) {
        return std::find_if(resource.methods.begin(),
                            resource.methods.end(),
                            std::bind2nd(MatchMethod<Method>(), method));
    }
    
    //
    // Classifier of internal list items, Payload context
    //
    template <>
    FORCEINLINE Section ClassifyInternaListBlock<Method>(const BlockIterator& begin,
                                                         const BlockIterator& end) {        
        if (HasHeaderSignature(begin, end))
            return HeadersSection;
        
        Name name;
        SourceData mediaType;
        PayloadSignature payload = GetPayloadSignature(begin, end, name, mediaType);
        if (payload == RequestPayloadSignature)
            return RequestSection;
        else if (payload == ResponsePayloadSignature)
            return ResponseSection;
        else if (payload == ObjectPayloadSignature)
            return ObjectSection;
        
        return UndefinedSection;
    }
        
    //
    // Block Classifier, Method Context
    //
    template <>
    FORCEINLINE Section ClassifyBlock<Method>(const BlockIterator& begin,
                                              const BlockIterator& end,
                                              const Section& context) {

        if (HasMethodSignature(*begin))
            return (context == UndefinedSection) ? MethodSection : UndefinedSection;
        
        if (HasResourceSignature(*begin) ||
            HasResourceGroupSignature(*begin))
            return UndefinedSection;
        
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
                                               BlueprintParserCore& parser,
                                               Method& method) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            
            switch (section) {                    
                case MethodSection:
                    result = HandleMethodOverviewBlock(cur, bounds, parser, method);
                    break;
                    
                case HeadersSection:
                    result = HandleHeaders(cur, bounds.second, parser, method);
                    break;
                    
                case RequestSection:
                case ResponseSection:
                    result = HandlePayload(section, cur, bounds.second, parser, method);
                    break;
                    
                case ForeignSection:
                    result = HandleForeignSection(cur, bounds);
                    break;
                    
                case UndefinedSection:
                    result.second = CloseListItemBlock(cur, bounds.second);
                    break;
                    
                case ObjectSection:
                    // ERR: Unexpected object definition
                    result.first.error = Error("unexpected object definiton, an object can be only defined in the resource section",
                                               SymbolError,
                                               cur->sourceMap);
                    break;
                    
                default:
                    result.first.error = UnexpectedBlockError(*cur);
                    break;
            }
            
            return result;
        }
        
        static ParseSectionResult HandleMethodOverviewBlock(const BlockIterator& cur,
                                                            const SectionBounds& bounds,
                                                            BlueprintParserCore& parser,
                                                            Method& method) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur(cur);
            if (cur->type == HeaderBlockType &&
                cur == bounds.first) {
                
                GetMethodSignature(*cur, method.name, method.method);
            }
            else {
                
                if (sectionCur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (sectionCur->type == ListBlockBeginType) {
                    
                    SourceDataBlock descriptionMap;
                    sectionCur = SkipToDescriptionListEnd<Method>(sectionCur, bounds.second, descriptionMap);
                    
                    if (sectionCur->type != ListBlockEndType) {
                        if (!descriptionMap.empty())
                            method.description += MapSourceData(parser.sourceData, descriptionMap);
                        
                        result.second = sectionCur;
                        return result;
                    }
                }
                
                if (!CheckCursor(sectionCur, bounds, cur, result.first))
                    return result;
                method.description += MapSourceData(parser.sourceData, sectionCur->sourceMap);
            }
            
            result.second = ++sectionCur;
            return result;
        }
        
        /**
         *  \brief  Parse method payload
         *  \param  begin   The begin of the block to be parsed.
         *  \param  end     The end of the markdown block buffer.
         *  \param  parser  A parser's instance.
         *  \param  method  An output buffer to store parsed payload into.
         *  \return A block parser section result.
         */
        static ParseSectionResult HandlePayload(const Section &section,
                                                const BlockIterator& begin,
                                                const BlockIterator& end,
                                                BlueprintParserCore& parser,
                                                Method& method)
        {
            Payload payload;
            ParseSectionResult result = PayloadParser::Parse(begin, end, parser, payload);
            if (result.first.error.code != Error::OK)
                return result;
            
            // Check for duplicate
            if (IsPayloadDuplicate(section, payload, method)) {
                // WARN: duplicate payload
                std::stringstream ss;
                ss << SectionName(section) << " payload `" << payload.name << "`";
                ss << " already defined for `" << method.method << "` method";
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                result.first.warnings.push_back(Warning(ss.str(),
                                                        DuplicateWarning,
                                                        nameBlock->sourceMap));
            }
            
            BlockIterator nameBlock = ListItemNameBlock(begin, end);

            // Check payload integrity
            CheckPayload(section, payload, nameBlock->sourceMap, result.first);
            
            // Inject parsed payload into method
            if (section == RequestSection) {
                method.requests.push_back(payload);
            }
            else if (section == ResponseSection) {
                method.responses.push_back(payload);
            }
            
            // Check header duplicates
            CheckHeaderDuplicates(method, payload, nameBlock->sourceMap, result.first);
            
            return result;
        }
        
        
        /**
         *  \brief  Check & report payload validity.
         *  \param  section     A section of the payload.
         *  \param  sourceMap   Payload signature source map.
         *  \param  payload     The payload to be checked.
         */
        static void CheckPayload(const Section& section,
                                 const Payload& payload,
                                 const SourceDataBlock& sourceMap,
                                 Result& result) {
            
            bool warnEmptyBody = false;
            if (section == RequestSection) {
                warnEmptyBody = payload.body.empty();
            }
            else if (section == ResponseSection) {
                // Check status code
                HTTPStatusCode code = 0;
                if (!payload.name.empty()) {
                    std::stringstream(payload.name) >> code;
                }
                StatusCodeTraits traits = GetStatusCodeTrait(code);
                if (traits.allowBody) {
                    warnEmptyBody = payload.body.empty();
                }
                else if (!payload.body.empty()) {
                    // WARN: not empty body
                    std::stringstream ss;
                    ss << "the " << code << " response MUST NOT include a " << SectionName(BodySection);
                    result.warnings.push_back(Warning(ss.str(),
                                                      EmptyDefinitionWarning,
                                                      sourceMap));
                    return;
                }
            }
            
            // Issue the warning
            if (warnEmptyBody) {
                // WARN: empty body
                std::stringstream ss;
                ss << "empty " << SectionName(section) << " " << SectionName(BodySection);
                result.warnings.push_back(Warning(ss.str(),
                                                  EmptyDefinitionWarning,
                                                  sourceMap));
            }
        }

        /**
         *  Checks whether given section payload has duplicate.
         *  \return True when a duplicate is found, false otherwise.
         */
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
