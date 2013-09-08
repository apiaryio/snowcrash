//
//  AssetParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/17/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
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
static const std::string BodyRegex("^[ \\t]*[Bb]ody[ \\t]*$");

// Schema matching regex
static const std::string SchemaRegex("^[ \\t]*[Ss]chema[ \\t]*$");

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
    FORCEINLINE AssetSignature GetAssetSignature(const BlockIterator& begin,
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
    
    FORCEINLINE bool HasAssetSignature(const BlockIterator& begin,
                                       const BlockIterator& end) {
        AssetSignature signature = GetAssetSignature(begin, end);
        return (signature != NoAssetSignature);
    }
    
    /**
     *  Return true if given block might be a dangling asset block,
     *  false otherwise.
     */
    FORCEINLINE bool IsDanglingBlock(const BlockIterator& begin, const BlockIterator& end) {
        return (begin->type == ParagraphBlockType
                || begin->type == CodeBlockType);
    }
    
    /**
     *  Block Classifier, asset context.
     */
    template <>
    FORCEINLINE Section ClassifyBlock<Asset>(const BlockIterator& begin,
                                             const BlockIterator& end,
                                             const Section& context) {
        
        if (context == UndefinedSection) {
            AssetSignature asset = GetAssetSignature(begin, end);
            if (asset == BodyAssetSignature || asset == PayloadBodyAssetSignature)
                return BodySection;
            if (asset == SchemaAssetSignature)
                return SchemaSection;
        }
        else if (context == BodySection ||
                 context == SchemaSection) {
            
            // Section closure
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType) {

                // Look ahead for a dangling asset
                BlockIterator cur = CloseList(begin, end);
                if (cur == end)
                    return UndefinedSection;
                
                if (IsDanglingBlock(cur, end))
                    return (context == BodySection) ? DanglingBodySection : DanglingSchemaSection;
                else
                    return UndefinedSection;
            }
            
            // Adjacent list item
            if (begin->type == ListItemBlockBeginType) {
                    return UndefinedSection;
            }
        }
        else if (context == DanglingBodySection ||
                 context == DanglingSchemaSection) {
            
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSection;
            
            if (!IsDanglingBlock(begin, end))
                return UndefinedSection;
        }
        
        return (context == BodySection ||
                context == SchemaSection ||
                context == DanglingBodySection ||
                context == DanglingSchemaSection) ? context : UndefinedSection;
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
                    
                case DanglingBodySection:
                case DanglingSchemaSection:
                    result = HandleDanglingAssetSectionBlock(section, cur, bounds, parser, asset);
                    break;
                    
                case UndefinedSection:
                    result.second = CloseListItemBlock(cur, bounds.second);
                    break;
                    
                default:
                    result.first.error = UnexpectedBlockError(*cur);
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
        
        static ParseSectionResult HandleDanglingAssetSectionBlock(const Section& section,
                                                                  const BlockIterator& cur,
                                                                  const SectionBounds& bounds,
                                                                  BlueprintParserCore& parser,
                                                                  Asset& asset) {
            
            // Skip any closing list blocks
            BlockIterator sectionCur = CloseList(cur, bounds.second);
            
            Section originalSection = (section == DanglingSchemaSection) ? SchemaSection : BodySection;
            SourceData data;
            SourceDataBlock sourceMap;
            ParseSectionResult result = ParseListPreformattedBlock(originalSection,
                                                                   sectionCur,
                                                                   bounds,
                                                                   parser,
                                                                   data,
                                                                   sourceMap);
            if (result.first.error.code != Error::OK ||
                data.empty())
                return result;
            
            asset += data;
            
            // WARN: Dangling block
            std::stringstream ss;
            ss << "dangling " << SectionName(originalSection);
            ss << ", increase its indentation to nest it properly";
            result.first.warnings.push_back(Warning(ss.str(),
                                                    FormattingWarning,
                                                    sourceMap));
            return result;
        }

    };
    
    typedef BlockParser<Asset, SectionParser<Asset> > AssetParser;

}

#endif
