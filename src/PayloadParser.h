//
//  PayloadParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/7/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
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

/** Media type in brackets regex */
#define MEDIA_TYPE "([[:blank:]]*\\(([^\\)]*)\\))"

/** Request matching regex */
static const std::string RequestRegex("^[[:blank:]]*[Rr]equest" SYMBOL_IDENTIFIER "?" MEDIA_TYPE "?[[:blank:]]*");

/** Response matching regex */
static const std::string ResponseRegex("^[[:blank:]]*[Rr]esponse([[:blank:][:digit:]]+)?" MEDIA_TYPE "?[[:blank:]]*");

/** Object matching regex */
static const std::string ObjectRegex("^[ \\t]*(" SYMBOL_IDENTIFIER ")[ \\t][Oo]bject([ \\t]\\(([^\\)]*)\\))?[ \\t]*$");

/** Model matching regex */
static const std::string ModelRegex("^[[:blank:]]*" SYMBOL_IDENTIFIER "?[Mm]odel" MEDIA_TYPE "?[[:blank:]]*");

namespace snowcrash {
    
    /** 
     *  Payload signature 
     */
    enum PayloadSignature {
        UndefinedPayloadSignature,  /// < Undefined payload.
        NoPayloadSignature,         /// < Not a payload.
        RequestPayloadSignature,    /// < Request payload.
        ResponsePayloadSignature,   /// < Response payload.
        ObjectPayloadSignature,     /// < Resource object payload.
        ModelPayloadSignature       /// < Resource Model payload.
    };
    
    /** 
     *  \brief  Query the payload signature of a given block.
     *  \param  begin   The begin of the block to be queried.
     *  \param  end     The end of the markdown block buffer.
     *  \param  name    A buffer to retrieve payload name into.
     *  \param  mediaType   A buffer to retrieve payload media type into.
     *  \return The %PayloadSignature of the given block.
     */
    FORCEINLINE PayloadSignature GetPayloadSignature(const BlockIterator& begin,
                                                     const BlockIterator& end,
                                                     Name& name,
                                                     SourceData& mediaType) {
        
        if (begin->type == ListBlockBeginType || begin->type == ListItemBlockBeginType) {
            
            BlockIterator cur = ListItemNameBlock(begin, end);
            if (cur == end)
                return NoPayloadSignature;
            
            if (cur->type != ParagraphBlockType &&
                cur->type != ListItemBlockEndType)
                return NoPayloadSignature;
            
            std::string content = GetFirstLine(cur->content);
            
            CaptureGroups captureGroups;
            if (RegexCapture(content, RequestRegex, captureGroups, 5)) {
                name = captureGroups[1];
                TrimString(name);
                mediaType = captureGroups[3];
                return RequestPayloadSignature;
            }
            else if (RegexCapture(content, ResponseRegex, captureGroups, 5)) {
                name = captureGroups[1];
                TrimString(name);
                mediaType = captureGroups[3];
                return ResponsePayloadSignature;
            }
            else if (RegexCapture(content, ObjectRegex, captureGroups, 5)) {
                name = captureGroups[1];
                TrimString(name);
                mediaType = captureGroups[4];
                return ObjectPayloadSignature;
            }
            else if (RegexCapture(content, ModelRegex, captureGroups, 5)) {
                name = captureGroups[1];
                TrimString(name);
                mediaType = captureGroups[3];
                return ModelPayloadSignature;
            }
        }

        return NoPayloadSignature;
    }
    
    /**
     *  Returns true if given block has any payload signature, false otherwise.
     */
    FORCEINLINE bool HasPayloadSignature(const BlockIterator& begin,
                                         const BlockIterator& end) {
        Name name;
        SourceData mediaType;
        PayloadSignature signature = GetPayloadSignature(begin, end, name, mediaType);
        return signature != NoPayloadSignature;
    }
    
    /**
     *  Retruns true if given block has any payload signature and 
     *  is written in the abbreviated form. False otherwise.
     */
    FORCEINLINE bool HasPayloadAssetSignature(const BlockIterator& begin,
                                              const BlockIterator& end) {
        if (!HasPayloadSignature(begin, end))
            return false;
        
        return !HasNestedListBlock(begin, end);
    }
    
    /**
     *  Classifier of internal list items, payload context.
     */
    template <>
    FORCEINLINE Section ClassifyInternaListBlock<Payload>(const BlockIterator& begin,
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
    
    /**
     *  Block Classifier, payload context.
     */
    template <>
    FORCEINLINE Section ClassifyBlock<Payload>(const BlockIterator& begin,
                                               const BlockIterator& end,
                                               const Section& context) {
        
        if (context == UndefinedSection) {
            
            Name name;
            SourceData mediaType;
            PayloadSignature payload = GetPayloadSignature(begin, end, name, mediaType);
            if (payload == RequestPayloadSignature) {

                return (HasNestedListBlock(begin, end)) ? RequestSection : RequestBodySection;
            }
            else if (payload == ResponsePayloadSignature) {
                
                return (HasNestedListBlock(begin, end)) ? ResponseSection : ResponseBodySection;
            }
            else if (payload == ObjectPayloadSignature) {
                
                return (HasNestedListBlock(begin, end)) ? ObjectSection : ObjectBodySection;
            }
            else if (payload == ModelPayloadSignature) {
                
                return (HasNestedListBlock(begin, end)) ? ModelSection : ModelBodySection;
            }

        }
        else if (context == RequestSection ||
                 context == ResponseSection ||
                 context == ObjectSection ||
                 context == ModelSection) {
            
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
            
            if (HasPayloadAssetSignature(begin, end))
                return UndefinedSection;
            
            return ForeignSection;
        }
        
        return (context == RequestSection ||
                context == ResponseSection ||
                context == ObjectSection ||
                context == ModelSection) ? context : UndefinedSection;
    }
    
    /**
     *  Payload section parser.
     */
    template<>
    struct SectionParser<Payload> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               BlueprintParserCore& parser,
                                               Payload& payload) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);

            switch (section) {
                case RequestSection:
                case ResponseSection:
                case ObjectSection:
                case ModelSection:
                    result = HandleOverviewSectionBlock(section, cur, bounds, parser, payload);
                    break;
                    
                case RequestBodySection:
                case ResponseBodySection:
                case ObjectBodySection:
                case ModelBodySection:
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
                    result.first.error = UnexpectedBlockError(*cur);
                    break;
            }
            
            return result;
        }
        
        /**
         *  \brief  Parse Payload's overview blocks.
         *  \param  section The current section's signature.
         *  \param  cur     The actual position within Markdown block buffer.
         *  \param  bound   Boundaries of Markdown block buffer.
         *  \param  parser  A parser's instance.
         *  \param  payload An output buffer to write overview description into.
         *  \return A block parser section result.
         */
        static ParseSectionResult HandleOverviewSectionBlock(const Section& section,
                                                             const BlockIterator& cur,
                                                             const SectionBounds& bounds,
                                                             BlueprintParserCore& parser,
                                                             Payload& payload) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur = cur;

            if (sectionCur == bounds.first) {
                // Signature
                ProcessSignature(section, sectionCur, bounds.second, parser.sourceData, result.first, payload);
                sectionCur = FirstContentBlock(cur, bounds.second);
            }
            else {
                // Description
                if (sectionCur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (sectionCur->type == ListBlockBeginType) {

                    SourceDataBlock descriptionMap;
                    sectionCur = SkipToDescriptionListEnd<Payload>(sectionCur, bounds.second, descriptionMap);
                    
                    if (sectionCur->type != ListBlockEndType) {
                        // Found recognized section in description
                        if (!descriptionMap.empty())
                            payload.description += MapSourceData(parser.sourceData, descriptionMap);

                        result.second = sectionCur;
                        return result;
                    }
                }
                
                if (!CheckCursor(sectionCur, bounds, cur, result.first))
                    return result;
                payload.description += MapSourceData(parser.sourceData, sectionCur->sourceMap);
            }
            
            if (sectionCur != bounds.second)
                result.second = ++sectionCur;
            
            return result;
        }
        
        /**
         *  \brief  Parse an asset.
         *  \param  section The current section's signature.
         *  \param  begin   The begin of the block to be parsed.
         *  \param  end     The end of the markdown block buffer.
         *  \param  parser  A parser's instance.
         *  \param  payload An output buffer to save the parsed asset to.
         *  \return A block parser section result.
         */
        static ParseSectionResult HandleAsset(const Section& section,
                                              const BlockIterator& begin,
                                              const BlockIterator& end,
                                              BlueprintParserCore& parser,
                                              Payload& payload) {
            Asset asset;
            ParseSectionResult result = AssetParser::Parse(begin, end, parser, asset);
            if (result.first.error.code != Error::OK)
                return result;

            if (!SetAsset(section, asset, payload)) {
                // WARN: asset already set
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                std::stringstream ss;
                ss << "ignoring additional " << SectionName(section) << " content, content is already defined";
                result.first.warnings.push_back(Warning(ss.str(),
                                                        RedefinitionWarning,
                                                        nameBlock->sourceMap));
            }
            
            return result;
        }
        
        /**
         *  \brief  Parse payload and abbreviated asset.
         *  \param  section The current section's signature.
         *  \param  begin   The parsed of the block to be queried.
         *  \param  end     The end of the markdown block buffer.
         *  \param  parser  A parser's instance.
         *  \param  payload An output buffer to save the parsed paylod and asset into.
         *  \return A block parser section result.
         */
        static ParseSectionResult HandlePayloadAsset(const Section& section,
                                                     const BlockIterator& begin,
                                                     const BlockIterator& end,
                                                     BlueprintParserCore& parser,
                                                     Payload& payload) {
            // Try to parse as a Symbol reference
            SymbolName symbol;
            SourceDataBlock symbolSourceMap;
            ParseSectionResult result = ParseSymbolReference(begin, end, parser, symbol, symbolSourceMap);
            if (result.first.error.code != Error::OK)
                return result;
            
            if (result.second != begin) {
                // Process a symbol reference
                ResourceModelSymbolTable::const_iterator symbolEntry = parser.symbolTable.resourceModels.find(symbol);
                if (symbolEntry == parser.symbolTable.resourceModels.end()) {
                    
                    // ERR: Undefined symbol
                    std::stringstream ss;
                    ss << "undefined symbol '" << symbol << "'";
                    result.first.error = Error(ss.str(),
                                               SymbolError,
                                               symbolSourceMap);
                    return result;
                }
                
                // Retrieve payload from symbol table
                payload = symbolEntry->second;
            }
            else {
                // Parse as an asset
                result = HandleAsset(BodySection, begin, end, parser, payload);
            }
            
            // Retrieve signature
            ProcessSignature(section, begin, end, parser.sourceData, result.first, payload);
            
            return result;
        }
        
        /**
         *  \brief  Parse a symbol reference.
         *  \param  begin   The begin of the block to be parsed.
         *  \param  end     The end of the markdown block buffer.
         *  \param  parser  A parser's instance.
         *  \param  symbolName  Output buffer to put parsed symbol's name into.
         *  \param  symbolSourceMap Source map of the parsed symbol reference.
         *  \return A block parser section result.
         */
        static ParseSectionResult ParseSymbolReference(const BlockIterator& begin,
                                                       const BlockIterator& end,
                                                       BlueprintParserCore& parser,
                                                       SymbolName& symbolName,
                                                       SourceDataBlock& symbolSourceMap) {
            
            ParseSectionResult result = std::make_pair(Result(), begin);
            BlockIterator cur = begin;
            SourceData content;
            SourceData signature = GetListItemSignature(cur, end, content);
            if (!content.empty()) {
                cur = ListItemNameBlock(cur, end);
            }
            else {
                cur = FirstContentBlock(begin, end);
                if (cur == end ||
                    cur->type != ParagraphBlockType)
                    return result;

                // Try the next block
                if (++cur == end ||
                    cur->type != ParagraphBlockType)
                    return result;
                
                content = cur->content;
            }
            
            TrimString(content);
            SymbolName symbol;
            if (!GetSymbolReference(content, symbol))
                return result;
            
            symbolName = symbol;
            symbolSourceMap = cur->sourceMap;
            
            // Close list item
            BlockIterator endCur = begin;
            if (endCur->type == ListBlockBeginType)
                ++endCur;
            endCur = SkipToSectionEnd(endCur, end, ListItemBlockBeginType, ListItemBlockEndType);
            
            // Check extraneous content
            if (cur != endCur) {
                ++cur;
                for (; cur != endCur; ++cur) {

                    if (cur->type == QuoteBlockBeginType)
                        cur = SkipToSectionEnd(cur, endCur, QuoteBlockBeginType, QuoteBlockEndType);
                    
                    if (cur->type == ListBlockBeginType)
                        cur = SkipToSectionEnd(cur, endCur, ListBlockBeginType, ListBlockEndType);
                    
                    // WARN: ignoring extraneous content after symbol reference
                    std::stringstream ss;
                    ss << "ignoring extraneous content after symbol reference";
                    ss << ", expected symbol reference only e.g. '[" << symbolName << "][]'";
                    result.first.warnings.push_back(Warning(ss.str(), IgnoringWarning, cur->sourceMap));
                }
            }
            
            endCur = CloseListItemBlock(cur, end);
            result.second = endCur;
            
            return result;
        }
        
        /**
         *  Retrieve and process payload signature.
         */
        static void ProcessSignature(const Section& section,
                                     const BlockIterator& begin,
                                     const BlockIterator& end,
                                     const SourceData& sourceData,
                                     Result& result,
                                     Payload& payload) {
            
            SourceData remainingContent;
            SourceData signature = GetListItemSignature(begin, end, remainingContent);

            // Capture name & payload type
            SourceData mediaType;
            GetPayloadSignature(begin, end, payload.name, mediaType);
            
            // Check signature
            if (!CheckSignature(section, begin, end, signature, result)) {
                // Clear and readouts
                payload.name.clear();
                mediaType.clear();
                remainingContent.clear();
            }
            
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
                result.warnings.push_back(Warning("missing response HTTP status code, assuming 'Response 200'",
                                                  EmptyDefinitionWarning,
                                                  nameBlock->sourceMap));
                payload.name = "200";
            }
            
            
            // WARN: Object deprecation
            if (section == ObjectSection || section == ObjectBodySection) {
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                std::stringstream ss;
                ss << "the 'object' keyword is deprecated and as such it will be removed in a future release, please use the 'model' keyword instead";
                result.warnings.push_back(Warning(ss.str(),
                                                  DeprecatedWarning,
                                                  nameBlock->sourceMap));
            }
            
            if (!mediaType.empty()) {
                Header header = std::make_pair(HTTPHeaderName::ContentType, mediaType);
                TrimString(header.second);
                payload.headers.push_back(header);
            }
        }
        
        /** 
         *  \brief Checks and report invalid signature 
         *  \return True if signature is correct, false otherwise.
         */
        static bool CheckSignature(const Section& section,
                                   const BlockIterator& begin,
                                   const BlockIterator& end,
                                   const SourceData& signature,
                                   Result& result) {
            
            std::string regex;
            switch (section) {
                    
                case RequestSection:
                case RequestBodySection:
                    regex = RequestRegex;
                    break;
                    
                case ResponseBodySection:
                case ResponseSection:
                    regex = ResponseRegex;
                    break;
                    
                case ModelSection:
                case ModelBodySection:
                    regex = ModelRegex;
                    break;
                    
                default:
                    return true;
            }
            
            CaptureGroups captureGroups;
            if (RegexCapture(signature, regex, captureGroups) &&
                !captureGroups.empty()) {

                std::string target = signature;
                std::string::size_type pos = target.find(captureGroups[0]);
                if (pos != std::string::npos)
                    target.replace(pos, captureGroups[0].length(), std::string());

                TrimString(target);
                if (!target.empty()) {
                    // WARN: unable to parse payload signature
                    BlockIterator nameBlock = ListItemNameBlock(begin, end);
                    std::stringstream ss;
                    ss << "unable to parse " << SectionName(section) << " signature, expected ";
        
                    switch (section) {
                            
                        case RequestSection:
                        case RequestBodySection:
                            ss << "'request [<identifier>] [(<media type>)]'";
                            break;
                            
                        case ResponseBodySection:
                        case ResponseSection:
                            ss << "'response [<HTTP status code>] [(<media type>)]'";
                            break;
                            
                        case ModelSection:
                        case ModelBodySection:
                            ss << "'model [(<media type>)]'";
                            break;
                            
                        default:
                            return false;
                    }
                    result.warnings.push_back(Warning(ss.str(),
                                                      FormattingWarning,
                                                      nameBlock->sourceMap));
                    
                    return false;
                }
            }
            
            return true;
        }
        
        /**
         *  \brief  Set payload's asset. 
         *  \return True on success, false when an asset is already set.
         */
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
    
    /** Payload Parser */
    typedef BlockParser<Payload, SectionParser<Payload> > PayloadParser;    
}

#endif
