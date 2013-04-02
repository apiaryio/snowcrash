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
    REQUIRE_NOTHROW(parser.parse(MarkdownBlock(), nullptr));
    
    bool didEnterCallback = false;
    parser.parse(MarkdownBlock(), [&](const Result& report, const Blueprint& blueprint){
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
    parser.parse(markdown, [&](const Result& report, const Blueprint& blueprint){
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
    MarkdownBlock markdown;
    markdown.blocks = { MarkdownBlock(MarkdownBlockType::Header, "API Name", 1),
                        MarkdownBlock(MarkdownBlockType::Paragraph, "p1"),
                        MarkdownBlock(MarkdownBlockType::Header, "Overview Header", 2),
                        MarkdownBlock(MarkdownBlockType::Paragraph, "p2")};
    
    bool didEnterCallback = false;
    parser.parse(markdown, [&](const Result& report, const Blueprint& blueprint){
        didEnterCallback = true;
        
        REQUIRE(report.error.code == Error::OK);
        REQUIRE(blueprint.name == "API Name");
        // TODO:
        //REQUIRE(blueprint.description == "p1");
        
    });
    REQUIRE(didEnterCallback);
}

TEST_CASE("bpparser/parse-group", "Parse resource group.")
{
    BlueprintParser parser;
    MarkdownBlock markdown;
    markdown.blocks = { MarkdownBlock(MarkdownBlockType::Header, "API Name", 1),
                        MarkdownBlock(MarkdownBlockType::Paragraph, "p1"),
                        MarkdownBlock(MarkdownBlockType::HRule),
                        MarkdownBlock(MarkdownBlockType::Header, "Group Name", 1),
                        MarkdownBlock(MarkdownBlockType::Paragraph, "p2", 1),
                        MarkdownBlock(MarkdownBlockType::Header, "Group Description Header", 2),
                        MarkdownBlock(MarkdownBlockType::Paragraph, "p3", 1)};
    
    bool didEnterCallback = false;
    parser.parse(markdown, [&](const Result& report, const Blueprint& blueprint){
        didEnterCallback = true;
        
        REQUIRE(report.error.code == Error::OK);
        REQUIRE(blueprint.resourceGroups.size() == 1);
        REQUIRE(blueprint.resourceGroups.front().name == "Group Name");

    });
    REQUIRE(didEnterCallback);
}