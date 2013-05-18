//
//  AssetParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/17/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_ASSETPARSER_H
#define SNOWCRASH_ASSETPARSER_H

#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "ListUtility.h"
#include "RegexMatch.h"
#include "TrimString.h"

// Body matching regex
static const std::string BodyRegex("^[Bb]ody[[:space:]]*$");

namespace snowcrash {
    
    // Asset signature
    enum AssetSignature {
        UndefinedAssetSignature,
        NoAssetSignature,
        BodyAssetSignature,
        SchemaAssetSignature,
        GenericAssetSignature
    };
    
    // Query asset signature a of given block
    inline AssetSignature GetAssetSignature(const BlockIterator& begin,
                                            const BlockIterator& end) {
        
        if (begin->type == ListBlockBeginType || begin->type == ListItemBlockBeginType) {
            
            BlockIterator cur = FirstContentBlock(begin, end);
            if (cur == end)
                return NoAssetSignature;
            
            if (cur->type != ParagraphBlockType &&
                cur->type != ListItemBlockEndType)
                return NoAssetSignature;
            
            std::string content = GetFirstLine(cur->content);
            if (RegexMatch(content, BodyRegex))
                return BodyAssetSignature;

        }
        
        return NoAssetSignature;
    }
    
    inline bool HasAssetSignature(const BlockIterator& begin,
                                  const BlockIterator& end) {
        AssetSignature signature = GetAssetSignature(begin, end);
        return (signature == BodyAssetSignature);
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
            if (asset == BodyAssetSignature)
                return BodySection;
        }
        else if (context == BodySection) {
            
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSection;    // closing
            
            // Alien list item
            if (begin->type == ListItemBlockBeginType)
                return UndefinedSection;
        }
        
        return (context == BodySection) ? context : UndefinedSection;
    }
    
    //
    // Asset Section Parser
    //
    template<>
    struct SectionParser<Asset> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Asset& asset) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            switch (section) {
                case BodySection:
                    result = HandleBodySectionBlock(cur, bounds, sourceData, blueprint, asset);
                    break;
                    
                case UndefinedSection:
                    // Cleanup previous list item
                    if (cur->type == ListBlockEndType ||
                        cur->type == ListItemBlockEndType)
                        result = SkipAfterListBlockEnd(cur, bounds.second);
                    
                    if (asset.empty())
                        result.first.warnings.push_back(Warning("empty asset",
                                                                0,
                                                                cur->sourceMap));
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, cur->sourceMap);
                    break;
            }
            
            return result;
        }
        
        static ParseSectionResult HandleBodySectionBlock(const BlockIterator& cur,
                                                         const SectionBounds& bounds,
                                                         const SourceData& sourceData,
                                                         const Blueprint& blueprint,
                                                         Asset& asset) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);

            // Eat leading list (item) harness
            if (cur == bounds.first) {
                result.second = FirstContentBlock(cur, bounds.second);
                
                if (result.second != bounds.second)
                    ++result.second; // skip payload signature
                
                return result;
            }
            
            // Retrieve asset
            BlockIterator sectionCur = cur;
            if (sectionCur->type == CodeBlockType) {
                asset += sectionCur->content;
            }
            else {
                
                if (sectionCur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (sectionCur->type == ListBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, ListBlockBeginType, ListBlockEndType);
                }
                
                result.first.warnings.push_back(Warning("asset is expected to be preformatted code block",
                                                        0,
                                                        sectionCur->sourceMap));
                asset += MapSourceData(sourceData, sectionCur->sourceMap);
            }
            
            result.second = ++sectionCur; // advance to next block
            return result;
        }
    };
    
    typedef BlockParser<Asset, SectionParser<Asset> > AssetParser;

}

#endif
