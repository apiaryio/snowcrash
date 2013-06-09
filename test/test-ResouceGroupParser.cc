//
//  test-ResouceGroupParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <iterator>
#include "Fixture.h"
#include "catch.hpp"
#include "ResourceGroupParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("rgparser/parse", "Parse resource group with empty resource")
{
    SourceData source = "012";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Group Name", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/resource", 1, MakeSourceDataBlock(2, 1)));
    
    ResourceGroup resourceGroup;
    BlueprinParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = ResourceGroupParser::Parse(markdown.begin(), markdown.end(), parser, resourceGroup);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(resourceGroup.name == "Group Name");
    REQUIRE(resourceGroup.description == "1");
    REQUIRE(resourceGroup.resources.size() == 1);
    REQUIRE(resourceGroup.resources.front().uri == "/resource");
}

TEST_CASE("rgparser/parse-multiple-resource-description", "Parse multiple resource in anonymous group")
{
    SourceData source = "0123";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/r1", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/r2", 1, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p2", 0, MakeSourceDataBlock(3, 1)));
    
    ResourceGroup resourceGroup;
    BlueprinParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = ResourceGroupParser::Parse(markdown.begin(), markdown.end(), parser, resourceGroup);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 4);
    
    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 2);
    REQUIRE(resourceGroup.resources[0].uri == "/r1");
    REQUIRE(resourceGroup.resources[0].description == "1");
    REQUIRE(resourceGroup.resources[1].uri == "/r2");
    REQUIRE(resourceGroup.resources[1].description == "3");
}

TEST_CASE("rgparser/parse-multiple-resource", "Parse multiple resources with payloads")
{
    // Blueprint in question:
    //R"(
    //# /1
    //## GET
    //+ request
    //
    //# /2
    //## GET
    //+ request
    //");
    
    SourceData source = "01234567";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/1", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 2, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "request", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/2", 1, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 2, MakeSourceDataBlock(5, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "request", 0, MakeSourceDataBlock(6, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(7, 1)));
    
    ResourceGroup resourceGroup;
    BlueprinParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = ResourceGroupParser::Parse(markdown.begin(), markdown.end(), parser, resourceGroup);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 4); // 2x no response specified + 2x empty body asset
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 12);
    
    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 2);
    
    Resource resource1 = resourceGroup.resources[0];
    REQUIRE(resource1.uri == "/1");
    REQUIRE(resource1.description.empty());
    REQUIRE(resource1.methods.size() == 1);
    REQUIRE(resource1.methods[0].method == "GET");
    REQUIRE(resource1.methods[0].description.empty());
    REQUIRE(resource1.methods[0].requests.size() == 1);
    REQUIRE(resource1.methods[0].requests[0].name.empty());
    REQUIRE(resource1.methods[0].requests[0].description.empty());
    REQUIRE(resource1.methods[0].requests[0].body.empty());
    REQUIRE(resource1.methods[0].responses.empty());

    Resource resource2 = resourceGroup.resources[1];
    REQUIRE(resource2.uri == "/2");
    REQUIRE(resource2.description.empty());
    REQUIRE(resource2.methods.size() == 1);
    REQUIRE(resource2.methods[0].method == "GET");
    REQUIRE(resource2.methods[0].description.empty());
    REQUIRE(resource2.methods[0].requests[0].name.empty());
    REQUIRE(resource2.methods[0].requests[0].description.empty());
    REQUIRE(resource2.methods[0].requests[0].body.empty());
    REQUIRE(resource2.methods[0].responses.empty());
}

TEST_CASE("rgparser/parse-multiple-same", "Parse multiple same resources")
{
    SourceData source = "01";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/r1", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/r1", 1, MakeSourceDataBlock(1, 1)));
    
    ResourceGroup resourceGroup;
    BlueprinParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = ResourceGroupParser::Parse(markdown.begin(), markdown.end(), parser, resourceGroup);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 2);
}

TEST_CASE("rgparser/parse-resource-description-list", "Parse resource with list in its description")
{
    
    // Blueprint in question:
    //R"(
    //# /1
    //# GET
    //+ Request
    //Hello
    //+ Body
    
    SourceData source = "01234";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/1", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request\nHello\n", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Body\n", 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
    
    ResourceGroup resourceGroup;
    BlueprinParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = ResourceGroupParser::Parse(markdown.begin(), markdown.end(), parser, resourceGroup);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 3);   // preformatted asset & ignoring unrecognized body & no response
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 8);
    
    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 1);
    REQUIRE(resourceGroup.resources[0].uri == "/1");
    REQUIRE(resourceGroup.resources[0].description.empty());
    REQUIRE(resourceGroup.resources[0].methods.size() == 1);
    REQUIRE(resourceGroup.resources[0].methods[0].method == "GET");
    REQUIRE(resourceGroup.resources[0].methods[0].description == "");
}

TEST_CASE("rgparser/parse-terminator", "Parse resource groups finalized by terminator")
{
    
    // Blueprint in question:
    //R"(
    //# 1
    //---
    //A
    
    SourceData source = "01234";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "1", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HRuleBlockType, SourceData(), 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "A", 0, MakeSourceDataBlock(2, 1)));
    
    ResourceGroup resourceGroup;
    BlueprinParserCore parser(0, SourceDataFixture, Blueprint());   
    ParseSectionResult result = ResourceGroupParser::Parse(markdown.begin(), markdown.end(), parser, resourceGroup);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 2);
    
    REQUIRE(resourceGroup.name == "1");
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.empty());
}

