//
//  test-ResouceGroupParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iterator>
#include "Fixture.h"
#include "catch.hpp"
#include "ResourceGroupParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

snowcrash::MarkdownBlock::Stack snowcrashtest::CanonicalResourceGroupFixture()
{
    // Blueprint in question:
    //R"(
    //# Group First
    //
    //Fiber optics
    //
    //<see CanonicalResourceFixture()>
    //
    //
    //# Group Second
    //
    //Assembly language
    //)";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Group First", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Fiber optics", 0, MakeSourceDataBlock(1, 1)));

    MarkdownBlock::Stack blocks = CanonicalResourceFixture();
    markdown.insert(markdown.end(), blocks.begin(), blocks.end());
    
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Group Second", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Assembly language", 0, MakeSourceDataBlock(2, 1)));
    
    return markdown;
}

TEST_CASE("rgparser/classifier", "Resource Group block classifier")
{
    MarkdownBlock::Stack markdown = CanonicalResourceGroupFixture();

    BlockIterator cur = markdown.begin();

    // # Group First
    REQUIRE(ClassifyBlock<ResourceGroup>(cur, markdown.end(), UndefinedSectionType) == ResourceGroupSectionType);
    REQUIRE(ClassifyBlock<ResourceGroup>(cur, markdown.end(), ResourceGroupSectionType) == UndefinedSectionType);
    
    ++cur; // Fiber optics
    REQUIRE(ClassifyBlock<ResourceGroup>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<ResourceGroup>(cur, markdown.end(), ResourceGroupSectionType) == ResourceGroupSectionType);

    ++cur; // # My Resource [/resource]
    REQUIRE(ClassifyBlock<ResourceGroup>(cur, markdown.end(), UndefinedSectionType) == ResourceSectionType);
    REQUIRE(ClassifyBlock<ResourceGroup>(cur, markdown.end(), ResourceGroupSectionType) == ResourceSectionType);

    std::advance(cur, 42 + 31); // # Group Second
    REQUIRE(ClassifyBlock<ResourceGroup>(cur, markdown.end(), UndefinedSectionType) == ResourceGroupSectionType);
    REQUIRE(ClassifyBlock<ResourceGroup>(cur, markdown.end(), ResourceGroupSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<ResourceGroup>(cur, markdown.end(), ResourceSectionType) == UndefinedSectionType);    
}

TEST_CASE("rgparser/parse", "Parse canonical resource group")
{
    MarkdownBlock::Stack markdown = CanonicalResourceGroupFixture();
    
    ResourceGroup resourceGroup;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceGroupParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resourceGroup);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 44 + 31);
    
    REQUIRE(resourceGroup.name == "First");
    REQUIRE(resourceGroup.description == "1");
    REQUIRE(resourceGroup.resources.size() == 1);
    REQUIRE(resourceGroup.resources.front().uriTemplate == "/resource/{id}{?limit}");
    REQUIRE(resourceGroup.resources.front().name == "My Resource");
}

TEST_CASE("rgparser/parse-empty-resource", "Parse resource group with empty resource")
{
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Group Name", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/resource", 1, MakeSourceDataBlock(2, 1)));
    
    ResourceGroup resourceGroup;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceGroupParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resourceGroup);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(resourceGroup.name == "Name");
    REQUIRE(resourceGroup.description == "1");
    REQUIRE(resourceGroup.resources.size() == 1);
    REQUIRE(resourceGroup.resources.front().uriTemplate == "/resource");
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
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceGroupParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resourceGroup);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 4);
    
    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 2);
    REQUIRE(resourceGroup.resources[0].uriTemplate == "/r1");
    REQUIRE(resourceGroup.resources[0].description == "1");
    REQUIRE(resourceGroup.resources[1].uriTemplate == "/r2");
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
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceGroupParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resourceGroup);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 4); // 2x no response specified + 2x empty body asset
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 12);
    
    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 2);
    
    Resource resource1 = resourceGroup.resources[0];
    REQUIRE(resource1.uriTemplate == "/1");
    REQUIRE(resource1.description.empty());
    REQUIRE(resource1.actions.size() == 1);
    REQUIRE(resource1.actions[0].method == "GET");
    REQUIRE(resource1.actions[0].description.empty());
    REQUIRE(!resource1.actions[0].examples.empty());
    REQUIRE(resource1.actions[0].examples[0].requests.size() == 1);
    REQUIRE(resource1.actions[0].examples[0].requests[0].name.empty());
    REQUIRE(resource1.actions[0].examples[0].requests[0].description.empty());
    REQUIRE(resource1.actions[0].examples[0].requests[0].body.empty());
    REQUIRE(resource1.actions[0].examples[0].responses.empty());

    Resource resource2 = resourceGroup.resources[1];
    REQUIRE(resource2.uriTemplate == "/2");
    REQUIRE(resource2.description.empty());
    REQUIRE(resource2.actions.size() == 1);
    REQUIRE(resource2.actions[0].method == "GET");
    REQUIRE(resource2.actions[0].description.empty());
    REQUIRE(!resource2.actions[0].examples.empty());
    REQUIRE(resource2.actions[0].examples[0].requests[0].name.empty());
    REQUIRE(resource2.actions[0].examples[0].requests[0].description.empty());
    REQUIRE(resource2.actions[0].examples[0].requests[0].body.empty());
    REQUIRE(resource2.actions[0].examples[0].responses.empty());
}

TEST_CASE("rgparser/parse-multiple-same", "Parse multiple resources with the same name")
{
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/r1", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/r1", 1, MakeSourceDataBlock(1, 1)));
    
    ResourceGroup resourceGroup;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceGroupParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resourceGroup);
    
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
    //   Hello
    //+ Body
    
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
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceGroupParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resourceGroup);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 3);   // preformatted asset & ignoring unrecognized body & no response
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 8);
    
    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 1);
    REQUIRE(resourceGroup.resources[0].uriTemplate == "/1");
    REQUIRE(resourceGroup.resources[0].description.empty());
    REQUIRE(resourceGroup.resources[0].actions.size() == 1);
    REQUIRE(resourceGroup.resources[0].actions[0].method == "GET");
    REQUIRE(resourceGroup.resources[0].actions[0].description == "");
}

TEST_CASE("rgparser/parse-hr", "Parse resource groups with hr in description")
{
    
    // Blueprint in question:
    //R"(
    //# Group 1
    //---
    //A
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Group 1", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HRuleBlockType, SourceData(), 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "A", 0, MakeSourceDataBlock(2, 1)));
    
    ResourceGroup resourceGroup;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceGroupParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resourceGroup);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(resourceGroup.name == "1");
    REQUIRE(resourceGroup.description == "12");
    REQUIRE(resourceGroup.resources.empty());
}

