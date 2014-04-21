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
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 13);
    
    ASTNode& node = ast.children().front();
    REQUIRE(node.type == ParagraphASTNodeType);
    REQUIRE(node.text == "Hello World!");
    REQUIRE(node.children().empty());
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
    REQUIRE(ast.children().size() == 2);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 13);
    
    ASTNode& node = ast.children()[0];
    REQUIRE(node.type == ParagraphASTNodeType);
    REQUIRE(node.text == "Lorem");
    REQUIRE(node.data == 0);
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 7);
    
    node = ast.children()[1];
    REQUIRE(node.type == ParagraphASTNodeType);
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
    ASTNode ast;
    
    ByteBuffer src = "# Header\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 9);
    
    ASTNode& node = ast.children().front();
    REQUIRE(node.type == HeaderASTNodeType);
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
    ASTNode ast;
    
    ByteBuffer src =\
    "# Header 1\n"\
    "## Header 2\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 2);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 23);
    
    ASTNode node = ast.children()[0];
    REQUIRE(node.type == HeaderASTNodeType);
    REQUIRE(node.text == "Header 1");
    REQUIRE(node.data == 1);
    REQUIRE(node.children().empty());
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 11);
    
    node = ast.children()[1];
    REQUIRE(node.type == HeaderASTNodeType);
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
    ASTNode ast;
    
    ByteBuffer src = "---\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 4);
    
    ASTNode node = ast.children().front();
    REQUIRE(node.type == HRuleASTNodeType);
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
    ASTNode ast;
    
    ByteBuffer src = "    <code>42</code>\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 20);
    
    ASTNode node = ast.children().front();
    REQUIRE(node.type == CodeASTNodeType);
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
    ASTNode ast;
    
    ByteBuffer src = "<div>some</div>\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 16);
    
    ASTNode node = ast.children().front();
    REQUIRE(node.type == HTMLASTNodeType);
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
    ASTNode ast;
    
    ByteBuffer src = "- list item\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 12);
    
    ASTNode node = ast.children()[0];
    REQUIRE(node.type == ListASTNodeType);
    REQUIRE(node.text.empty());
    REQUIRE(node.data == 0);
    REQUIRE(node.children().size() == 1);
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 12);
    
    node = node.children()[0];
    REQUIRE(node.type == ListItemASTNodeType);
    REQUIRE(node.text.empty());
    REQUIRE(node.data == 0);
    REQUIRE(node.children().size() == 1);
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 2);
    REQUIRE(node.sourceMap[0].length == 10);
    
    node = node.children()[0];
    REQUIRE(node.type == ParagraphASTNodeType);
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
    ASTNode ast;
    
    ByteBuffer src =\
    "- A\n"\
    "    - B\n"\
    "        - C\n"\
    "    - D\n"\
    "- E\n";
    
    /* Topology:
     
     + root
        + list (1)
            + list item 
                + paragraph "A"
                + list (2)
                    + list item
                        + paragraph "B"
                        + list (3)
                            + list item
                                + paragraph "C"
                    + list item
                        + paragraph "D"
            + list item
                + paragraph "E"
     */
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 36);
    
    ASTNode& list1 = ast.children()[0];
    REQUIRE(list1.type == ListASTNodeType);
    REQUIRE(list1.text.empty());
    REQUIRE(list1.data == 0);
    REQUIRE(list1.children().size() == 2);
    REQUIRE(list1.sourceMap.size() == 1);
    REQUIRE(list1.sourceMap[0].location == 0);
    REQUIRE(list1.sourceMap[0].length == 36);
    
    // List Item A
    ASTNode itemA = list1.children()[0];
    REQUIRE(itemA.type == ListItemASTNodeType);
    REQUIRE(itemA.text.empty());
    REQUIRE(itemA.data == 0);
    REQUIRE(itemA.children().size() == 2);
    REQUIRE(itemA.sourceMap.size() == 4);
    REQUIRE(itemA.sourceMap[0].location == 2);
    REQUIRE(itemA.sourceMap[0].length == 2);
    REQUIRE(itemA.sourceMap[1].location == 8);
    REQUIRE(itemA.sourceMap[1].length == 4);
    REQUIRE(itemA.sourceMap[2].location == 16);
    REQUIRE(itemA.sourceMap[2].length == 8);
    REQUIRE(itemA.sourceMap[3].location == 28);
    REQUIRE(itemA.sourceMap[3].length == 4);
    
    REQUIRE(itemA.children()[0].type == ParagraphASTNodeType);
    REQUIRE(itemA.children()[0].text == "A\n");
    REQUIRE(itemA.children()[0].data == 0);
    REQUIRE(itemA.children()[0].children().empty());
    
    // Note: in case of artifical nodes source map is inherited from the parents
    REQUIRE(itemA.children()[0].sourceMap.size() == 4);
    REQUIRE(itemA.children()[0].sourceMap[0].location == 2);
    REQUIRE(itemA.children()[0].sourceMap[0].length == 2);
    REQUIRE(itemA.children()[0].sourceMap[1].location == 8);
    REQUIRE(itemA.children()[0].sourceMap[1].length == 4);
    REQUIRE(itemA.children()[0].sourceMap[2].location == 16);
    REQUIRE(itemA.children()[0].sourceMap[2].length == 8);
    REQUIRE(itemA.children()[0].sourceMap[3].location == 28);
    REQUIRE(itemA.children()[0].sourceMap[3].length == 4);
    
    ASTNode& list2 = itemA.children()[1];
    REQUIRE(list2.type == ListASTNodeType);
    REQUIRE(list2.text.empty());
    REQUIRE(list2.data == 0);
    REQUIRE(list2.children().size() == 2);
    REQUIRE(list2.sourceMap.size() == 3);
    REQUIRE(list2.sourceMap[0].location == 8);
    REQUIRE(list2.sourceMap[0].length == 4);
    REQUIRE(list2.sourceMap[1].location == 16);
    REQUIRE(list2.sourceMap[1].length == 8);
    REQUIRE(list2.sourceMap[2].location == 28);
    REQUIRE(list2.sourceMap[2].length == 4);
    
    // List Item B
    ASTNode itemB = list2.children()[0];
    REQUIRE(itemB.type == ListItemASTNodeType);
    REQUIRE(itemB.text.empty());
    REQUIRE(itemB.data == 0);
    REQUIRE(itemB.children().size() == 2);
    REQUIRE(itemB.sourceMap.size() == 2);
    REQUIRE(itemB.sourceMap[0].location == 10);
    REQUIRE(itemB.sourceMap[0].length == 2);
    REQUIRE(itemB.sourceMap[1].location == 20);
    REQUIRE(itemB.sourceMap[1].length == 4);
    
    REQUIRE(itemB.children()[0].type == ParagraphASTNodeType);
    REQUIRE(itemB.children()[0].text == "B\n");
    REQUIRE(itemB.children()[0].data == 0);
    REQUIRE(itemB.children()[0].children().empty());
    REQUIRE(itemB.children()[0].sourceMap[0].location == 10);
    REQUIRE(itemB.children()[0].sourceMap[0].length == 2);
    REQUIRE(itemB.children()[0].sourceMap[1].location == 20);
    REQUIRE(itemB.children()[0].sourceMap[1].length == 4);
    
    ASTNode& list3 = itemB.children()[1];
    REQUIRE(list3.type == ListASTNodeType);
    REQUIRE(list3.text.empty());
    REQUIRE(list3.data == 0);
    REQUIRE(list3.children().size() == 1);
    REQUIRE(list3.sourceMap.size() == 1);
    REQUIRE(list3.sourceMap[0].location == 20);
    REQUIRE(list3.sourceMap[0].length == 4);

    // List Item C
    ASTNode itemC = list3.children()[0];
    REQUIRE(itemC.type == ListItemASTNodeType);
    REQUIRE(itemC.text.empty());
    REQUIRE(itemC.data == 0);
    REQUIRE(itemC.children().size() == 1);
    REQUIRE(itemC.sourceMap.size() == 1);
    REQUIRE(itemC.sourceMap[0].location == 22);
    REQUIRE(itemC.sourceMap[0].length == 2);
    
    REQUIRE(itemC.children()[0].type == ParagraphASTNodeType);
    REQUIRE(itemC.children()[0].text == "C\n");
    REQUIRE(itemC.children()[0].data == 0);
    REQUIRE(itemC.children()[0].children().empty());
    REQUIRE(itemC.children()[0].sourceMap.size() == 1);
    REQUIRE(itemC.children()[0].sourceMap[0].location == 22);
    REQUIRE(itemC.children()[0].sourceMap[0].length == 2);
    
    // List Item D
    ASTNode itemD = list2.children()[1];
    REQUIRE(itemD.type == ListItemASTNodeType);
    REQUIRE(itemD.text.empty());
    REQUIRE(itemD.data == 0);
    REQUIRE(itemD.children().size() == 1);
    REQUIRE(itemD.sourceMap.size() == 1);
    REQUIRE(itemD.sourceMap[0].location == 30);
    REQUIRE(itemD.sourceMap[0].length == 2);

    REQUIRE(itemD.children()[0].type == ParagraphASTNodeType);
    REQUIRE(itemD.children()[0].text == "D\n");
    REQUIRE(itemD.children()[0].data == 0);
    REQUIRE(itemD.children()[0].children().empty());
    REQUIRE(itemD.children()[0].sourceMap.size() == 1);
    REQUIRE(itemD.children()[0].sourceMap[0].location == 30);
    REQUIRE(itemD.children()[0].sourceMap[0].length == 2);
    
    // List Item E
    ASTNode itemE = list1.children()[1];
    REQUIRE(itemE.type == ListItemASTNodeType);
    REQUIRE(itemE.text.empty());
    REQUIRE(itemE.data == 0);
    REQUIRE(itemE.children().size() == 1);
    REQUIRE(itemE.sourceMap.size() == 1);
    REQUIRE(itemE.sourceMap[0].location == 34);
    REQUIRE(itemE.sourceMap[0].length == 2);
    
    REQUIRE(itemE.children()[0].type == ParagraphASTNodeType);
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
    ASTNode ast;
    
    ByteBuffer src =\
    "- A\n"\
    "\n"\
    "    C\n"\
    "    D\n"\
    "\n"\
    "    E\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 24);
    
    ASTNode node = ast.children()[0];
    REQUIRE(node.type == ListASTNodeType);
    REQUIRE(node.text.empty());
    REQUIRE(node.data == 2);
    REQUIRE(node.children().size() == 1);
    REQUIRE(node.sourceMap.size() == 1);
    REQUIRE(node.sourceMap[0].location == 0);
    REQUIRE(node.sourceMap[0].length == 24);
    
    node = node.children()[0];
    REQUIRE(node.type == ListItemASTNodeType);
    REQUIRE(node.text.empty());
    REQUIRE(node.data == 2);
    REQUIRE(node.children().size() == 3);
    REQUIRE(node.sourceMap.size() == 4);
    REQUIRE(node.sourceMap[0].location == 2);
    REQUIRE(node.sourceMap[0].length == 3);
    REQUIRE(node.sourceMap[1].location == 9);
    REQUIRE(node.sourceMap[1].length == 2);
    REQUIRE(node.sourceMap[2].location == 15);
    REQUIRE(node.sourceMap[2].length == 3);
    REQUIRE(node.sourceMap[3].location == 22);
    REQUIRE(node.sourceMap[3].length == 2);
    
    ASTNode& p1 = node.children()[0];
    REQUIRE(p1.type == ParagraphASTNodeType);
    REQUIRE(p1.text == "A");
    REQUIRE(p1.data == 0);
    REQUIRE(p1.children().empty());
    REQUIRE(p1.sourceMap.size() == 1);
    REQUIRE(p1.sourceMap[0].location == 2);
    REQUIRE(p1.sourceMap[0].length == 3);
    
    ASTNode& p2 = node.children()[1];
    REQUIRE(p2.type == ParagraphASTNodeType);
    REQUIRE(p2.text == "C\nD");
    REQUIRE(p2.data == 0);
    REQUIRE(p2.children().empty());
    REQUIRE(p2.sourceMap.size() == 2);
    REQUIRE(p2.sourceMap[0].location == 9);
    REQUIRE(p2.sourceMap[0].length == 2);
    REQUIRE(p2.sourceMap[1].location == 15);
    REQUIRE(p2.sourceMap[1].length == 3);
    
    ASTNode& p3 = node.children()[2];
    REQUIRE(p3.type == ParagraphASTNodeType);
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
    ASTNode ast;
    
    ByteBuffer src = "> quote\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootASTNode);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 8);
    
    ASTNode& quote = ast.children()[0];
    REQUIRE(quote.type == QuoteASTNodeType);
    REQUIRE(quote.text.empty());
    REQUIRE(quote.data == 0);
    REQUIRE(quote.children().size() == 1);
    REQUIRE(quote.sourceMap.size() == 1);
    REQUIRE(quote.sourceMap[0].location == 0);
    REQUIRE(quote.sourceMap[0].length == 8);
    
    ASTNode& para = quote.children()[0];
    REQUIRE(para.type == ParagraphASTNodeType);
    REQUIRE(para.text == "quote");
    REQUIRE(para.data == 0);
    REQUIRE(para.children().empty());
    REQUIRE(para.sourceMap.size() == 1);
    REQUIRE(para.sourceMap[0].location == 2);
    REQUIRE(para.sourceMap[0].length == 6);
}

// TODO:
// Move to separate test file
//TEST_CASE("Multi-byte characters Czech", "[parser][sourcemap][bytbuffer]")
//{
//    MarkdownParser parser;
//    ASTNode ast;
//    
//    ByteBuffer src = "\x50\xC5\x99\xC3\xAD\xC5\xA1\x65\x72\x6E\xC4\x9B\x20\xC5\xBE\x6C\x75\xC5\xA5\x6F\x75\xC4\x8D\x6B\xC3\xBD\x20\x6B\xC5\xAF\xC5\x88\x20\xC3\xBA\x70\xC4\x9B\x6C\x20\xC4\x8F\xC3\xA1\x62\x65\x6C\x73\x6B\xC3\xA9\x20\xC3\xB3\x64\x79\n";
//
//    parser.parse(src, ast);
//    
//    REQUIRE(ast.type == RootASTNode);
//    REQUIRE(ast.text.empty());
//    REQUIRE(ast.data == 0);
//    REQUIRE(ast.children().size() == 1);
//    REQUIRE(ast.sourceMap.size() == 1);
//    REQUIRE(ast.sourceMap[0].location == 0);
//    REQUIRE(ast.sourceMap[0].length == 57);
//    
//    CharactersRangeSet charMap = BytesToCharactersSet(ast.sourceMap, src);
//    REQUIRE(charMap.size() == 1);
//    REQUIRE(charMap[0].location == 0);
//    REQUIRE(charMap[0].length == 41);
//    
//    ByteBuffer mappedBuffer = MapBytesRangeSet(ast.sourceMap, src);
//    REQUIRE(mappedBuffer == src);
//}
