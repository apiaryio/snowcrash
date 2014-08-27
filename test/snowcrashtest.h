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

    typedef std::vector<snowcrash::ResourceModelSymbol> Symbols;

    template <typename T, typename TSM, typename PARSER>
    struct SectionParserHelper {

        static void parse(const mdp::ByteBuffer& source,
                          snowcrash::SectionType type,
                          snowcrash::Report& report,
                          T& output,
                          const Symbols& symbols = Symbols(),
                          const snowcrash::BlueprintParserOptions& opts = 0,
                          snowcrash::Blueprint* bp = NULL) {

            mdp::MarkdownParser markdownParser;
            mdp::MarkdownNode markdownAST;

            snowcrash::Blueprint blueprint;
            snowcrash::Blueprint* bppointer;

            markdownParser.parse(source, markdownAST);

            REQUIRE(!markdownAST.children().empty());

            if (bp == NULL) {
                bppointer = &blueprint;
            } else {
                bppointer = bp;
            }

            snowcrash::SectionParserData pd(opts, source, *bppointer);

            pd.sectionsContext.push_back(type);

            pd.symbolTable.resourceModels.insert(symbols.begin(), symbols.end());

            TSM outputSM;

            PARSER::parse(markdownAST.children().begin(),
                          markdownAST.children(),
                          pd,
                          report,
                          output,
                          outputSM);
        }
    };
}

#endif
