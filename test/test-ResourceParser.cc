//
//  test-ResourceParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "ResourceParser.h"

using namespace snowcrash;

TEST_CASE("resourcepparser/parse", "Parse resource")
{
    SourceData source = "012";
    MarkdownBlock markdown;
    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "/resource", 1, MakeSourceDataBlock(0, 1)));
    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    
    Resource resource;
    ParseSectionResult result = ParseResource(markdown.blocks.begin(), markdown.blocks.end(), source, resource);
    
    REQUIRE(result.first.error.code == Error::OK);
    
    const MarkdownBlock::Stack &blocks = markdown.blocks;
    REQUIRE(std::distance(blocks.begin(), result.second) == 2);
    
    REQUIRE(resource.uri == "/resource");
    REQUIRE(resource.description == "1");
}
