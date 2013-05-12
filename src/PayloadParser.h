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
#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "RegexMatch.h"
#include "TrimString.h"
#include "ListUtility.h"

// Request matching regex
static const std::string RequestRegex("^[Rr]equest([[:space:]]+([A-Za-z0-9_]|[[:space:]])*)?([[:space:]]\\([^\\)]*\\))?[[:space:]]*$");

// Response matching regex
static const std::string ResponseRegex("^[Rr]esponse([[:space:]]+([0-9_])*)?([[:space:]]\\([^\\)]*\\))?[[:space:]]*$");

// Body matching regex
static const std::string BodyRegex("^[Bb]ody[[:space:]]*$");

namespace snowcrash {
    
    // Payload signature
    enum PayloadSignature {
        UndefinedPayloadSignature,
        NoPayloadSignature,
        RequestPayloadSignature,
        ResponsePayloadSignature,
        GenericPayloadSignature
    };
    
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
    
    // Query payload signature a of given block
    inline PayloadSignature HasPayloadSignature(const BlockIterator& begin,
                                                const BlockIterator& end) {
        
        if (begin->type == ListBlockBeginType || begin->type == ListItemBlockBeginType) {
            
            BlockIterator cur = FirstContentBlock(begin, end);
            if (cur == end)
                return NoPayloadSignature;
            
            if (cur->type != ParagraphBlockType &&
                cur->type != ListItemBlockEndType)
                return NoPayloadSignature;
            
            if (RegexMatch(cur->content, RequestRegex))
                return RequestPayloadSignature;
            
            if (RegexMatch(cur->content, ResponseRegex))
                return ResponsePayloadSignature;
        }

        return NoPayloadSignature;
    }
    
    // Query payload's body signature
    inline bool HasBodySignature(const BlockIterator& begin,
                                 const BlockIterator& end) {
        
        if (begin->type == ListBlockBeginType || begin->type == ListItemBlockBeginType) {
            
            BlockIterator cur = FirstContentBlock(begin, end);
            if (cur == end)
                return false;
            
            if (cur->type != ParagraphBlockType)
                return false;
            
            return RegexMatch(cur->content, BodyRegex);
        }

        return false;
    }
    
    //
    // Block Classifier, Payload Context
    //
    template <>
    inline Section TClassifyBlock<Payload>(const BlockIterator& begin,
                                           const BlockIterator& end,
                                           const Section& context) {
        
        // Leading list
        if (context == UndefinedSection &&
            (begin->type == ListBlockBeginType || begin->type == ListItemBlockBeginType)) {
            
            PayloadSignature payload = HasPayloadSignature(begin, end);
            if (payload == RequestPayloadSignature)
                return RequestSection;
            else if (payload == ResponsePayloadSignature)
                return ResponseSection;
        }

        // Internal lists (params, headers, body, schema)
        if ((context == RequestSection || context == ResponseSection) &&
            (begin->type == ListBlockBeginType || begin->type == ListItemBlockBeginType)) {
            
            if (HasBodySignature(begin, end))
                return BodySection;
        }
        
        return (context == RequestSection || context == ResponseSection) ? context : UndefinedSection;
    }
    
    //
    // Payload Overview Parser
    //
    template<>
    struct SectionOverviewParser<Payload>  {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Payload& payload) {

            if (section != RequestSection && section != ResponseSection)
                return std::make_pair(Result(), cur);
            
            Result result;
            BlockIterator sectionCur(cur);

            // Cleanup previous list item
            if (cur->type == ListBlockEndType ||
                cur->type == ListItemBlockEndType)
                return SkipAfterListBlockEnd(cur, bounds.second);
            
            // Signature
            if (sectionCur == bounds.first) {
                
                sectionCur = FirstContentBlock(sectionCur, bounds.second);
                if (sectionCur == bounds.second)
                    return std::make_pair(Result(), sectionCur);
                
                ContentParts content = ExtractFirstLine(*sectionCur);
                
                if (section == RequestSection) {
                    payload.name = RegexCaptureFirst(content[0], RequestRegex);
                }
                else if (section == ResponseSection) {
                    payload.name = RegexCaptureFirst(content[0], ResponseRegex);
                }
                
                if (!payload.name.empty())
                    TrimString(payload.name);
                
                if (content.size() == 2) {
                    payload.description += content[1]; // MapSourceData(sourceData, content[1].sourceMap);
                    TrimString(payload.description);
                }
            }
            else {
                
                // TODO: handle list / quotes
                payload.description += MapSourceData(sourceData, sectionCur->sourceMap);
            }
            
            if (payload.name.empty() && section == ResponseSection) {
                // ERR: missing status code
                result.error = Error("missing response status code", 1, sectionCur->sourceMap);
            }
            
            return std::make_pair(result, ++sectionCur);
        }
    };
    
    typedef BlockParser<Payload, SectionOverviewParser<Payload> > PayloadOverviewParser;    
    
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

            // Cleanup previous list item
            if (cur->type == ListBlockEndType ||
                cur->type == ListItemBlockEndType)
                return SkipAfterListBlockEnd(cur, bounds.second);
            
            switch (section) {
                case RequestSection:
                case ResponseSection:
                    
                    result = PayloadOverviewParser::Parse(cur, bounds.second, sourceData, blueprint, payload);
                    break;
                    
                case BodySection:
                    result = ParseBody(cur, bounds.second, sourceData, blueprint, payload);
                    break;
                    
                case UndefinedSection:
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, cur->sourceMap);
                    break;
            }
            
            return result;
        }
        
        // Parse body asset
        static ParseSectionResult ParseBody(const BlockIterator& begin,
                                            const BlockIterator& end,
                                            const SourceData& sourceData,
                                            const Blueprint& blueprint,
                                            Payload& payload)
        {
            ParseSectionResult result = std::make_pair(Result(), begin);
            BlockIterator sectionCur = FirstContentBlock(begin, end);
            
            // Skip body signature
            if (++sectionCur == end) {
                result.second = end;
                return result;
            }
            
            // Retrieve asset
            if (sectionCur->type != CodeBlockType || sectionCur->content.empty())
                result.first.warnings.push_back(Warning("expected payload's body asset", 0, sectionCur->sourceMap));
            else
                payload.body = sectionCur->content;
            
            // Forward to end of this section
            result.second = SkipToListBlockEnd(++sectionCur, end, result.first);
            ++result.second;
            return result;
        }
        
    };
    
    typedef BlockParser<Payload, SectionParser<Payload> > PayloadParser;    
}

#endif
