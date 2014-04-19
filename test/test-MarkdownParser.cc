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

TEST_CASE("Parse one paragaraph", "[parser][paragraph]")
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

TEST_CASE("Parse multiple paragaraphs", "[parser][paragraph]")
{
    MarkdownParser parser;
    ASTNode ast;
    
    ByteBuffer src =\
    "Lorem\n"\
    "\n"\
    "Ipsum\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children.size() == 2);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 13);
    
    ASTNode& node = ast.children[0];
    REQUIRE(node.type == ParagraphASTNodeType);
    REQUIRE(node.text == "Lorem");
    REQUIRE(node.data == 0);
    REQUIRE(node.children.empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 7);
    
    node = ast.children[1];
    REQUIRE(node.type == ParagraphASTNodeType);
    REQUIRE(node.text == "Ipsum");
    REQUIRE(node.data == 0);
    REQUIRE(node.children.empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 7);
    REQUIRE(node.sourceMap[0].length == 6);
}

TEST_CASE("Parse header", "[parser][header]")
{
    MarkdownParser parser;
    ASTNode ast;
    
    ByteBuffer src = "# Header\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children.size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 9);
    
    ASTNode& node = ast.children.front();
    REQUIRE(node.type == HeaderASTNodeType);
    REQUIRE(node.text == "Header");
    REQUIRE(node.data == 1);
    REQUIRE(node.children.empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 9);
}

TEST_CASE("Parse multiple headers", "[parser][header]")
{
    MarkdownParser parser;
    ASTNode ast;
    
    ByteBuffer src =\
    "# Header 1\n"\
    "## Header 2\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children.size() == 2);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 23);
    
    ASTNode node = ast.children[0];
    REQUIRE(node.type == HeaderASTNodeType);
    REQUIRE(node.text == "Header 1");
    REQUIRE(node.data == 1);
    REQUIRE(node.children.empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 11);
    
    node = ast.children[1];
    REQUIRE(node.type == HeaderASTNodeType);
    REQUIRE(node.text == "Header 2");
    REQUIRE(node.data == 2);
    REQUIRE(node.children.empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 11);
    REQUIRE(node.sourceMap[0].length == 12);
}

TEST_CASE("Parse horizontal rule", "[parser][hrule]")
{
    MarkdownParser parser;
    ASTNode ast;
    
    ByteBuffer src = "---\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children.size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 4);
    
    ASTNode node = ast.children.front();
    REQUIRE(node.type == HRuleASTNodeType);
    REQUIRE(node.text.empty());
    REQUIRE(node.data == 0);
    REQUIRE(node.children.empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 4);
}

TEST_CASE("Parse code block", "[parser][code]")
{
    MarkdownParser parser;
    ASTNode ast;
    
    ByteBuffer src = "    <code>42</code>\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children.size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 20);
    
    ASTNode node = ast.children.front();
    REQUIRE(node.type == CodeASTNodeType);
    REQUIRE(node.text == "<code>42</code>\n");
    REQUIRE(node.data == 0);
    REQUIRE(node.children.empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 20);
}

TEST_CASE("Parse HTML block tag", "[parser][html]")
{
    MarkdownParser parser;
    ASTNode ast;
    
    ByteBuffer src = "<div>some</div>\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children.size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 16);
    
    ASTNode node = ast.children.front();
    REQUIRE(node.type == HTMLASTNodeType);
    REQUIRE(node.text == "<div>some</div>\n");
    REQUIRE(node.data == 0);
    REQUIRE(node.children.empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 16);
}

TEST_CASE("Parse single list item", "[parser][list][now]")
{
    MarkdownParser parser;
    ASTNode ast;
    
    ByteBuffer src = "- list item\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children.size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 12);
    
    ASTNode node = ast.children[0];
    REQUIRE(node.type == ListASTNodeType);
    REQUIRE(node.text.empty());
    REQUIRE(node.data == 0);
    REQUIRE(node.children.size() == 1);
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 12);
    
    node = node.children[0];
    REQUIRE(node.type == ListItemASTNodeType);
    REQUIRE(node.text == "list item\n");
    REQUIRE(node.data == 0);
    REQUIRE(node.children.empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 2);
    REQUIRE(node.sourceMap[0].length == 10);
}
