//
//  test-ByteBuffer.cc
//  markdownparser
//
//  Created by Zdenek Nemec on 4/22/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "catch.hpp"
#include "MarkdownParser.h"

using namespace mdp;

TEST_CASE("Multi-byte characters Czech", "[bytebuffer][sourcemap]")
{
    MarkdownParser parser;
    MarkdownNode ast;

    ByteBuffer src = "\x50\xC5\x99\xC3\xAD\xC5\xA1\x65\x72\x6E\xC4\x9B\x20\xC5\xBE\x6C\x75\xC5\xA5\x6F\x75\xC4\x8D\x6B\xC3\xBD\x20\x6B\xC5\xAF\xC5\x88\x20\xC3\xBA\x70\xC4\x9B\x6C\x20\xC4\x8F\xC3\xA1\x62\x65\x6C\x73\x6B\xC3\xA9\x20\xC3\xB3\x64\x79\n";

    parser.parse(src, ast);

    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 57);

    CharactersRangeSet charMap = BytesRangeSetToCharactersRangeSet(ast.sourceMap, src);
    REQUIRE(charMap.size() == 1);
    REQUIRE(charMap[0].location == 0);
    REQUIRE(charMap[0].length == 41);

    ByteBuffer mappedBuffer = MapBytesRangeSet(ast.sourceMap, src);
    REQUIRE(mappedBuffer == src);
}

TEST_CASE("Multi-byte characters in blockquote", "[bytebuffer][sourcemap]")
{
    MarkdownParser parser;
    MarkdownNode ast;

    // "> Ni Hao"
    ByteBuffer src = "> \xE4\xBD\xA0\xE5\xA5\xBD\n";
    
    parser.parse(src, ast);
    
    REQUIRE(ast.type == RootMarkdownNodeType);
    REQUIRE(ast.text.empty());
    REQUIRE(ast.data == 0);
    REQUIRE(ast.children().size() == 1);
    REQUIRE(ast.sourceMap.size() == 1);
    REQUIRE(ast.sourceMap[0].location == 0);
    REQUIRE(ast.sourceMap[0].length == 9);
    
    MarkdownNode& quote = ast.children()[0];
    REQUIRE(quote.type == QuoteMarkdownNodeType);
    REQUIRE(quote.children().size() == 1);
    REQUIRE(quote.sourceMap.size() == 1);
    REQUIRE(quote.sourceMap[0].location == 0);
    REQUIRE(quote.sourceMap[0].length == 9);
    
    MarkdownNode& para = quote.children()[0];
    REQUIRE(para.type == ParagraphMarkdownNodeType);
    REQUIRE(para.sourceMap.size() == 1);
    REQUIRE(para.sourceMap[0].location == 2);
    REQUIRE(para.sourceMap[0].length == 7);

    ByteBuffer mappedBuffer = MapBytesRangeSet(para.sourceMap, src);
    REQUIRE(mappedBuffer == "\xE4\xBD\xA0\xE5\xA5\xBD\n");

    CharactersRangeSet charMap = BytesRangeSetToCharactersRangeSet(quote.sourceMap, src);
    REQUIRE(charMap.size() == 1);
    REQUIRE(charMap[0].location == 0);
    REQUIRE(charMap[0].length == 5);
    
    charMap = BytesRangeSetToCharactersRangeSet(para.sourceMap, src);
    REQUIRE(charMap.size() == 1);
    REQUIRE(charMap[0].location == 2);
    REQUIRE(charMap[0].length == 3);
}
