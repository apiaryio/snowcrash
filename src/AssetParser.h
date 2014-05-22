//
//  AssetParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/17/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_ASSETPARSER_H
#define SNOWCRASH_ASSETPARSER_H

#include "SectionParser.h"
#include "RegexMatch.h"
#include "CodeBlockUtility.h"

namespace snowcrash {
    
    /// Asset signature
    enum AssetSignature {
        NoAssetSignature = 0,
        BodyAssetSignature,         /// < Explicit body asset
        ImplicitBodyAssetSignature, /// < Body asset using abbreviated syntax
        SchemaAssetSignature,       /// < Explicit Schema asset
        UndefinedAssetSignature = -1
    };
    
    /**
     *  Asset Section Processor
     */
    template<>
    struct SectionProcessor<Asset> : public SectionProcessorBase<Asset> {
        
        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     Report& report,
                                                     Asset& out) {
            
            CodeBlockUtility::signatureContentAsCodeBlock(node, pd, report, out);
            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       SectionParserData& pd,
                                                       Report& report,
                                                       Asset& out) {
            return node;
        }
        
        static MarkdownNodeIterator processContent(const MarkdownNodeIterator& node,
                                                   SectionParserData& pd,
                                                   Report& report,
                                                   Asset& out) {
            
            
            mdp::ByteBuffer content;
            CodeBlockUtility::contentAsCodeBlock(node, pd, report, content);
            out += content;
            return ++MarkdownNodeIterator(node);
        }
        
        static bool isDescriptionNode(const MarkdownNodeIterator& node) {
            return false;
        }
        
        static bool isContentNode(const MarkdownNodeIterator& node) {
            return !RecognizeSection(node);
        }
        
        static SectionType sectionType(const MarkdownNodeIterator& node) {
            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {
                
                AssetSignature signature = assetSignature(node);
                switch (signature) {
                    case BodyAssetSignature:
                    case ImplicitBodyAssetSignature:
                        return BodySectionType;

                    case SchemaAssetSignature:
                        return SchemaSectionType;
                        
                    default:
                        return UndefinedSectionType;
                }
            }
            return UndefinedSectionType;
        }
        
        /** Resolve asset signature */
        static AssetSignature assetSignature(const MarkdownNodeIterator& node) {
            mdp::ByteBuffer subject = node->children().front().text;
            
            /** Body matching regex */
            static const char* const BodyRegex = "^[[:blank:]]*[Bb]ody[[:blank:]]*$";
            
            /** Schema matching regex */
            static const char* const SchemaRegex = "^[[:blank:]]*[Ss]chema[[:blank:]]*$";
            
            if (RegexMatch(subject, BodyRegex))
                return BodyAssetSignature;

            if (RegexMatch(subject, SchemaRegex))
                return SchemaAssetSignature;
            
            return NoAssetSignature;
        }
    };
    
    /** Asset Section Parser */
    typedef SectionParser<Asset, ListSectionAdapter> AssetParser;
}

//#include <sstream>
//#include "BlueprintParserCore.h"
//#include "Blueprint.h"
//#include "RegexMatch.h"
//#include "StringUtility.h"
//#include "SectionUtility.h"
//
//
//
//namespace snowcrash {
//    
//    /// Asset signature
//    enum AssetSignature {
//        UndefinedAssetSignature,
//        NoAssetSignature,
//        BodyAssetSignature,         /// < Explicit body asset
//        PayloadBodyAssetSignature,  /// < Body asset using abbreviated syntax
//        SchemaAssetSignature,       /// < Explicit Schema asset
//        GenericAssetSignature
//    };
//    
//    // Query asset signature a of given block
//    FORCEINLINE AssetSignature GetAssetSignature(const BlockIterator& begin,
//                                                 const BlockIterator& end) {
//        
//        if (begin->type == ListBlockBeginType || begin->type == ListItemBlockBeginType) {
//            
//            BlockIterator cur = ListItemNameBlock(begin, end);
//            if (cur == end)
//                return NoAssetSignature;
//            
//            if (cur->type != ParagraphBlockType &&
//                cur->type != ListItemBlockEndType)
//                return NoAssetSignature;
//            
//            std::string content = GetFirstLine(cur->content);
//            if (RegexMatch(content, snowcrashconst::BodyRegex))
//                return BodyAssetSignature;
//
//            if (RegexMatch(content, snowcrashconst::SchemaRegex))
//                return SchemaAssetSignature;
//            
//            if (HasPayloadAssetSignature(begin, end))
//                return PayloadBodyAssetSignature;
//        }
//        
//        return NoAssetSignature;
//    }
//    
//    FORCEINLINE bool HasAssetSignature(const BlockIterator& begin,
//                                       const BlockIterator& end) {
//        AssetSignature signature = GetAssetSignature(begin, end);
//        return (signature != NoAssetSignature);
//    }
//    
//    /**
//     *  Return true if given block might be a dangling asset block,
//     *  false otherwise.
//     */
//    FORCEINLINE bool IsDanglingBlock(const BlockIterator& begin, const BlockIterator& end) {
//        return (begin->type == ParagraphBlockType
//                || begin->type == CodeBlockType);
//    }
//    
//    /**
//     *  Block Classifier, asset context.
//     */
//    template <>
//    FORCEINLINE SectionType ClassifyBlock<Asset>(const BlockIterator& begin,
//                                                 const BlockIterator& end,
//                                                 const SectionType& context) {
//        
//        if (context == UndefinedSectionType) {
//            AssetSignature asset = GetAssetSignature(begin, end);
//            if (asset == BodyAssetSignature || asset == PayloadBodyAssetSignature)
//                return BodySectionType;
//            if (asset == SchemaAssetSignature)
//                return SchemaSectionType;
//        }
//        else if (context == BodySectionType ||
//                 context == SchemaSectionType) {
//            
//            // SectionType closure
//            if (begin->type == ListItemBlockEndType ||
//                begin->type == ListBlockEndType) {
//
//                // Look ahead for a dangling asset
//                BlockIterator cur = CloseNestedList(begin, end);
//                if (cur == end)
//                    return UndefinedSectionType;
//                
//                if (IsDanglingBlock(cur, end))
//                    return (context == BodySectionType) ? DanglingBodySectionType : DanglingSchemaSectionType;
//                else
//                    return UndefinedSectionType;
//            }
//            
//            // Adjacent list item
//            if (begin->type == ListItemBlockBeginType) {
//                    return UndefinedSectionType;
//            }
//        }
//        else if (context == DanglingBodySectionType ||
//                 context == DanglingSchemaSectionType) {
//            
//            if (begin->type == ListItemBlockEndType ||
//                begin->type == ListBlockEndType)
//                return UndefinedSectionType;
//            
//            if (!IsDanglingBlock(begin, end))
//                return UndefinedSectionType;
//        }
//        
//        return (context == BodySectionType ||
//                context == SchemaSectionType ||
//                context == DanglingBodySectionType ||
//                context == DanglingSchemaSectionType) ? context : UndefinedSectionType;
//    }
//    
//    //
//    // Asset SectionType Parser
//    //
//    template<>
//    struct SectionParser<Asset> {
//        
//        static ParseSectionResult ParseSection(const BlueprintSection& section,
//                                               const BlockIterator& cur,
//                                               BlueprintParserCore& parser,
//                                               Asset& asset) {
//            
//            ParseSectionResult result = std::make_pair(Result(), cur);
//            switch (section.type) {
//                case BodySectionType:
//                case SchemaSectionType: 
//                    result = HandleAssetSectionBlock(section, cur, parser, asset);
//                    break;
//                    
//                case DanglingBodySectionType:
//                case DanglingSchemaSectionType:
//                    result = HandleDanglingAssetSectionBlock(section, cur, parser, asset);
//                    break;
//                    
//                case UndefinedSectionType:
//                    result.second = CloseList(cur, section.bounds.second);
//                    break;
//                    
//                default:
//                    result.first.error = UnexpectedBlockError(section, cur, parser.sourceData);
//                    break;
//            }
//            
//            return result;
//        }
//        
//        static void Finalize(const SectionBounds& bounds,
//                             BlueprintParserCore& parser,
//                             Asset& asset,
//                             Result& result) {}
//        
//        static ParseSectionResult HandleAssetSectionBlock(const BlueprintSection& section,
//                                                          const BlockIterator& cur,
//                                                          BlueprintParserCore& parser,
//                                                          Asset& asset) {
//
//            SourceData data;
//            SourceDataBlock sourceMap;
//            // NOTE: Use `Payload` traits for parsing pre-formatted list block.
//            ParseSectionResult result = ParseListPreformattedBlock<Payload>(section,
//                                                                            cur,
//                                                                            parser,
//                                                                            data,
//                                                                            sourceMap);
//            if (result.first.error.code != Error::OK ||
//                parser.sourceData.empty())
//                return result;
//            
//            asset += data;
//            return result;
//        }
//        
//        static ParseSectionResult HandleDanglingAssetSectionBlock(const BlueprintSection& section,
//                                                                  const BlockIterator& cur,
//                                                                  BlueprintParserCore& parser,
//                                                                  Asset& asset) {
//            
//            // Skip any closing list blocks
//            BlockIterator sectionCur = CloseNestedList(cur, section.bounds.second);
//            return HandleAssetSectionBlock(section, sectionCur, parser, asset);
//        }
//
//    };
//    
//    typedef BlockParser<Asset, SectionParser<Asset> > AssetParser;
//
//}

#endif
