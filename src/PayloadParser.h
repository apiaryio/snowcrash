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
static const std::string RequestRegex("^[Rr]equest([[:space:]]+([A-Za-z0-9_]|[[:space:]])*)?([[:space:]]\\(([^\\)]*)\\))?[[:space:]]*$");

// Response matching regex
static const std::string ResponseRegex("^[Rr]esponse([[:space:]]+([0-9_])*)?([[:space:]]\\(([^\\)]*)\\))?[[:space:]]*$");

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
    
    inline bool HasPayloadAssetSignature(const BlockIterator& begin, const BlockIterator& end) {
        if (!HasPayloadSignature(begin, end))
            return false;
        
        return !HasNestedListBlock(begin, end);
    }
    
    //
    // Classifier of internal list items, Payload context
    //
    template <>
    inline Section ClassifyInternaListBlock<Payload>(const BlockIterator& begin,
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
            if (payload == RequestPayloadSignature) {

                return (HasNestedListBlock(begin, end)) ? RequestSection : RequestBodySection;
            }
            else if (payload == ResponsePayloadSignature) {
                
                return (HasNestedListBlock(begin, end)) ? ResponseSection : ResponseBodySection;
            }
        }
        else if ((context == RequestSection || context == ResponseSection)) {
            
            // Section closure
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSection;

            Section listSection = ClassifyInternaListBlock<Payload>(begin, end);
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
            
            Section listSection = ClassifyInternaListBlock<Payload>(begin, end);
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
                                               const ParserCore& parser,
                                               Payload& payload) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);

            switch (section) {
                case RequestSection:
                case ResponseSection:
                    result = HandleOverviewSectionBlock(section, cur, bounds, parser, payload);
                    break;
                    
                case RequestBodySection:
                case ResponseBodySection:
                    result = HandlePayloadAsset(section, cur, bounds.second, parser, payload);
                    break;
                    
                case HeadersSection:
                    result = HandleHeaders(cur, bounds.second, parser, payload);
                    break;
                    
                case BodySection:
                case SchemaSection:
                    result = HandleAsset(section, cur, bounds.second, parser, payload);
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
        
        static ParseSectionResult HandleOverviewSectionBlock(const Section& section,
                                                             const BlockIterator& cur,
                                                             const SectionBounds& bounds,
                                                             const ParserCore& parser,
                                                             Payload& payload) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur = cur;

            if (sectionCur == bounds.first) {
                // Signature
                ProcessSignature(section, sectionCur, bounds.first, parser.sourceData, result.first, payload);
                sectionCur = FirstContentBlock(cur, bounds.second);
            }
            else {
                // Description
                if (sectionCur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (sectionCur->type == ListBlockBeginType) {
                    sectionCur = SkipToDescriptionListEnd<Payload>(sectionCur, bounds.second, result.first);
                }
                
                payload.description += MapSourceData(parser.sourceData, sectionCur->sourceMap);
            }
            
            if (sectionCur != bounds.second)
                result.second = ++sectionCur;
            
            return result;
        }
        
        static ParseSectionResult HandleAsset(const Section& section,
                                              const BlockIterator& begin,
                                              const BlockIterator& end,
                                              const ParserCore& parser,
                                              Payload& payload) {
            Asset asset;
            ParseSectionResult result = AssetParser::Parse(begin, end, parser, asset);
            if (result.first.error.code != Error::OK)
                return result;
            
            if (asset.empty()) {
                // WARN: empty asset
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                std::stringstream ss;
                ss << "empty " << SectionName(section) << " asset";
                result.first.warnings.push_back(Warning(ss.str(),
                                                        0,
                                                        nameBlock->sourceMap));
            }
            
            
            if (!SetAsset(section, asset, payload)) {
                // WARN: asset already set
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                std::stringstream ss;
                ss << "ignoring " << SectionName(section) << " asset, asset already defined";
                result.first.warnings.push_back(Warning(ss.str(),
                                                        0,
                                                        nameBlock->sourceMap));
            }
            
            return result;
        }
        
        // Handle payload + body asset abbreviation
        static ParseSectionResult HandlePayloadAsset(const Section& section,
                                                     const BlockIterator& begin,
                                                     const BlockIterator& end,
                                                     const ParserCore& parser,
                                                     Payload& payload) {
        
            // Parse as an asset
            ParseSectionResult result = HandleAsset(BodySection, begin, end, parser, payload);
            
            // Retrieve signature
            ProcessSignature(section, begin, end, parser.sourceData, result.first, payload);
            
            return result;
        }
        
        // Retrieves & process payload name - signature
        static void ProcessSignature(const Section& section,
                                     const BlockIterator& begin,
                                     const BlockIterator& end,
                                     const SourceData& sourceData,
                                     Result& result,
                                     Payload& payload) {
            
            SourceData remainingContent;
            SourceData signature = GetListItemSignature(begin, end, remainingContent);

            // Capture name
            CaptureGroups groups;
            if (section == RequestSection || section == RequestBodySection)
                RegexCapture(signature, RequestRegex, groups, 5);
            else if (section == ResponseSection || section == ResponseBodySection)
                RegexCapture(signature, ResponseRegex, groups, 5);
            
            payload.name = groups[1];
            
            // Clean & trim
            if (!payload.name.empty())
                TrimString(payload.name);
            
            // Add any extra lines to description unless abbreviated body
            if (!remainingContent.empty() &&
                section != RequestBodySection &&
                section != ResponseBodySection) {
                payload.description += remainingContent;
            }
            
            // WARN: missing status code
            if (payload.name.empty() &&
                (section == ResponseSection || section == ResponseBodySection)) {
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                result.warnings.push_back(Warning("missing response HTTP status code, assuming `Response 200`",
                                                  0,
                                                  nameBlock->sourceMap));
                payload.name = "200";
            }
            
            if (!groups[4].empty()) {
                Header header = std::make_pair("Content-Type", groups[4]);
                TrimString(header.second);
                payload.headers.push_back(header);
            }
        }
        
        // Sets payload section asset. Returns true on success, false when asset is already set.
        static bool SetAsset(const Section& section, const Asset& asset, Payload& payload) {
            
            if (section == BodySection) {
                if (!payload.body.empty())
                    return false;

                payload.body = asset;
            }
            else if (section == SchemaSection) {
                if (!payload.schema.empty())
                    return false;
                
                payload.schema = asset;
            }
            
            return true;
        }
    };
    
    typedef BlockParser<Payload, SectionParser<Payload> > PayloadParser;    
}

#endif
