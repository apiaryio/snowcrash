//
//  PayloadParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/7/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_PARSEPAYLOAD_H
#define SNOWCRASH_PARSEPAYLOAD_H

#include <algorithm>
#include <sstream>
#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "RegexMatch.h"
#include "StringUtility.h"
#include "ListUtility.h"
#include "AssetParser.h"
#include "HeaderParser.h"

// Request matching regex
static const std::string RequestRegex("^[Rr]equest([[:space:]]+([A-Za-z0-9_]|[[:space:]])*)?([[:space:]]\\([^\\)]*\\))?[[:space:]]*$");

// Response matching regex
static const std::string ResponseRegex("^[Rr]esponse([[:space:]]+([0-9_])*)?([[:space:]]\\([^\\)]*\\))?[[:space:]]*$");

namespace snowcrash {
    
    inline Collection<Request>::const_iterator FindRequest(const Method& method, const Request& request) {
        return std::find_if(method.requests.begin(),
                            method.requests.end(),
                            std::bind2nd(MatchName<Request>(), request));
    }
    
    inline Collection<Request>::const_iterator FindResponse(const Method& method, const Response& response) {
        return std::find_if(method.responses.begin(),
                            method.responses.end(),
                            std::bind2nd(MatchName<Response>(), response));
    }
    
    // Payload signature
    enum PayloadSignature {
        UndefinedPayloadSignature,
        NoPayloadSignature,
        RequestPayloadSignature,
        ResponsePayloadSignature,
        GenericPayloadSignature
    };
    
    // Query payload signature a of given block
    inline PayloadSignature GetPayloadSignature(const BlockIterator& begin,
                                                const BlockIterator& end) {
        
        if (begin->type == ListBlockBeginType || begin->type == ListItemBlockBeginType) {
            
            BlockIterator cur = ListItemNameBlock(begin, end);
            if (cur == end)
                return NoPayloadSignature;
            
            if (cur->type != ParagraphBlockType &&
                cur->type != ListItemBlockEndType)
                return NoPayloadSignature;
            
            std::string content = GetFirstLine(cur->content);
            if (RegexMatch(content, RequestRegex))
                return RequestPayloadSignature;
            
            if (RegexMatch(content, ResponseRegex))
                return ResponsePayloadSignature;
        }

        return NoPayloadSignature;
    }
    
    inline bool HasPayloadSignature(const BlockIterator& begin,
                                    const BlockIterator& end) {
        PayloadSignature signature = GetPayloadSignature(begin, end);
        return (signature == RequestPayloadSignature || signature == ResponsePayloadSignature);
    }
    
    //
    // Classifier of internal list items (params, headers, body, schema)
    //
    inline static Section ClassifyInternaListBlock(const BlockIterator& begin,
                                                   const BlockIterator& end) {
        
        AssetSignature asset = GetAssetSignature(begin, end);
        if (asset == BodyAssetSignature)
            return BodySection;
        else if (asset == SchemaAssetSignature)
            return SchemaSection;
        
        if (HasHeaderSignature(begin, end))
            return HeadersSection;
        
        return UndefinedSection;
    }
    
    //
    // Block Classifier, Payload Context
    //
    template <>
    inline Section ClassifyBlock<Payload>(const BlockIterator& begin,
                                          const BlockIterator& end,
                                          const Section& context) {
        
        if (context == UndefinedSection) {
            
            PayloadSignature payload = GetPayloadSignature(begin, end);
            if (payload == RequestPayloadSignature)
                return RequestSection;
            else if (payload == ResponsePayloadSignature)
                return ResponseSection;
        }
        else if ((context == RequestSection || context == ResponseSection)) {
            
            // Section closure
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSection;

            Section listSection = ClassifyInternaListBlock(begin, end);
            if (listSection != UndefinedSection)
                return listSection;
            
            // Adjacent list item
            if (begin->type == ListItemBlockBeginType)
                return UndefinedSection;
        }
        else if (context == HeadersSection ||
                 context == BodySection ||
                 context == SchemaSection ||
                 context == ForeignSection) {

            // Section closure
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSection;
            
            Section listSection = ClassifyInternaListBlock(begin, end);
            if (listSection != UndefinedSection)
                return listSection;
            
            return ForeignSection;
        }
        
        return (context == RequestSection || context == ResponseSection) ? context : UndefinedSection;
    }
    
    //
    // Payload Section Parser
    //
    template<>
    struct SectionParser<Payload> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Payload& payload) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);

            switch (section) {
                case RequestSection:
                case ResponseSection:
                    result = HandleOverviewSectionBlock(section, cur, bounds, sourceData, blueprint, payload);
                    break;
                    
                case HeadersSection:
                    result = HandleHeaders(cur, bounds.second, sourceData, blueprint, payload);
                    break;
                    
                case BodySection:
                    result = HandleBody(cur, bounds.second, sourceData, blueprint, payload);
                    break;
                    
                case SchemaSection:
                    result = HandleSchema(cur, bounds.second, sourceData, blueprint, payload);
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
        
        static ParseSectionResult HandleOverviewSectionBlock(const Section& section,
                                                             const BlockIterator& cur,
                                                             const SectionBounds& bounds,
                                                             const SourceData& sourceData,
                                                             const Blueprint& blueprint,
                                                             Payload& payload) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur = cur;

            if (sectionCur == bounds.first) {
                // Parse payload signature
                sectionCur = ListItemNameBlock(cur, bounds.second);
                if (sectionCur == bounds.second)
                    return std::make_pair(Result(), sectionCur);
                
                // Extract signagure
                ContentParts content = ExtractFirstLine(*sectionCur);
                if (content.empty() ||
                    content.front().empty()) {
                    result.first.error = Error("unable to parse payload signature",
                                               1,
                                               sectionCur->sourceMap);
                    result.second = sectionCur;
                    return result;
                }
                
                // Capture name
                if (section == RequestSection)
                    payload.name = RegexCaptureFirst(content[0], RequestRegex);
                else if (section == ResponseSection)
                    payload.name = RegexCaptureFirst(content[0], ResponseRegex);
                
                // Clean & trim
                if (!payload.name.empty())
                    TrimString(payload.name);
                
                // Add any extra lines to description
                if (content.size() == 2) {
                    payload.description += content[1];
                }
                
                // WARN: missing status code
                if (payload.name.empty() && section == ResponseSection) {
                    result.first.warnings.push_back(Warning("missing response HTTP status code, assuming `Response 200`",
                                                            0,
                                                            sectionCur->sourceMap));
                    payload.name = "200";
                }
                
                sectionCur = FirstContentBlock(cur, bounds.second);
            }
            else {
                // Description
                if (sectionCur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (sectionCur->type == ListBlockBeginType) {
                    sectionCur = SkipToListBlockEndChecking(sectionCur, bounds.second, result.first);
                }
                
                payload.description += MapSourceData(sourceData, sectionCur->sourceMap);
            }
            
            if (sectionCur != bounds.second)
                result.second = ++sectionCur;
            
            return result;
        }
        
        static BlockIterator SkipToListBlockEndChecking(const BlockIterator& begin,
                                                        const BlockIterator& end,
                                                        Result& result) {
            
            BlockIterator cur(begin);
            if (++cur == end)
                return cur;
            
            while (cur != end &&
                   cur->type == ListItemBlockBeginType) {
                
                // Check for internal lists
                Section listSection = ClassifyInternaListBlock(cur, end);
                cur = SkipToSectionEnd(cur, end, ListItemBlockBeginType, ListItemBlockEndType);
                
                if (listSection == HeadersSection ||
                    listSection == BodySection ||
                    listSection == SchemaSection) {
                    
                    // WARN: skipping asset section in description
                    std::stringstream ss;
                    ss << "ignoring " << SectionName(listSection);
                    ss << "in description, description should not end with list";
                    result.warnings.push_back(Warning(ss.str(),
                                                      0,
                                                      (cur != end) ? cur->sourceMap : MakeSourceDataBlock(0,0)));
                }
                
                // TODO: Check parameters

                if (cur != end)
                    ++cur;
            }
            
            return cur;
        }
        
        static ParseSectionResult HandleHeaders(const BlockIterator& begin,
                                                const BlockIterator& end,
                                                const SourceData& sourceData,
                                                const Blueprint& blueprint,
                                                Payload& payload)
        {
            HeaderCollection headers;
            ParseSectionResult result = HeadersParser::Parse(begin, end, sourceData, blueprint, headers);
            if (result.first.error.code != Error::OK)
                return result;
            
            if (headers.empty()) {
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                result.first.warnings.push_back(Warning("no headers specified",
                                                        0,
                                                        nameBlock->sourceMap));
            }
            else {
                // TODO: Check duplicates
                payload.headers.insert(payload.headers.end(), headers.begin(), headers.end());
                
            }
            return result;
        }
        
        static ParseSectionResult HandleBody(const BlockIterator& begin,
                                             const BlockIterator& end,
                                             const SourceData& sourceData,
                                             const Blueprint& blueprint,
                                             Payload& payload)
        {
            Asset body;
            ParseSectionResult result = AssetParser::Parse(begin, end, sourceData, blueprint, body);
            if (result.first.error.code != Error::OK)
                return result;
            
            if (body.empty()) {
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                result.first.warnings.push_back(Warning("empty body asset",
                                                        0,
                                                        nameBlock->sourceMap));
            }
            
            if (!payload.body.empty()) {
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                result.first.warnings.push_back(Warning("ignoring body asset, payload body already defined",
                                                        0,
                                                        nameBlock->sourceMap));
            }
            else {
                payload.body = body;
            }

            return result;
        }
        
        static ParseSectionResult HandleSchema(const BlockIterator& begin,
                                               const BlockIterator& end,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Payload& payload)
        {
            Asset schema;
            ParseSectionResult result = AssetParser::Parse(begin, end, sourceData, blueprint, schema);
            if (result.first.error.code != Error::OK)
                return result;
            
            if (schema.empty()) {
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                result.first.warnings.push_back(Warning("empty schema asset",
                                                        0,
                                                        nameBlock->sourceMap));
            }
            
            if (!payload.schema.empty()) {
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                result.first.warnings.push_back(Warning("ignoring schema asset, payload schema already defined",
                                                        0,
                                                        nameBlock->sourceMap));
            }
            else {
                payload.schema = schema;
            }
            
            return result;
        }
    };
    
    typedef BlockParser<Payload, SectionParser<Payload> > PayloadParser;    
}

#endif
