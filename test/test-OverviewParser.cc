//
//  test-OverviewParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "OverviewParser.h"

using namespace snowcrash;

TEST_CASE("overviewparser/parse", "Parse overview section")
{
    SourceData source = "0123";
    MarkdownBlock markdown;
    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "API Name", 1, MakeSourceDataBlock(0, 1)));
    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.blocks.push_back(MarkdownBlock(HRuleBlockType, MarkdownBlock::Content(), 0, MakeSourceDataBlock(2, 1)));
    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p2", 0, MakeSourceDataBlock(3, 1)));
    
    Blueprint blueprint;
    ParseSectionResult result = ParseOverview(markdown.blocks.begin(), markdown.blocks.end(), source, blueprint);
    
    REQUIRE(result.first.error.code == Error::OK);
    
    const MarkdownBlock::Stack &blocks = markdown.blocks;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(blueprint.name == "API Name");
    REQUIRE(blueprint.description == "1");
    REQUIRE(blueprint.resourceGroups.empty());
}
