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
    
    template <typename T, typename PARSER>
    struct SectionParserHelper {
        
        static void parse(const mdp::ByteBuffer& source,
                          snowcrash::SectionType type,
                          snowcrash::Report& report,
                          T& output) {
            
            mdp::MarkdownParser markdownParser;
            mdp::MarkdownNode markdownAST;
            markdownParser.parse(source, markdownAST);
            
            REQUIRE(!markdownAST.children().empty());
            
            snowcrash::Blueprint bp;
            snowcrash::SectionParserData pd(0, source, bp);
            pd.sectionsContext.push_back(type);
            
            PARSER::parse(markdownAST.children().begin(),
                          markdownAST.children(),
                          pd,
                          report,
                          output);
        }

        static void symbolAndParse(const mdp::ByteBuffer& source,
                                   const snowcrash::ResourceModelSymbol& symbol,
                                   snowcrash::SectionType type,
                                   snowcrash::Report& report,
                                   T& output) {

            mdp::MarkdownParser markdownParser;
            mdp::MarkdownNode markdownAST;
            markdownParser.parse(source, markdownAST);

            REQUIRE(!markdownAST.children().empty());

            snowcrash::Blueprint bp;
            snowcrash::SectionParserData pd(0, source, bp);

            pd.symbolTable.resourceModels.insert(symbol);
            pd.sectionsContext.push_back(type);

            PARSER::parse(markdownAST.children().begin(),
                          markdownAST.children(),
                          pd,
                          report,
                          output);
        }
    };
}

#endif
