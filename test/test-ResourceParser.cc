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
    SourceData source = "0123";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/resource", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(2, 1)));
    
    Resource resource;
    ParseSectionResult result = ParseResource(markdown.begin(), markdown.end(), source, resource);
    
    REQUIRE(result.first.error.code == Error::OK);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(resource.uri == "/resource");
    REQUIRE(resource.description == "1");
    REQUIRE(resource.methods.size() == 1);
    
    REQUIRE(resource.methods.front().method == "GET");
    REQUIRE(resource.methods.front().description.empty());
}

TEST_CASE("resourcepparser/parse-multi", "Parse mutliple methods")
{
    SourceData source = "0123";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/1", 1, MakeSourceDataBlock(0, 1)));    
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "POST", 1, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p2", 0, MakeSourceDataBlock(3, 1)));
    
    Resource resource;
    ParseSectionResult result = ParseResource(markdown.begin(), markdown.end(), source, resource);
    
    REQUIRE(result.first.error.code == Error::OK);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 5);
    
    REQUIRE(resource.uri == "/1");
    REQUIRE(resource.description.empty());
    REQUIRE(resource.methods.size() == 2);
    REQUIRE(resource.methods[0].method == "GET");
    REQUIRE(resource.methods[0].description == "1");
    REQUIRE(resource.methods[1].method == "POST");
    REQUIRE(resource.methods[1].description == "3");
}
