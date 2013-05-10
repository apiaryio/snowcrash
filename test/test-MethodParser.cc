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

TEST_CASE("mparser/parse", "Parse method")
{
    SourceData source = "01";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    
    Method method;
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), method);
    
    REQUIRE(result.first.error.code == Error::OK);

    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 2);
    
    REQUIRE(method.method == "GET");
    REQUIRE(method.description == "1");
}
