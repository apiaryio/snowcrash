//
//  snowcrashtest.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/21/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SNOWCRASHTEST_H
#define SNOWCRASH_SNOWCRASHTEST_H

#include "catch.hpp"
#include "MarkdownParser.h"
#include "SectionParser.h"

namespace snowcrashtest {

    struct Symbols {

        std::vector<snowcrash::ResourceModelSymbol> models;
        std::vector<snowcrash::ResourceModelSymbolSourceMap> modelsSM;

    };

    template <typename T, typename PARSER>
    struct SectionParserHelper {

        static void parse(const mdp::ByteBuffer& source,
                          snowcrash::SectionType type,
                          const snowcrash::ParseResultRef<T>& out,
                          const snowcrash::BlueprintParserOptions& opts = 0,
                          const Symbols& symbols = Symbols(),
                          snowcrash::ParseResult<snowcrash::Blueprint>* bp = NULL) {

            mdp::MarkdownParser markdownParser;
            mdp::MarkdownNode markdownAST;

            snowcrash::ParseResult<snowcrash::Blueprint> blueprint;
            snowcrash::ParseResult<snowcrash::Blueprint>* bppointer;

            markdownParser.parse(source, markdownAST);

            REQUIRE(!markdownAST.children().empty());

            if (bp == NULL) {
                bppointer = &blueprint;
            } else {
                bppointer = bp;
            }

            snowcrash::SectionParserData pd(opts, source, bppointer->node);

            pd.sectionsContext.push_back(type);

            pd.symbolTable.resourceModels.insert(symbols.models.begin(), symbols.models.end());
            pd.symbolSourceMapTable.resourceModels.insert(symbols.modelsSM.begin(), symbols.modelsSM.end());

            PARSER::parse(markdownAST.children().begin(),
                          markdownAST.children(),
                          pd,
                          out);
        }
    };

    struct SymbolHelper {

        /** Builds a Symbols entry for testing purposes */
        static void buildSymbol(mdp::ByteBuffer name,
                                Symbols& symbols) {

            snowcrash::ResourceModel model;
            snowcrash::SourceMap<snowcrash::ResourceModel> modelSM;
            mdp::BytesRangeSet sourcemap;

            sourcemap.push_back(mdp::BytesRange(0, 1));

            model.description = "Foo";
            model.body = "Bar";
            symbols.models.push_back(snowcrash::ResourceModelSymbol(name, model));

            modelSM.description.sourceMap = sourcemap;
            modelSM.body.sourceMap = sourcemap;
            symbols.modelsSM.push_back(snowcrash::ResourceModelSymbolSourceMap(name, modelSM));
        }
    };

    /**
     * \brief Helper to test signature parsing. Uses Blueprint as dummy type.
     */
    struct SignatureParserHelper {

        static scpl::Signature parse(const mdp::ByteBuffer& source,
                                     const snowcrash::ParseResultRef<snowcrash::Blueprint>& out,
                                     const scpl::SignatureTraits::Traits traits,
                                     const mdp::MarkdownNode* node = NULL) {

            mdp::MarkdownParser markdownParser;
            mdp::MarkdownNode markdownAST;

            snowcrash::ParseResult<snowcrash::Blueprint> blueprint;
            snowcrash::ParseResult<snowcrash::Blueprint>* bppointer;

            if (node == NULL) {
                markdownParser.parse(source, markdownAST);
            } else {
                markdownAST = *node;
            }

            REQUIRE(!markdownAST.children().empty());

            bppointer = &blueprint;
            snowcrash::SectionParserData pd(0, source, bppointer->node);

            scpl::Signature signature;
            scpl::SignatureTraits signatureTraits(traits);

            signature = scpl::SignatureSectionProcessorBase<snowcrash::Blueprint>
                            ::parseSignature(markdownAST.children().begin(), pd, signatureTraits, out);

            return signature;
        }
    };
}

#endif
