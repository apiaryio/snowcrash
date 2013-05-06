//
//  test-BlueprintParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "catch.hpp"
#include "BlueprintParser.h"

using namespace snowcrash;

TEST_CASE("bpparser/init", "Blueprint parser construction")
{
    BlueprintParser* parser;
    REQUIRE_NOTHROW(parser = ::new BlueprintParser);
    REQUIRE_NOTHROW(::delete parser);
}

TEST_CASE("bpparser/parse-params", "parse() method parameters.")
{
    BlueprintParser parser;
    Result result;
    Blueprint blueprint;
    
    parser.parse("", MarkdownBlock(), result, blueprint);
    REQUIRE(result.error.code == Error::OK);
}

TEST_CASE("bpparser/parse-bp-name", "Parse blueprint name.")
{
    BlueprintParser parser;
    Result result;
    Blueprint blueprint;
    
    MarkdownBlock markdown;
    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "API Name", 1));
    
    parser.parse("", markdown, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(blueprint.name == "API Name");
    REQUIRE(blueprint.description.length() == 0);
}

TEST_CASE("bpparser/parse-overview", "Parse blueprint overview section.")
{
    BlueprintParser parser;
    Result result;
    Blueprint blueprint;
    
    SourceData source = "0123";
    MarkdownBlock markdown;
    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "API Name", 1, MakeSourceDataBlock(0, 1)));
    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "Overview Header", 2, MakeSourceDataBlock(2, 1)));
    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p2", 0, MakeSourceDataBlock(3, 1)));
    
    parser.parse(source, markdown, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(blueprint.name == "API Name");
    REQUIRE(blueprint.description == "123");
    REQUIRE(blueprint.resourceGroups.empty());
}

TEST_CASE("bpparser/parse-group", "Parse resource group.")
{
    BlueprintParser parser;
    Result result;
    Blueprint blueprint;
    
    SourceData source = "0123456";
    MarkdownBlock markdown;
    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "API Name", 1, MakeSourceDataBlock(0, 1)));
    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.blocks.push_back(MarkdownBlock(HRuleBlockType, MarkdownBlock::Content(), 0, MakeSourceDataBlock(2, 1)));
    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "Group Name", 1, MakeSourceDataBlock(3, 1)));
    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p2", 0, MakeSourceDataBlock(4, 1)));
    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "Group Description Header", 2, MakeSourceDataBlock(5, 1)));
    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p3", 1, MakeSourceDataBlock(6, 1)));
    
    parser.parse(source, markdown, result, blueprint);

    REQUIRE(result.error.code == Error::OK);
    REQUIRE(blueprint.resourceGroups.size() == 1);
    
    ResourceGroup group = blueprint.resourceGroups.front();
    REQUIRE(group.name == "Group Name");
    REQUIRE(group.description == "456");
}

TEST_CASE("bpparser/parse-name-resource", "Parse API with Name and resouce")
{
    // Blueprint in question:
    //R"(# API
    //A
    //
    //# /resource
    //B)";

    BlueprintParser parser;
    Result result;
    Blueprint blueprint;
    
    SourceData source = "0123";
    MarkdownBlock markdown;
    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "API Name", 1, MakeSourceDataBlock(0, 1)));
    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "/resource", 1, MakeSourceDataBlock(2, 1)));
    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p2", 0, MakeSourceDataBlock(3, 1)));
    
    parser.parse(source, markdown, result, blueprint);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(blueprint.resourceGroups.size() == 1);
    
    ResourceGroup group = blueprint.resourceGroups.front();
    REQUIRE(group.name.empty());
    REQUIRE(group.description.empty());
    REQUIRE(group.resources.size() == 1);
    
    Resource resource = group.resources.front();
    REQUIRE(resource.uri == "/resource");
    REQUIRE(resource.description == "3");
}

TEST_CASE("bpparser/parse-nameless-description", "Parse nameless blueprint description")
{
    // Blueprint in question:
    //R"(A
    //# B
    //");
    
    BlueprintParser parser;
    Result result;
    Blueprint blueprint;
    
    SourceData source = "01";
    MarkdownBlock markdown;
    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "A", 1, MakeSourceDataBlock(0, 1)));
    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "B", 0, MakeSourceDataBlock(1, 1)));
    
    parser.parse(source, markdown, result, blueprint);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(blueprint.resourceGroups.size() == 0);
    
    REQUIRE(blueprint.description == "01");
}

TEST_CASE("bpparser/parse-list-only", "Parse nameless blueprint with a list description")
{
    // Blueprint in question:
    //R"(+ list
    //");
    
    BlueprintParser parser;
    Result result;
    Blueprint blueprint;
    
    SourceData source = "01";
    
    MarkdownBlock listItem(ListItemBlockType, "list", 0, MakeSourceDataBlock(0, 1));
    MarkdownBlock list(ListBlockType, SourceData(), 0, MakeSourceDataBlock(0, 2));
    list.blocks.push_back(listItem);
    
    MarkdownBlock markdown;
    markdown.blocks.push_back(list);
    
    parser.parse(source, markdown, result, blueprint);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(blueprint.resourceGroups.size() == 0);
    REQUIRE(blueprint.description == "01");
}

//TEST_CASE("bpparser/parse-resource", "Parse simple resource.")
//{
//    BlueprintParser parser;
//    Result result;
//    Blueprint blueprint;
//    
//    SourceData source = "0";
//
//    MarkdownBlock listItem(ListItemBlockType);
//    listItem.blocks.push_back(MarkdownBlock(ParagraphBlockType, "Response 200"));
//    listItem.blocks.push_back(MarkdownBlock(CodeBlockType, "body"));
//    
//    MarkdownBlock list(ListBlockType);
//    list.blocks.push_back(listItem);
//    
//    MarkdownBlock markdown;
//    markdown.blocks.push_back(MarkdownBlock(HeaderBlockType, "GET /resource", 1));
//    markdown.blocks.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
//    markdown.blocks.push_back(list);
//    
//    parser.parse(source, markdown, result, blueprint);
//
//    REQUIRE(result.error.code == Error::OK);
//    REQUIRE(blueprint.name.empty());
//    REQUIRE(blueprint.description.empty());
//    
//    REQUIRE(blueprint.resourceGroups.size() == 1);
//
//    ResourceGroup group = blueprint.resourceGroups.front();
//    REQUIRE(group.name.empty());
//    REQUIRE(group.description.empty());
//    REQUIRE(group.resources.size() == 1);
//    
//    Resource resource = group.resources.front();
//    REQUIRE(resource.uri == "/resource");
//    REQUIRE(resource.description == "0");
//    REQUIRE(resource.headers.empty());
//    REQUIRE(resource.parameters.empty());
//    REQUIRE(resource.methods.size() == 1);
//    
//    Method method = resource.methods.front();
//    REQUIRE(method.method == "GET");
//    REQUIRE(method.description.empty());
//    REQUIRE(method.parameters.empty());
//    REQUIRE(method.headers.empty());
//    REQUIRE(method.requests.empty());
//    REQUIRE(method.responses.size() == 1);
//    
//    Response response = method.responses.front();
//    REQUIRE(response.name == "200");
//    REQUIRE(response.description.empty());
//    REQUIRE(response.parameters.empty());
//    REQUIRE(response.headers.empty());
//    REQUIRE(response.body == "body");
//    REQUIRE(response.schema.empty());
//}


