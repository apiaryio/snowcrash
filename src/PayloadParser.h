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
#include "AssetParser.h"
#include "HeaderParser.h"
#include "DescriptionSectionUtility.h"
#include "StringUtility.h"
#include "BlockUtility.h"

/** Media type in brackets regex */
#define MEDIA_TYPE "([[:blank:]]*\\(([^\\)]*)\\))"

namespace snowcrashconst {
    
    /** Request matching regex */
    const char* const RequestRegex = "^[[:blank:]]*[Rr]equest" SYMBOL_IDENTIFIER "?" MEDIA_TYPE "?[[:blank:]]*";
    
    /** Response matching regex */
    const char* const ResponseRegex = "^[[:blank:]]*[Rr]esponse([[:blank:][:digit:]]+)?" MEDIA_TYPE "?[[:blank:]]*";
    
    /** Object matching regex */
    const char* const ObjectRegex = "^[[:blank:]]*" SYMBOL_IDENTIFIER "[Oo]bject" MEDIA_TYPE "?[[:blank:]]*$";
    
    /** Model matching regex */
    const char* const  ModelRegex = "^[[:blank:]]*" SYMBOL_IDENTIFIER "?[Mm]odel" MEDIA_TYPE "?[[:blank:]]*";
}

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
            if (RegexCapture(content, snowcrashconst::RequestRegex, captureGroups, 5)) {
                name = captureGroups[1];
                TrimString(name);
                mediaType = captureGroups[3];
                return RequestPayloadSignature;
            }
            else if (RegexCapture(content, snowcrashconst::ResponseRegex, captureGroups, 5)) {
                name = captureGroups[1];
                TrimString(name);
                mediaType = captureGroups[3];
                return ResponsePayloadSignature;
            }
            else if (RegexCapture(content, snowcrashconst::ObjectRegex, captureGroups, 5)) {
                name = captureGroups[1];
                TrimString(name);
                mediaType = captureGroups[3];
                return ObjectPayloadSignature;
            }
            else if (RegexCapture(content, snowcrashconst::ModelRegex, captureGroups, 5)) {
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
    FORCEINLINE SectionType ClassifyInternaListBlock<Payload>(const BlockIterator& begin,
                                                          const BlockIterator& end) {
        
        AssetSignature asset = GetAssetSignature(begin, end);
        if (asset == BodyAssetSignature)
            return BodySectionType;
        else if (asset == SchemaAssetSignature)
            return SchemaSectionType;
        
        if (HasHeaderSignature(begin, end))
            return HeadersSectionType;
        
        return UndefinedSectionType;
    }
    
    /** Children blocks classifier */
    template <>
    FORCEINLINE SectionType ClassifyChildrenListBlock<Payload>(const BlockIterator& begin,
                                                              const BlockIterator& end) {
        
        SectionType type = ClassifyInternaListBlock<Payload>(begin, end);
        if (type != UndefinedSectionType)
            return type;
                
        return UndefinedSectionType;
    }
    
    /**
     *  Block Classifier, payload context.
     */
    template <>
    FORCEINLINE SectionType ClassifyBlock<Payload>(const BlockIterator& begin,
                                                   const BlockIterator& end,
                                                   const SectionType& context) {
        
        if (context == UndefinedSectionType) {
            
            Name name;
            SourceData mediaType;
            PayloadSignature payload = GetPayloadSignature(begin, end, name, mediaType);
            if (payload == RequestPayloadSignature) {

                return (HasNestedListBlock(begin, end)) ? RequestSectionType : RequestBodySectionType;
            }
            else if (payload == ResponsePayloadSignature) {
                
                return (HasNestedListBlock(begin, end)) ? ResponseSectionType : ResponseBodySectionType;
            }
            else if (payload == ObjectPayloadSignature) {
                
                return (HasNestedListBlock(begin, end)) ? ObjectSectionType : ObjectBodySectionType;
            }
            else if (payload == ModelPayloadSignature) {
                
                return (HasNestedListBlock(begin, end)) ? ModelSectionType : ModelBodySectionType;
            }

        }
        else if (context == RequestSectionType ||
                 context == ResponseSectionType ||
                 context == ObjectSectionType ||
                 context == ModelSectionType) {
            
            // SectionType closure
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSectionType;

            SectionType listSection = ClassifyInternaListBlock<Payload>(begin, end);
            if (listSection != UndefinedSectionType)
                return listSection;
            
            // Adjacent list item
            if (begin->type == ListItemBlockBeginType)
                return UndefinedSectionType;
        }
        else if (context == HeadersSectionType ||
                 context == BodySectionType ||
                 context == SchemaSectionType ||
                 context == ForeignSectionType) {

            // SectionType closure
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSectionType;
            
            SectionType listSection = ClassifyInternaListBlock<Payload>(begin, end);
            if (listSection != UndefinedSectionType)
                return listSection;
            
            if (HasPayloadAssetSignature(begin, end))
                return UndefinedSectionType;
            
            return ForeignSectionType;
        }
        
        return (context == RequestSectionType ||
                context == ResponseSectionType ||
                context == ObjectSectionType ||
                context == ModelSectionType) ? context : UndefinedSectionType;
    }
    
    /**
     *  Payload section parser.
     */
    template<>
    struct SectionParser<Payload> {
        
        static ParseSectionResult ParseSection(const BlueprintSection& section,
                                               const BlockIterator& cur,
                                               BlueprintParserCore& parser,
                                               Payload& payload) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);

            switch (section.type) {
                case RequestSectionType:
                case ResponseSectionType:
                case ObjectSectionType:
                case ModelSectionType:
                    result = HandleDescriptionSectionBlock(section, cur, parser, payload);
                    break;
                    
                case RequestBodySectionType:
                case ResponseBodySectionType:
                case ObjectBodySectionType:
                case ModelBodySectionType:
                    result = HandlePayloadAsset(section, cur, parser, payload);
                    break;
                    
                case HeadersSectionType:
                    result = HandleHeaders(section, cur, parser, payload);
                    break;
                    
                case BodySectionType:
                case SchemaSectionType:
                    result = HandleAsset(section, cur, parser, payload);
                    break;
                    
                case UndefinedSectionType:
                    result.second = CloseList(cur, section.bounds.second);
                    break;
                    
                case ForeignSectionType:
                    result = HandleForeignSection<Payload>(section, cur, parser.sourceData);
                    break;
                                        
                default:
                    result.first.error = UnexpectedBlockError(section, cur, parser.sourceData);
                    break;
            }
            
            return result;
        }
        
        static void Finalize(const SectionBounds& bounds,
                             BlueprintParserCore& parser,
                             Payload& payload,
                             Result& result) {}
        
        /**
         *  \brief  Parse Payload's description blocks.
         *  \param  section The current section's signature.
         *  \param  cur     The actual position within Markdown block buffer.
         *  \param  bound   Boundaries of Markdown block buffer.
         *  \param  parser  A parser's instance.
         *  \param  payload An output buffer to write overview description into.
         *  \return A block parser section result.
         */
        static ParseSectionResult HandleDescriptionSectionBlock(const BlueprintSection& section,
                                                                const BlockIterator& cur,
                                                                BlueprintParserCore& parser,
                                                                Payload& payload) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur = cur;

            // Signature
            if (sectionCur == section.bounds.first) {

                ProcessSignature(section, sectionCur, parser.sourceData, result.first, payload);
                sectionCur = FirstContentBlock(cur, section.bounds.second);

                result.second = ++sectionCur;
                return result;
                
            }

            // Description
            result = ParseDescriptionBlock<Payload>(section,
                                                    sectionCur,
                                                    parser.sourceData,
                                                    payload);
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
        static ParseSectionResult HandleAsset(const BlueprintSection& section,
                                              const BlockIterator& cur,
                                              BlueprintParserCore& parser,
                                              Payload& payload) {
            Asset asset;
            ParseSectionResult result = AssetParser::Parse(cur, section.bounds.second, section, parser, asset);
            if (result.first.error.code != Error::OK)
                return result;

            if (!SetAsset(section.type, asset, payload)) {
                // WARN: asset already set
                std::stringstream ss;
                ss << "ignoring additional " << SectionName(section.type) << " content, content is already defined";
                
                BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, parser.sourceData);
                result.first.warnings.push_back(Warning(ss.str(),
                                                        RedefinitionWarning,
                                                        sourceBlock));
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
        static ParseSectionResult HandlePayloadAsset(const BlueprintSection& section,
                                                     const BlockIterator& cur,
                                                     BlueprintParserCore& parser,
                                                     Payload& payload) {
            // Try to parse as a Symbol reference
            SymbolName symbol;
            SourceDataBlock symbolSourceMap;
            ParseSectionResult result = ParseSymbolReference(cur, section.bounds, parser, symbol, symbolSourceMap);
            if (result.first.error.code != Error::OK)
                return result;
            
            if (result.second != cur) {
                // Process a symbol reference
                ResourceModelSymbolTable::const_iterator symbolEntry = parser.symbolTable.resourceModels.find(symbol);
                if (symbolEntry == parser.symbolTable.resourceModels.end()) {
                    
                    // ERR: Undefined symbol
                    std::stringstream ss;
                    ss << "undefined symbol '" << symbol << "'";
                    result.first.error = Error(ss.str(),
                                               SymbolError,
                                               MapSourceDataBlock(symbolSourceMap, parser.sourceData));
                    return result;
                }
                
                // Retrieve payload from symbol table
                payload = symbolEntry->second;
            }
            else {
                // Parse as an asset
                result = HandleAsset(section, cur, parser, payload);
            }
            
            // Retrieve signature
            ProcessSignature(section, cur, parser.sourceData, result.first, payload);
            
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
        static ParseSectionResult ParseSymbolReference(const BlockIterator& cur,
                                                       const SectionBounds& bounds,
                                                       BlueprintParserCore& parser,
                                                       SymbolName& symbolName,
                                                       SourceDataBlock& symbolSourceMap) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur = cur;
            SourceData content;
            SourceData signature = GetListItemSignature(sectionCur, bounds.second, content);
            if (!content.empty()) {
                sectionCur = ListItemNameBlock(sectionCur, bounds.second);
            }
            else {
                sectionCur = FirstContentBlock(cur, bounds.second);
                if (sectionCur == bounds.second ||
                    sectionCur->type != ParagraphBlockType)
                    return result;

                // Try the next block
                if (++sectionCur == bounds.second ||
                    sectionCur->type != ParagraphBlockType)
                    return result;
                
                content = sectionCur->content;
            }
            
            TrimString(content);
            SymbolName symbol;
            if (!GetSymbolReference(content, symbol))
                return result;
            
            symbolName = symbol;
            symbolSourceMap = sectionCur->sourceMap;
            
            // Close list item
            BlockIterator endCur = cur;
            if (endCur->type == ListBlockBeginType)
                ++endCur;
            endCur = SkipToClosingBlock(endCur, bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
            
            // Check extraneous content
            if (sectionCur != endCur) {
                ++sectionCur;
                for (; sectionCur != endCur; ++sectionCur) {

                    if (sectionCur->type == QuoteBlockBeginType)
                        sectionCur = SkipToClosingBlock(sectionCur, endCur, QuoteBlockBeginType, QuoteBlockEndType);
                    
                    if (sectionCur->type == ListBlockBeginType)
                        sectionCur = SkipToClosingBlock(sectionCur, endCur, ListBlockBeginType, ListBlockEndType);
                    
                    // WARN: ignoring extraneous content after symbol reference
                    std::stringstream ss;
                    ss << "ignoring extraneous content after symbol reference";
                    ss << ", expected symbol reference only e.g. '[" << symbolName << "][]'";
                    
                    SourceCharactersBlock sourceBlock = CharacterMapForBlock(sectionCur, cur, bounds, parser.sourceData);
                    result.first.warnings.push_back(Warning(ss.str(),
                                                            IgnoringWarning,
                                                            sourceBlock));
                }
            }
            
            endCur = CloseList(sectionCur, bounds.second);
            result.second = endCur;
            
            return result;
        }
        
        /**
         *  Retrieve and process payload signature.
         */
        static void ProcessSignature(const BlueprintSection& section,
                                     const BlockIterator& cur,
                                     const SourceData& sourceData,
                                     Result& result,
                                     Payload& payload) {
            
            SourceData remainingContent;
            SourceData signature = GetListItemSignature(cur, section.bounds.second, remainingContent);

            // Capture name & payload type
            SourceData mediaType;
            GetPayloadSignature(cur, section.bounds.second, payload.name, mediaType);
            
            // Check signature
            if (!CheckSignature(section, cur, signature, sourceData, result)) {
                // Clear all readouts
                payload.name.clear();
                mediaType.clear();
                remainingContent.clear();
            }
            
            // Add any extra lines to description unless abbreviated body
            if (!remainingContent.empty() &&
                section.type != RequestBodySectionType &&
                section.type != ResponseBodySectionType) {
                payload.description += remainingContent;
            }
            
            // WARN: missing status code
            if (payload.name.empty() &&
                (section.type == ResponseSectionType || section.type == ResponseBodySectionType)) {
                
                BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, sourceData);
                result.warnings.push_back(Warning("missing response HTTP status code, assuming 'Response 200'",
                                                  EmptyDefinitionWarning,
                                                  sourceBlock));
                payload.name = "200";
            }
            
            
            // WARN: Object deprecation
            if (section.type == ObjectSectionType || section.type == ObjectBodySectionType) {

                std::stringstream ss;
                ss << "the 'object' keyword is deprecated and as such it will be removed in a future release, please use the 'model' keyword instead";
                
                BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, sourceData);
                result.warnings.push_back(Warning(ss.str(),
                                                  DeprecatedWarning,
                                                  sourceBlock));
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
        static bool CheckSignature(const BlueprintSection& section,
                                   const BlockIterator& cur,
                                   const SourceData& signature,
                                   const SourceData& sourceData,
                                   Result& result) {
            
            std::string regex;
            switch (section.type) {
                    
                case RequestSectionType:
                case RequestBodySectionType:
                    regex = snowcrashconst::RequestRegex;
                    break;
                    
                case ResponseBodySectionType:
                case ResponseSectionType:
                    regex = snowcrashconst::ResponseRegex;
                    break;
                    
                case ModelSectionType:
                case ModelBodySectionType:
                    regex = snowcrashconst::ModelRegex;
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
                    std::stringstream ss;
                    ss << "unable to parse " << SectionName(section.type) << " signature, expected ";
        
                    switch (section.type) {
                            
                        case RequestSectionType:
                        case RequestBodySectionType:
                            ss << "'request [<identifier>] [(<media type>)]'";
                            break;
                            
                        case ResponseBodySectionType:
                        case ResponseSectionType:
                            ss << "'response [<HTTP status code>] [(<media type>)]'";
                            break;
                            
                        case ModelSectionType:
                        case ModelBodySectionType:
                            ss << "'model [(<media type>)]'";
                            break;
                            
                        default:
                            return false;
                    }
                    
                    BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
                    SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, sourceData);
                    result.warnings.push_back(Warning(ss.str(),
                                                      FormattingWarning,
                                                      sourceBlock));
                    
                    return false;
                }
            }
            
            return true;
        }
        
        /**
         *  \brief  Set payload's asset. 
         *  \return True on success, false when an asset is already set.
         */
        static bool SetAsset(const SectionType& sectionType, const Asset& asset, Payload& payload) {
            
            if (sectionType == BodySectionType ||
                sectionType == RequestBodySectionType ||
                sectionType == ResponseBodySectionType ||
                sectionType == ModelBodySectionType ||
                sectionType == ObjectBodySectionType ||
                sectionType == DanglingBodySectionType) {
                if (!payload.body.empty())
                    return false;

                payload.body = asset;
            }
            else if (sectionType == SchemaSectionType ||
                     sectionType == DanglingSchemaSectionType) {
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
