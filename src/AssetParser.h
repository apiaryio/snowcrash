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

static const std::string AssetFormattingWarning = "body asset is expected to be preformatted code block";

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
            
            BlockIterator cur = ListItemNameBlock(begin, end);
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
            
            // Section closure
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSection;
            
            // Adjacent list item
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
                    result.second = CloseListItemBlock(cur, bounds.second);
                    
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
            BlockIterator sectionCur = cur;
            
            if (sectionCur == bounds.first) {

                sectionCur = ListItemNameBlock(cur, bounds.second);
                if (sectionCur == bounds.second)
                    return std::make_pair(Result(), sectionCur);
                
                ContentParts content = ExtractFirstLine(*sectionCur);
                if (content.empty() ||
                    content.front().empty()) {
                    result.first.error = Error("unable to parse body signature",
                                               1,
                                               sectionCur->sourceMap);
                    result.second = sectionCur;
                    return result;
                }
                
                // Add any extra lines after signature to asset body
                if (content.size() == 2 && !content[1].empty()) {
                    asset += content[1];
                    result.first.warnings.push_back(Warning(AssetFormattingWarning,
                                                            0,
                                                            sectionCur->sourceMap));
                }
                                
                sectionCur = FirstContentBlock(cur, bounds.second);
            }
            else if (sectionCur->type == CodeBlockType) {
                // Well-formed asset body
                asset += sectionCur->content;
            }
            else {
                // Other blocks
                if (sectionCur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (sectionCur->type == ListBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, ListBlockBeginType, ListBlockEndType);
                }
                
                asset += MapSourceData(sourceData, sectionCur->sourceMap);

                result.first.warnings.push_back(Warning(AssetFormattingWarning,
                                                        0,
                                                        sectionCur->sourceMap));
            }
            
            if (sectionCur != bounds.second)
                result.second = ++sectionCur;
            
            return result;
        }        
    };
    
    typedef BlockParser<Asset, SectionParser<Asset> > AssetParser;

}

#endif
