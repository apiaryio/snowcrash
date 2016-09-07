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
    MarkdownNode ast;

    // NOTE: +1 Error
    // Used version of sundown automatically adds a newline if one is missing.
    // If the input buffer does not ends with new line it might be "prolonged".
    
    parser.parse("Hello World!\n", ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 13);
    
    MarkdownNode& node = ast.children().front();
    REQUIRE(node.type == ParagraphMarkdownNodeType);
    REQUIRE(node.text == "Hello World!");
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 13);
}

TEST_CASE("Parse when starting with empty line", "[parser][empty_line]")
{
    MarkdownParser parser;
    MarkdownNode ast;

    ByteBuffer src =\
    "\n"\
    "Lorem\n"\
    "\n"\
    "Ipsum\n";

    parser.parse(src, ast);

    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 2);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 14);

    MarkdownNode& node = ast.children()[0];
    REQUIRE(node.type == ParagraphMarkdownNodeType);
    REQUIRE(node.text == "Lorem");
    REQUIRE(node.data == 0);
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 1);
    REQUIRE(node.sourceMap[0].length == 7);

    node = ast.children()[1];
    REQUIRE(node.type == ParagraphMarkdownNodeType);
    REQUIRE(node.text == "Ipsum");
    REQUIRE(node.data == 0);
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 8);
    REQUIRE(node.sourceMap[0].length == 6);
}

TEST_CASE("Parse multiple paragaraphs", "[parser][paragraph]")
{
    MarkdownParser parser;
    MarkdownNode ast;
    
    ByteBuffer src =\
    "Lorem\n"\
    "\n"\
    "Ipsum\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 2);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 13);
    
    MarkdownNode& node = ast.children()[0];
    REQUIRE(node.type == ParagraphMarkdownNodeType);
    REQUIRE(node.text == "Lorem");
    REQUIRE(node.data == 0);
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 7);
    
    node = ast.children()[1];
    REQUIRE(node.type == ParagraphMarkdownNodeType);
    REQUIRE(node.text == "Ipsum");
    REQUIRE(node.data == 0);
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 7);
    REQUIRE(node.sourceMap[0].length == 6);
}

TEST_CASE("Parse header", "[parser][header]")
{
    MarkdownParser parser;
    MarkdownNode ast;
    
    ByteBuffer src = "# Header\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 9);
    
    MarkdownNode& node = ast.children().front();
    REQUIRE(node.type == HeaderMarkdownNodeType);
    REQUIRE(node.text == "Header");
    REQUIRE(node.data == 1);
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 9);
}

TEST_CASE("Parse multiple headers", "[parser][header]")
{
    MarkdownParser parser;
    MarkdownNode ast;
    
    ByteBuffer src =\
    "# Header 1\n"\
    "## Header 2\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 2);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 23);
    
    MarkdownNode node = ast.children()[0];
    REQUIRE(node.type == HeaderMarkdownNodeType);
    REQUIRE(node.text == "Header 1");
    REQUIRE(node.data == 1);
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 11);
    
    node = ast.children()[1];
    REQUIRE(node.type == HeaderMarkdownNodeType);
    REQUIRE(node.text == "Header 2");
    REQUIRE(node.data == 2);
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 11);
    REQUIRE(node.sourceMap[0].length == 12);
}

TEST_CASE("Parse horizontal rule", "[parser][hrule]")
{
    MarkdownParser parser;
    MarkdownNode ast;
    
    ByteBuffer src = "---\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 4);
    
    MarkdownNode node = ast.children().front();
    REQUIRE(node.type == HRuleMarkdownNodeType);
    REQUIRE(node.text.empty());
    REQUIRE(node.data == 0);
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 4);
}

TEST_CASE("Parse code block", "[parser][code]")
{
    MarkdownParser parser;
    MarkdownNode ast;
    
    ByteBuffer src = "    <code>42</code>\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 20);
    
    MarkdownNode node = ast.children().front();
    REQUIRE(node.type == CodeMarkdownNodeType);
    REQUIRE(node.text == "<code>42</code>\n");
    REQUIRE(node.data == 0);
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 20);
}

TEST_CASE("Parse HTML block tag", "[parser][html]")
{
    MarkdownParser parser;
    MarkdownNode ast;
    
    ByteBuffer src = "<div>some</div>\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 16);
    
    MarkdownNode node = ast.children().front();
    REQUIRE(node.type == HTMLMarkdownNodeType);
    REQUIRE(node.text == "<div>some</div>\n");
    REQUIRE(node.data == 0);
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 16);
}

TEST_CASE("Parse single list item", "[parser][list]")
{
    MarkdownParser parser;
    MarkdownNode ast;
    
    ByteBuffer src = "- list item\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 12);
    
    MarkdownNode node = ast.children()[0];
    REQUIRE(node.type == ListItemMarkdownNodeType);
    REQUIRE(node.text.empty());
    REQUIRE(node.data == 0);
    REQUIRE(node.children().size() == 1);
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 12);
    
    node = node.children()[0];
    REQUIRE(node.type == ParagraphMarkdownNodeType);
    REQUIRE(node.text == "list item\n");
    REQUIRE(node.data == 0);
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 2);
    REQUIRE(node.sourceMap[0].length == 10);
}

TEST_CASE("Parse nested list items", "[parser][list]")
{
    MarkdownParser parser;
    MarkdownNode ast;
    
    ByteBuffer src =\
    "- A\n"\
    "    - B\n"\
    "        - C\n"\
    "    - D\n"\
    "- E\n";
    
    /* Topology:
     
     + root
        + list item
            + paragraph "A"
            + list item
                + paragraph "B"
                + list item
                    + paragraph "C"
            + list item
                + paragraph "D"
        + list item
            + paragraph "E"
     */
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 2);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 36);
    
    // List Item A
    MarkdownNode& itemA = ast.children()[0];
    REQUIRE(itemA.type == ListItemMarkdownNodeType);
    REQUIRE(itemA.text.empty());
    REQUIRE(itemA.data == 0);
    REQUIRE(itemA.children().size() == 3);
    REQUIRE(itemA.sourceMap.size() == 1);
    REQUIRE(itemA.sourceMap[0].location == 0);
    REQUIRE(itemA.sourceMap[0].length == 32);
    
    REQUIRE(itemA.children()[0].type == ParagraphMarkdownNodeType);
    REQUIRE(itemA.children()[0].text == "A\n");
    REQUIRE(itemA.children()[0].data == 0);
    REQUIRE(itemA.children()[0].children().empty());
    REQUIRE(itemA.children()[0].sourceMap.size() == 1);
    REQUIRE(itemA.children()[0].sourceMap[0].location == 2);
    REQUIRE(itemA.children()[0].sourceMap[0].length == 2);
    
    // List Item B
    MarkdownNode itemB = itemA.children()[1];
    REQUIRE(itemB.type == ListItemMarkdownNodeType);
    REQUIRE(itemB.text.empty());
    REQUIRE(itemB.data == 0);
    REQUIRE(itemB.children().size() == 2);
    REQUIRE(itemB.sourceMap.size() == 2);
    REQUIRE(itemB.sourceMap[0].location == 8);
    REQUIRE(itemB.sourceMap[0].length == 4);
    REQUIRE(itemB.sourceMap[1].location == 16);
    REQUIRE(itemB.sourceMap[1].length == 8);
    
    REQUIRE(itemB.children()[0].type == ParagraphMarkdownNodeType);
    REQUIRE(itemB.children()[0].text == "B\n");
    REQUIRE(itemB.children()[0].data == 0);
    REQUIRE(itemB.children()[0].children().empty());
    MarkdownNode paraBX = itemB.children()[0];
    REQUIRE(itemB.children()[0].sourceMap.size() == 1);
    REQUIRE(itemB.children()[0].sourceMap[0].location == 10);
    REQUIRE(itemB.children()[0].sourceMap[0].length == 2);

    // List Item C
    MarkdownNode itemC = itemB.children()[1];
    REQUIRE(itemC.type == ListItemMarkdownNodeType);
    REQUIRE(itemC.text.empty());
    REQUIRE(itemC.data == 0);
    REQUIRE(itemC.children().size() == 1);
    REQUIRE(itemC.sourceMap.size() == 1);
    REQUIRE(itemC.sourceMap[0].location == 20);
    REQUIRE(itemC.sourceMap[0].length == 4);
    
    REQUIRE(itemC.children()[0].type == ParagraphMarkdownNodeType);
    REQUIRE(itemC.children()[0].text == "C\n");
    REQUIRE(itemC.children()[0].data == 0);
    REQUIRE(itemC.children()[0].children().empty());
    REQUIRE(itemC.children()[0].sourceMap.size() == 1);
    REQUIRE(itemC.children()[0].sourceMap[0].location == 22);
    REQUIRE(itemC.children()[0].sourceMap[0].length == 2);
    
    // List Item D
    MarkdownNode itemD = itemA.children()[2];
    REQUIRE(itemD.type == ListItemMarkdownNodeType);
    REQUIRE(itemD.text.empty());
    REQUIRE(itemD.data == 0);
    REQUIRE(itemD.children().size() == 1);
    REQUIRE(itemD.sourceMap.size() == 1);
    REQUIRE(itemD.sourceMap[0].location == 28);
    REQUIRE(itemD.sourceMap[0].length == 4);

    REQUIRE(itemD.children()[0].type == ParagraphMarkdownNodeType);
    REQUIRE(itemD.children()[0].text == "D\n");
    REQUIRE(itemD.children()[0].data == 0);
    REQUIRE(itemD.children()[0].children().empty());
    REQUIRE(itemD.children()[0].sourceMap.size() == 1);
    REQUIRE(itemD.children()[0].sourceMap[0].location == 30);
    REQUIRE(itemD.children()[0].sourceMap[0].length == 2);
    
    // List Item E
    MarkdownNode itemE = ast.children()[1];
    REQUIRE(itemE.type == ListItemMarkdownNodeType);
    REQUIRE(itemE.text.empty());
    REQUIRE(itemE.data == 0);
    REQUIRE(itemE.children().size() == 1);
    REQUIRE(itemE.sourceMap.size() == 1);
    REQUIRE(itemE.sourceMap[0].location == 32);
    REQUIRE(itemE.sourceMap[0].length == 4);
    
    REQUIRE(itemE.children()[0].type == ParagraphMarkdownNodeType);
    REQUIRE(itemE.children()[0].text == "E\n");
    REQUIRE(itemE.children()[0].data == 0);
    REQUIRE(itemE.children()[0].children().empty());
    REQUIRE(itemE.children()[0].sourceMap.size() == 1);
    REQUIRE(itemE.children()[0].sourceMap[0].location == 34);
    REQUIRE(itemE.children()[0].sourceMap[0].length == 2);
}

TEST_CASE("Parse list item with multiple paragraphs", "[parser][list]")
{
    MarkdownParser parser;
    MarkdownNode ast;
    
    ByteBuffer src =\
    "- A\n"\
    "\n"\
    "    C\n"\
    "    D\n"\
    "\n"\
    "    E\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 24);
    
    MarkdownNode node = ast.children()[0];
    REQUIRE(node.type == ListItemMarkdownNodeType);
    REQUIRE(node.text.empty());
    REQUIRE(node.data == 2);
    REQUIRE(node.children().size() == 3);
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 24);

    MarkdownNode& p1 = node.children()[0];
    REQUIRE(p1.type == ParagraphMarkdownNodeType);
    REQUIRE(p1.text == "A");
    REQUIRE(p1.data == 0);
    REQUIRE(p1.children().empty());
    REQUIRE(p1.sourceMap.size() == 1);
    REQUIRE(p1.sourceMap[0].location == 2);
    REQUIRE(p1.sourceMap[0].length == 3);
    
    MarkdownNode& p2 = node.children()[1];
    REQUIRE(p2.type == ParagraphMarkdownNodeType);
    REQUIRE(p2.text == "C\nD");
    REQUIRE(p2.data == 0);
    REQUIRE(p2.children().empty());
    REQUIRE(p2.sourceMap.size() == 2);
    REQUIRE(p2.sourceMap[0].location == 9);
    REQUIRE(p2.sourceMap[0].length == 2);
    REQUIRE(p2.sourceMap[1].location == 15);
    REQUIRE(p2.sourceMap[1].length == 3);
    
    MarkdownNode& p3 = node.children()[2];
    REQUIRE(p3.type == ParagraphMarkdownNodeType);
    REQUIRE(p3.text == "E");
    REQUIRE(p3.data == 0);
    REQUIRE(p3.children().empty());
    REQUIRE(p3.sourceMap.size() == 1);
    REQUIRE(p3.sourceMap[0].location == 22);
    REQUIRE(p3.sourceMap[0].length == 2);
}

TEST_CASE("Parse a simple quote", "[parser][quote]")
{
    MarkdownParser parser;
    MarkdownNode ast;
    
    ByteBuffer src = "> quote\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 8);
    
    MarkdownNode& quote = ast.children()[0];
    REQUIRE(quote.type == QuoteMarkdownNodeType);
    REQUIRE(quote.text.empty());
    REQUIRE(quote.data == 0);
    REQUIRE(quote.children().size() == 1);
    REQUIRE(quote.sourceMap.size() == 1);
    REQUIRE(quote.sourceMap[0].location == 0);
    REQUIRE(quote.sourceMap[0].length == 8);
    
    MarkdownNode& para = quote.children()[0];
    REQUIRE(para.type == ParagraphMarkdownNodeType);
    REQUIRE(para.text == "quote");
    REQUIRE(para.data == 0);
    REQUIRE(para.children().empty());
    REQUIRE(para.sourceMap.size() == 1);
    REQUIRE(para.sourceMap[0].location == 2);
    REQUIRE(para.sourceMap[0].length == 6);
}

TEST_CASE("Source map crash", "[parser][sourcemap][issue][snowcrash][62]")
{
    MarkdownParser parser;
    MarkdownNode ast;
    
    ByteBuffer src = \
    "* B\n"\
    ">* CCC CC\n"\
    ">* D\n"\
    "\n"\
    "* E\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 2);
}

TEST_CASE("Map node without trailing newline", "[parser][sourcemap]")
{
    MarkdownParser parser;
    MarkdownNode ast;
    
    ByteBuffer src = "# Hello World";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 13);
    
    MarkdownNode& node = ast.children().front();
    REQUIRE(node.type == HeaderMarkdownNodeType);
    REQUIRE(node.text == "Hello World");
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 13);
}

TEST_CASE("Parse six nested level list items", "[parser]")
{
    MarkdownParser parser;
    MarkdownNode ast;

    ByteBuffer src = \
    "+ 1\n"\
    "    + 2\n"\
    "        + 3\n"\
    "            + 4\n"\
    "                + 5\n"\
    "                    + 6\n";

    parser.parse(src, ast);

    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.children().size() == 1);

    MarkdownNode& list = ast.children().back();
    REQUIRE(list.type == ListItemMarkdownNodeType);
    REQUIRE(list.children().size() == 2);
    REQUIRE(list.children().front().type == ParagraphMarkdownNodeType);
    REQUIRE(list.children().front().text == "1\n");

    list = list.children().back();
    REQUIRE(list.type == ListItemMarkdownNodeType);
    REQUIRE(list.children().size() == 2);
    REQUIRE(list.children().front().type == ParagraphMarkdownNodeType);
    REQUIRE(list.children().front().text == "2\n");

    list = list.children().back();
    REQUIRE(list.type == ListItemMarkdownNodeType);
    REQUIRE(list.children().size() == 2);
    REQUIRE(list.children().front().type == ParagraphMarkdownNodeType);
    REQUIRE(list.children().front().text == "3\n");

    list = list.children().back();
    REQUIRE(list.type == ListItemMarkdownNodeType);
    REQUIRE(list.children().size() == 2);
    REQUIRE(list.children().front().type == ParagraphMarkdownNodeType);
    REQUIRE(list.children().front().text == "4\n");

    list = list.children().back();
    REQUIRE(list.type == ListItemMarkdownNodeType);
    REQUIRE(list.children().size() == 2);
    REQUIRE(list.children().front().type == ParagraphMarkdownNodeType);
    REQUIRE(list.children().front().text == "5\n");

    list = list.children().back();
    REQUIRE(list.type == ListItemMarkdownNodeType);
    REQUIRE(list.children().size() == 1);
    REQUIRE(list.children().front().type == ParagraphMarkdownNodeType);
    REQUIRE(list.children().front().text == "6\n");
}

TEST_CASE("Multi-paragraph list item source map", "[parser][sourcemap]")
{
    MarkdownParser parser;
    MarkdownNode ast;

    // List item with two paragraphs & lazy indentation:
    //
    //+   Lorem ipsum
    //dolor sit amet
    //
    //    consectetur
    //adipiscing elit
    
    ByteBuffer src = \
    "+   Lorem ipsum\n"\
    "dolor sit amet\n"\
    "\n"\
    "    consectetur\n"\
    "adipiscing elit\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 64);
    
    MarkdownNode& node = ast.children().front();
    REQUIRE(node.type == ListItemMarkdownNodeType);
    REQUIRE(node.children().size() == 2);
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 64);

    MarkdownNode& p1 = node.children().front();
    REQUIRE(p1.text == "  Lorem ipsum\ndolor sit amet");
    REQUIRE(p1.children().empty());
    REQUIRE(p1.sourceMap.size() == 1);
    REQUIRE(p1.sourceMap[0].location == 2);
    REQUIRE(p1.sourceMap[0].length == 30);
    
    MarkdownNode& p2 = node.children().back();
    REQUIRE(p2.text == "consectetur\nadipiscing elit");
    REQUIRE(p2.children().empty());
    REQUIRE(p2.sourceMap.size() == 1);
    REQUIRE(p2.sourceMap[0].location == 36);
    REQUIRE(p2.sourceMap[0].length == 28);
}

TEST_CASE("Sublist should have more indentation than the list item", "[parser]")
{
    MarkdownParser parser;
    MarkdownNode ast;

    ByteBuffer src = \
    " + 1\n"\
    "+ 2\n";

    parser.parse(src, ast);

    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.children().size() == 2);

    MarkdownNode& list = ast.children().front();
    REQUIRE(list.type == ListItemMarkdownNodeType);
    REQUIRE(list.children().size() == 1);
    REQUIRE(list.children().front().type == ParagraphMarkdownNodeType);
    REQUIRE(list.children().front().text == "1\n");

    list = ast.children().back();
    REQUIRE(list.type == ListItemMarkdownNodeType);
    REQUIRE(list.children().size() == 1);
    REQUIRE(list.children().front().type == ParagraphMarkdownNodeType);
    REQUIRE(list.children().front().text == "2\n");
}
