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

    // NOTE: +1 Error
    // Used version of sundown automatically adds a newline if one is missing.
    // If the input buffer does not ends with new line it might be "prolonged".
    
    parser.parse("Hello World!\n", ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.children.size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 13);
    
    ASTNode& node = ast.children.front();
    REQUIRE(node.type == ParagraphASTNodeType);
    REQUIRE(node.text == "Hello World!");
    REQUIRE(node.children.empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 13);
}
