//
//  test-MarkdownParser.cc
//  markdownparser
//
//  Created by Zdenek Nemec on 4/18/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "catch.hpp"
#include "MarkdownParser.h"

using namespace mdp;

TEST_CASE("Parse one paragaraph", "[parser]")
{
    MarkdownParser parser;
    ASTNode ast;

    parser.parse("Hello World!", ast);
    
    REQUIRE(ast.type == ParagraphASTNodeType);
    REQUIRE(ast.text == "Hello World!");
    REQUIRE(ast.children.empty());
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 12);
}
