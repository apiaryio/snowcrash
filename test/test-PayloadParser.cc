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

//TEST_CASE("payloadparser/parse", "Parse payload")
//{
//    SourceData source = "01234567";
//    
//    // Blueprint in question:
//    //R"(
//    //+ Payload
//    //
//    //  Description
//    //    + Body
//    //
//    //            Code
//    //
//    //)";
//
//    // Body list item
//    MarkdownBlock bodyListItem(ListItemBlockType, SourceData(), 0, MakeSourceDataBlock(0, 1));
//    bodyListItem.blocks.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(1, 1)));
//    bodyListItem.blocks.push_back(MarkdownBlock(ParagraphBlockType, "Description", 0, MakeSourceDataBlock(2, 1)));
//    bodyListItem.blocks.push_back(MarkdownBlock(CodeBlockType, "Code", 0, MakeSourceDataBlock(3, 1)));
//
//    // Body list
//    MarkdownBlock bodyList(ListBlockType, SourceData(), 0, MakeSourceDataBlock(4, 1));
//    bodyList.blocks.push_back(bodyListItem);
//    
//    // Payload list item
//    MarkdownBlock payloadListItem(ListItemBlockType, SourceData(), 0, MakeSourceDataBlock(5, 1));
//    payloadListItem.blocks.push_back(MarkdownBlock(CodeBlockType, "Payload", 0, MakeSourceDataBlock(6, 1)));
//    payloadListItem.blocks.push_back(bodyList);
//
//    // Payload markdown
//    MarkdownBlock markdown(ListBlockType, SourceData(), 0, MakeSourceDataBlock(7, 1));
//    markdown.blocks.push_back(payloadListItem);
//    
//    Payload payload;
//    ParseSectionResult result = ParsePayload(markdown.blocks.begin(), markdown.blocks.end(), source, payload);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//    
//    const MarkdownBlock::Stack &blocks = markdown.blocks;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 1);
//
//    REQUIRE(payload.name == "Payload");
//    REQUIRE(payload.description == "2");
//    REQUIRE(payload.parameters.size() == 0);
//    REQUIRE(payload.headers.size() == 0);
//    REQUIRE(payload.body == "Code");
//    REQUIRE(payload.schema.empty());
//}
