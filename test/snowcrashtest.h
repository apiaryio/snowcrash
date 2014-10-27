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

    struct Models {

        std::vector<snowcrash::ResourceModelSymbol> models;
        std::vector<snowcrash::ResourceModelSymbolSourceMap> modelsSM;

    };

    template <typename T, typename PARSER>
    struct SectionParserHelper {

        static void parse(const mdp::ByteBuffer& source,
                          snowcrash::SectionType type,
                          const snowcrash::ParseResultRef<T>& out,
                          const snowcrash::BlueprintParserOptions& opts = 0,
                          const Models& models = Models(),
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

            pd.modelTable.resourceModels.insert(models.models.begin(), models.models.end());
            pd.modelSourceMapTable.resourceModels.insert(models.modelsSM.begin(), models.modelsSM.end());

            PARSER::parse(markdownAST.children().begin(),
                          markdownAST.children(),
                          pd,
                          out);
        }
    };

    struct ModelHelper {

        /** Builds a Symbols entry for testing purposes */
        static void buildModel(mdp::ByteBuffer name,
                               Models& models) {

            snowcrash::ResourceModel model;
            snowcrash::SourceMap<snowcrash::ResourceModel> modelSM;
            mdp::BytesRangeSet sourcemap;

            sourcemap.push_back(mdp::BytesRange(0, 1));

            model.description = "Foo";
            model.body = "Bar";
            models.models.push_back(snowcrash::ResourceModelSymbol(name, model));

            modelSM.description.sourceMap = sourcemap;
            modelSM.body.sourceMap = sourcemap;
            models.modelsSM.push_back(snowcrash::ResourceModelSymbolSourceMap(name, modelSM));
        }
    };
}

#endif
