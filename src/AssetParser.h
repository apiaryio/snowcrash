//
//  AssetParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/17/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_ASSETPARSER_H
#define SNOWCRASH_ASSETPARSER_H

#include <sstream>
#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "ListUtility.h"
#include "RegexMatch.h"
#include "StringUtility.h"

// Body matching regex
static const std::string BodyRegex("^[Bb]ody[[:space:]]*$");

// Schema matching regex
static const std::string SchemaRegex("^[Ss]chema[[:space:]]*$");

static const std::string AssetFormattingWarning = " asset is expected to be preformatted code block";

namespace snowcrash {
    
    // Asset signature
    enum AssetSignature {
        UndefinedAssetSignature,
        NoAssetSignature,
        BodyAssetSignature,
        PayloadBodyAssetSignature,
        SchemaAssetSignature,
        GenericAssetSignature
    };
    
    // Query asset signature a of given block
    inline AssetSignature GetAssetSignature(const BlockIterator& begin,
                                            const BlockIterator& end) {
        
        if (begin->type == ListBlockBeginType || begin->type == ListItemBlockBeginType) {
            
            BlockIterator cur = ListItemNameBlock(begin, end);
            if (cur == end)
                return NoAssetSignature;
            
            if (cur->type != ParagraphBlockType &&
                cur->type != ListItemBlockEndType)
                return NoAssetSignature;
            
            std::string content = GetFirstLine(cur->content);
            if (RegexMatch(content, BodyRegex))
                return BodyAssetSignature;

            if (RegexMatch(content, SchemaRegex))
                return SchemaAssetSignature;
            
            if (HasPayloadAssetSignature(begin, end))
                return PayloadBodyAssetSignature;
        }
        
        return NoAssetSignature;
    }
    
    inline bool HasAssetSignature(const BlockIterator& begin,
                                  const BlockIterator& end) {
        AssetSignature signature = GetAssetSignature(begin, end);
        return (signature != NoAssetSignature);
    }
    
    //
    // Block Classifier, Asset Context
    //
    template <>
    inline Section ClassifyBlock<Asset>(const BlockIterator& begin,
                                        const BlockIterator& end,
                                        const Section& context) {
        if (context == UndefinedSection) {
            AssetSignature asset = GetAssetSignature(begin, end);
            if (asset == BodyAssetSignature || asset == PayloadBodyAssetSignature)
                return BodySection;
            if (asset == SchemaAssetSignature)
                return SchemaSection;
        }
        else if (context == BodySection || context == SchemaSection) {
            // Section closure
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSection;
            
            // Adjacent list item
            if (begin->type == ListItemBlockBeginType)
                return UndefinedSection;
        }
        
        return (context == BodySection || context == SchemaSection) ? context : UndefinedSection;
    }
    
    //
    // Asset Section Parser
    //
    template<>
    struct SectionParser<Asset> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               BlueprintParserCore& parser,
                                               Asset& asset) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            switch (section) {
                case BodySection:
                case SchemaSection: 
                    result = HandleAssetSectionBlock(section, cur, bounds, parser, asset);
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
        
        static ParseSectionResult HandleAssetSectionBlock(const Section& section,
                                                          const BlockIterator& cur,
                                                          const SectionBounds& bounds,
                                                          BlueprintParserCore& parser,
                                                          Asset& asset) {

            SourceData data;
            SourceDataBlock sourceMap;
            ParseSectionResult result = ParseListPreformattedBlock(section,
                                                                   cur,
                                                                   bounds,
                                                                   parser,
                                                                   data,
                                                                   sourceMap);
            if (result.first.error.code != Error::OK ||
                parser.sourceData.empty())
                return result;
            
            asset += data;
            return result;
        }

    };
    
    typedef BlockParser<Asset, SectionParser<Asset> > AssetParser;

}

#endif
