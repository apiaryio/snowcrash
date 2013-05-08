//
//  test-MethodParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/6/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "MethodParser.h"

using namespace snowcrash;
//
//TEST_CASE("methodparser/parse", "Parse method")
//{
//    SourceData source = "01";
//    MarkdownBlock markdown;
//    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(0, 1)));
//    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
//    
//    Method method;
//    ParseSectionResult result = ParseMethod(markdown.blocks.begin(), markdown.blocks.end(), source, method);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//
//    const MarkdownBlock::Stack &blocks = markdown.blocks;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 2);
//    
//    REQUIRE(method.method == "GET");
//    REQUIRE(method.description == "1");
//}
