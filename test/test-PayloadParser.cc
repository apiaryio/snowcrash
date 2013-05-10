//
//  test-PayloadParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/7/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "PayloadParser.h"

using namespace snowcrash;

TEST_CASE("pld/parse-request", "Parse request payload")
{
    SourceData source = "012345678";
    
    // Blueprint in question:
    //R"(
    //+ Payload
    //
    //  Description
    //    + Body
    //
    //            Code
    //
    //)";
        
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Description", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(7, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(8, 1)));
    
    Payload payload;
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 1);

    REQUIRE(payload.name == "Payload");
    REQUIRE(payload.description == "11");
    REQUIRE(payload.parameters.size() == 0);
    REQUIRE(payload.headers.size() == 0);
    REQUIRE(payload.body == "Code");
    REQUIRE(payload.schema.empty());
}
