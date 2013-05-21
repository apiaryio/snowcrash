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
#include "TrimString.h"

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
        }
        
        return NoAssetSignature;
    }
    
    inline bool HasAssetSignature(const BlockIterator& begin,
                                  const BlockIterator& end) {
        AssetSignature signature = GetAssetSignature(begin, end);
        return (signature == BodyAssetSignature || signature == SchemaAssetSignature);
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
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Asset& asset) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            switch (section) {
                case BodySection:
                case SchemaSection: 
                    result = HandleAssetSectionBlock(section, cur, bounds, sourceData, blueprint, asset);
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
                    
                    std::stringstream ss;
                    ss << "unable to parse " << SectionName(section) << " signature";
                    result.first.error = Error(ss.str(),
                                               1,
                                               sectionCur->sourceMap);
                    result.second = sectionCur;
                    return result;
                }
                
                // Add any extra lines after signature to asset body
                if (content.size() == 2 && !content[1].empty()) {
                    asset += content[1];
                    std::stringstream ss;
                    ss << SectionName(section) << AssetFormattingWarning;
                    result.first.warnings.push_back(Warning(ss.str(),
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

                std::stringstream ss;
                ss << SectionName(section) << AssetFormattingWarning;
                result.first.warnings.push_back(Warning(ss.str(),
                                                        0,
                                                        sectionCur->sourceMap));
            }
            
            if (sectionCur != bounds.second)
                result.second = ++sectionCur;
            
            return result;
        }
        
        static std::string SectionName(const Section& section) {
            switch (section) {
                case BodySection:
                    return "body";
                    
                case SchemaSection:
                    return "schema";
                    
                default:
                    return "section";
            }
        }
    };
    
    typedef BlockParser<Asset, SectionParser<Asset> > AssetParser;

}

#endif
