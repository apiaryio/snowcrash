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

TEST_CASE("Character index")
{
    // $¢€𐍈 (byte length - 1, 2, 3, 4, 1)
    ByteBuffer src = "\x24\xc2\xa2\xe2\x82\xac\xf0\x90\x8d\x88\n";
    ByteBufferCharacterIndex index;

    mdp::BuildCharacterIndex(index, src);

    REQUIRE(index.size() == src.length());

    REQUIRE(index.size() == 11);

    // '$' byte length - 1
    REQUIRE(index[0] == 0);

    // ¢ byte length - 2
    REQUIRE(index[1] == 1);
    REQUIRE(index[2] == 1);

    // € byte length - 3
    REQUIRE(index[3] == 2);
    REQUIRE(index[4] == 2);
    REQUIRE(index[5] == 2);

    // 𐍈 byte length - 4
    REQUIRE(index[6] == 3);
    REQUIRE(index[7] == 3);
    REQUIRE(index[8] == 3);
    REQUIRE(index[9] == 3);

    // \n byte length - 1
    REQUIRE(index[10] == 4);
}

TEST_CASE("Byte buffer and Index should provide equal information", "[bytebuffer][sourcemap]")
{
    MarkdownParser parser;
    MarkdownNode ast;

    ByteBuffer src = "\x50\xC5\x99\xC3\xAD\xC5\xA1\x65\x72\x6E\xC4\x9B\x20\xC5\xBE\x6C\x75\xC5\xA5\x6F\x75\xC4\x8D\x6B\xC3\xBD\x20\x6B\xC5\xAF\xC5\x88\x20\xC3\xBA\x70\xC4\x9B\x6C\x20\xC4\x8F\xC3\xA1\x62\x65\x6C\x73\x6B\xC3\xA9\x20\xC3\xB3\x64\x79\n";

    ByteBufferCharacterIndex index;
    mdp::BuildCharacterIndex(index, src);

    parser.parse(src, ast);

    CharactersRangeSet charMap = BytesRangeSetToCharactersRangeSet(ast.sourceMap, src);
    CharactersRangeSet indexMap = BytesRangeSetToCharactersRangeSet(ast.sourceMap, index);

    REQUIRE(charMap.size() == 1);
    REQUIRE(charMap[0].location == 0);
    REQUIRE(charMap[0].length == 41);

    REQUIRE(indexMap.size() == charMap.size());
    REQUIRE(indexMap[0].location == charMap[0].location);
    REQUIRE(charMap[0].length == charMap[0].length);
}

TEST_CASE("Byte buffer and Index should return equal char ranges", "[bytebuffer][sourcemap]")
{
    //          bytes:0123   4   5678901   2   3   4
    //          chars:0123       4567890           1
    ByteBuffer src = "19 \xc2\xa2 & 20 \xe2\x82\xac\n";

    ByteBufferCharacterIndex index;
    mdp::BuildCharacterIndex(index, src);

    BytesRangeSet byteMap;
    byteMap.push_back(Range(1,4));
    byteMap.push_back(Range(10,4));
    byteMap.push_back(Range(3,7));
    byteMap.push_back(Range(0,14));
    byteMap.push_back(Range(0,30));

    CharactersRangeSet charMap = BytesRangeSetToCharactersRangeSet(byteMap, src);
    CharactersRangeSet indexMap = BytesRangeSetToCharactersRangeSet(byteMap, index);

    REQUIRE(charMap[0].location == indexMap[0].location);
    REQUIRE(charMap[0].length == indexMap[0].length);

    REQUIRE(charMap[1].location == indexMap[1].location);
    REQUIRE(charMap[1].length == indexMap[1].length);

    REQUIRE(charMap[2].location == indexMap[2].location);
    REQUIRE(charMap[2].length == indexMap[2].length);

    REQUIRE(charMap[3].location == indexMap[3].location);
    REQUIRE(charMap[3].length == indexMap[3].length);

    REQUIRE(charMap[4].location == indexMap[4].location);
    REQUIRE(charMap[4].length == indexMap[4].length);
}
