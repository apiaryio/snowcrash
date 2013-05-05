//
//  test-ResouceGroupParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "ResourceGroupParser.h"

using namespace snowcrash;

//TEST_CASE("resourcegroupparser/parse-resource-method", "Parse resource group section terminated by resource with method")
//{
//    SourceData source = "012";
//    MarkdownBlock markdown;
//    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "Group Name", 1, MakeSourceDataBlock(0, 1)));
//    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
//    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "GET /resource", 1, MakeSourceDataBlock(2, 1)));
//    
//    ResourceGroup resourceGroup;
//    ParseSectionResult result = ParseResourceGroup(markdown.blocks.begin(), markdown.blocks.end(), source, resourceGroup);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//    
//    const MarkdownBlock::Stack &blocks = markdown.blocks;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
//    
//    REQUIRE(resourceGroup.name == "Group Name");
//    REQUIRE(resourceGroup.description == "1");
//    REQUIRE(resourceGroup.resources.empty());
//}

TEST_CASE("resourcegroupparser/parse-resource", "Parse resource group section terminated by a resource")
{
    SourceData source = "012";
    MarkdownBlock markdown;
    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "Group Name", 1, MakeSourceDataBlock(0, 1)));
    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "/resource", 1, MakeSourceDataBlock(2, 1)));
    
    ResourceGroup resourceGroup;
    ParseSectionResult result = ParseResourceGroup(markdown.blocks.begin(), markdown.blocks.end(), source, Blueprint(), resourceGroup);
    
    REQUIRE(result.first.error.code == Error::OK);
    
    const MarkdownBlock::Stack &blocks = markdown.blocks;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(resourceGroup.name == "Group Name");
    REQUIRE(resourceGroup.description == "1");
    REQUIRE(resourceGroup.resources.size() == 1);
    REQUIRE(resourceGroup.resources.front().uri == "/resource");
}
