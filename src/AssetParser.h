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

    /** Body matching regex */
    const char* const BodyRegex = "^[[:blank:]]*[Bb]ody[[:blank:]]*$";

    /** Schema matching regex */
    const char* const SchemaRegex = "^[[:blank:]]*[Ss]chema[[:blank:]]*$";

    /**
     *  Asset Section Processor
     */
    template<>
    struct SectionProcessor<Asset> : public SectionProcessorBase<Asset> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     const ParseResultRef<Asset>& out) {

            out.node = "";
            CodeBlockUtility::signatureContentAsCodeBlock(node, pd, out.report, out.node);

            if (pd.exportSourceMap() && !out.node.empty()) {
                out.sourceMap.sourceMap.append(node->sourceMap);
            }

            return ++MarkdownNodeIterator(node);
        }

        NO_SECTION_DESCRIPTION(Asset)

        static MarkdownNodeIterator processContent(const MarkdownNodeIterator& node,
                                                   const MarkdownNodes& siblings,
                                                   SectionParserData& pd,
                                                   const ParseResultRef<Asset>& out) {

            mdp::ByteBuffer content;
            CodeBlockUtility::contentAsCodeBlock(node, pd, out.report, content);

            out.node += content;

            if (pd.exportSourceMap() && !content.empty()) {
                out.sourceMap.sourceMap.append(node->sourceMap);
            }

            return ++MarkdownNodeIterator(node);
        }

        static bool isContentNode(const MarkdownNodeIterator& node,
                                  SectionType sectionType) {

            return (SectionKeywordSignature(node) == UndefinedSectionType);
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

            mdp::ByteBuffer remaining, subject = node->children().front().text;
            subject = GetFirstLine(subject, remaining);
            TrimString(subject);

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

#endif
