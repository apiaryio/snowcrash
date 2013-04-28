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
    REQUIRE_NOTHROW(parser.parse("", MarkdownBlock(), nullptr));
    
    bool didEnterCallback = false;
    parser.parse("", MarkdownBlock(), [&](const Result& report, const Blueprint& blueprint){
        didEnterCallback = true;
        
        REQUIRE(report.error.code == Error::OK);
    });
    REQUIRE(didEnterCallback);
}

TEST_CASE("bpparser/parse-bp-name", "Parse blueprint name.")
{
    BlueprintParser parser;
    MarkdownBlock markdown;
    markdown.blocks = { MarkdownBlock(MarkdownBlockType::Header, "API Name", 1) };
    
    bool didEnterCallback = false;
    parser.parse("", markdown, [&](const Result& report, const Blueprint& blueprint){
        didEnterCallback = true;
        
        REQUIRE(report.error.code == Error::OK);
        REQUIRE(blueprint.name == "API Name");
        REQUIRE(blueprint.description.length() == 0);
        
    });
    REQUIRE(didEnterCallback);
}

TEST_CASE("bpparser/parse-overview", "Parse blueprint overview section.")
{
    BlueprintParser parser;
    SourceData source = "0123";
    MarkdownBlock markdown;
    markdown.blocks = { MarkdownBlock(MarkdownBlockType::Header, "API Name", 1, MakeSourceDataBlock(0, 1)),
                        MarkdownBlock(MarkdownBlockType::Paragraph, "p1", 0, MakeSourceDataBlock(1, 1)),
                        MarkdownBlock(MarkdownBlockType::Header, "Overview Header", 2, MakeSourceDataBlock(2, 1)),
                        MarkdownBlock(MarkdownBlockType::Paragraph, "p2", 0, MakeSourceDataBlock(3, 1))};
    
    bool didEnterCallback = false;
    parser.parse(source, markdown, [&](const Result& report, const Blueprint& blueprint){
        didEnterCallback = true;
        
        REQUIRE(report.error.code == Error::OK);
        REQUIRE(blueprint.name == "API Name");
        REQUIRE(blueprint.description == "123");
        
    });
    REQUIRE(didEnterCallback);
}

TEST_CASE("bpparser/parse-group", "Parse resource group.")
{
    BlueprintParser parser;
    SourceData source = "0123456";
    MarkdownBlock markdown;
    markdown.blocks = { MarkdownBlock(MarkdownBlockType::Header, "API Name", 1, MakeSourceDataBlock(0, 1)),
                        MarkdownBlock(MarkdownBlockType::Paragraph, "p1", 0, MakeSourceDataBlock(1, 1)),
                        MarkdownBlock(MarkdownBlockType::HRule, MarkdownBlock::Content(), 0, MakeSourceDataBlock(2, 1) ),
                        MarkdownBlock(MarkdownBlockType::Header, "Group Name", 1, MakeSourceDataBlock(3, 1)),
                        MarkdownBlock(MarkdownBlockType::Paragraph, "p2", 0, MakeSourceDataBlock(4, 1)),
                        MarkdownBlock(MarkdownBlockType::Header, "Group Description Header", 2, MakeSourceDataBlock(5, 1)),
                        MarkdownBlock(MarkdownBlockType::Paragraph, "p3", 1, MakeSourceDataBlock(6, 1))};
    
    bool didEnterCallback = false;
    parser.parse(source, markdown, [&](const Result& report, const Blueprint& blueprint){
        didEnterCallback = true;
        
        REQUIRE(report.error.code == Error::OK);
        REQUIRE(blueprint.resourceGroups.size() == 1);
        
        ResourceGroup group = blueprint.resourceGroups.front();
        REQUIRE(group.name == "Group Name");
        REQUIRE(group.description == "456");

    });
    REQUIRE(didEnterCallback);
}

TEST_CASE("bpparser/parse-resource", "Parse simple resource.")
{
    BlueprintParser parser;
    SourceData source = "0";

    MarkdownBlock listItem(MarkdownBlockType::ListItem);
    listItem.blocks = { MarkdownBlock(MarkdownBlockType::Paragraph, "Response 200"),
                        MarkdownBlock(MarkdownBlockType::Code, "body") };
    
    MarkdownBlock list(MarkdownBlockType::List);
    list.blocks = { listItem };
    
    MarkdownBlock markdown;
    markdown.blocks = { MarkdownBlock(MarkdownBlockType::Header, "# GET /resource", 1),
                        MarkdownBlock(MarkdownBlockType::Paragraph, "p1", 0, MakeSourceDataBlock(1, 1)),
                        list };
    
    bool didEnterCallback = false;
    parser.parse(source, markdown, [&](const Result& report, const Blueprint& blueprint){
        didEnterCallback = true;
        
        REQUIRE(report.error.code == Error::OK);
        
        REQUIRE(blueprint.name.empty());
        REQUIRE(blueprint.description.empty());
        
        REQUIRE(blueprint.resourceGroups.size() == 1);

        ResourceGroup group = blueprint.resourceGroups.front();
        REQUIRE(group.name.empty());
        REQUIRE(group.description.empty());
        REQUIRE(group.resources.size() == 1);
        
        Resource resource = group.resources.front();
        REQUIRE(resource.uri == "/resource");
        REQUIRE(resource.description == "0");
        REQUIRE(resource.headers.empty());
        REQUIRE(resource.parameters.empty());
        REQUIRE(resource.methods.size() == 1);
        
        Method method = resource.methods.front();
        REQUIRE(method.method == "GET");
        REQUIRE(method.description.empty());
        REQUIRE(method.parameters.empty());
        REQUIRE(method.headers.empty());
        REQUIRE(method.requests.empty());
        REQUIRE(method.responses.size() == 1);
        
        Response response = method.responses.front();
        REQUIRE(response.name == "200");
        REQUIRE(response.description.empty());
        REQUIRE(response.parameters.empty());
        REQUIRE(response.headers.empty());
        REQUIRE(response.body == "body");
        REQUIRE(response.schema.empty());
        
    });
    REQUIRE(didEnterCallback);
}


