//
//  test-BlueprintParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "catch.hpp"
#include "Fixture.h"
#include "BlueprintParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

snowcrash::MarkdownBlock::Stack snowcrashtest::CanonicalBlueprintFixture()
{
    // Blueprint in question:
    //R"(
    //meta: verse
    //
    //# Snowcrash API
    //
    //## Character
    //Uncle Enzo
    //
    //<see CanonicalResourceGroupFixture()>
    //
    //)";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "meta: verse", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Snowcrash API", 1, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Character", 2, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Uncle Enzo", 0, MakeSourceDataBlock(3, 1)));
    
    MarkdownBlock::Stack methodBlocks = CanonicalResourceGroupFixture();
    markdown.insert(markdown.end(), methodBlocks.begin(), methodBlocks.end());
    
    return markdown;
}

TEST_CASE("bparser/classifier", "Blueprint block classifier")
{
    MarkdownBlock::Stack markdown = CanonicalBlueprintFixture();
    
    BlockIterator cur = markdown.begin();

    // meta: verse
    REQUIRE(ClassifyBlock<Blueprint>(cur, markdown.end(), UndefinedSection) == BlueprintSection);
    
    ++cur; // # Snowcrash API
    REQUIRE(ClassifyBlock<Blueprint>(cur, markdown.end(), UndefinedSection) == BlueprintSection);
    REQUIRE(ClassifyBlock<Blueprint>(cur, markdown.end(), BlueprintSection) == BlueprintSection);
    
    ++cur; // ## Character
    REQUIRE(ClassifyBlock<Blueprint>(cur, markdown.end(), UndefinedSection) == BlueprintSection);
    REQUIRE(ClassifyBlock<Blueprint>(cur, markdown.end(), BlueprintSection) == BlueprintSection);
    
    ++cur; // Uncle Enzo
    REQUIRE(ClassifyBlock<Blueprint>(cur, markdown.end(), UndefinedSection) == BlueprintSection);    
    REQUIRE(ClassifyBlock<Blueprint>(cur, markdown.end(), BlueprintSection) == BlueprintSection);
    
    ++cur; // # Group First
    REQUIRE(ClassifyBlock<Blueprint>(cur, markdown.end(), UndefinedSection) == ResourceGroupSection);
    REQUIRE(ClassifyBlock<Blueprint>(cur, markdown.end(), BlueprintSection) == ResourceGroupSection);
}

TEST_CASE("bpparser/init", "Blueprint parser construction")
{
    BlueprintParser* parser;
    REQUIRE_NOTHROW(parser = ::new BlueprintParser);
    REQUIRE_NOTHROW(::delete parser);
}

TEST_CASE("bpparser/parse", "Parse canonical blueprint")
{
    MarkdownBlock::Stack markdown = CanonicalBlueprintFixture();
    
    Blueprint blueprint;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = BlueprintParserInner::Parse(markdown.begin(), markdown.end(), parser, blueprint);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 50);

    REQUIRE(blueprint.metadata.size() == 1);
    REQUIRE(blueprint.metadata[0].first == "meta");
    REQUIRE(blueprint.metadata[0].second == "verse");
    REQUIRE(blueprint.name == "Snowcrash API");
    REQUIRE(blueprint.description == "23");
    REQUIRE(blueprint.resourceGroups.size() == 2);
    
    REQUIRE(blueprint.resourceGroups[0].name == "First");
    REQUIRE(blueprint.resourceGroups[0].description == "1");
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    
    REQUIRE(blueprint.resourceGroups[1].name == "Second");
    REQUIRE(blueprint.resourceGroups[1].description == "2");
    REQUIRE(blueprint.resourceGroups[1].resources.empty());
}

TEST_CASE("bpparser/parse-params", "parse() method parameters.")
{
    Result result;
    Blueprint blueprint;
    
    BlueprintParser::Parse("", MarkdownBlock::Stack(), 0, result, blueprint);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
}

TEST_CASE("bpparser/parse-bp-name", "Parse blueprint name.")
{
    Result result;
    Blueprint blueprint;
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "API Name", 1));
    
    BlueprintParser::Parse("", markdown, 0, result, blueprint);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(blueprint.name == "API Name");
    REQUIRE(blueprint.description.length() == 0);
}

TEST_CASE("bpparser/parse-bp-overview", "Parse blueprint overview section.")
{
    Result result;
    Blueprint blueprint;
    SourceData source = "0123";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "API Name", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Overview Header", 2, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p2", 0, MakeSourceDataBlock(3, 1)));
    
    BlueprintParser::Parse(source, markdown, 0, result, blueprint);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(blueprint.name == "API Name");
    REQUIRE(blueprint.description == "123");
    REQUIRE(blueprint.resourceGroups.empty());
}

TEST_CASE("bpparser/parse-group", "Parse resource group.")
{
    Result result;
    Blueprint blueprint;
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "API Name", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(HRuleBlockType, MarkdownBlock::Content(), 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Group Name", 1, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p2", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Description Header", 2, MakeSourceDataBlock(5, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p3", 1, MakeSourceDataBlock(6, 1)));
    
    BlueprintParser::Parse(SourceDataFixture, markdown, 0, result, blueprint);

    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);

    ResourceGroup group = blueprint.resourceGroups.front();
    REQUIRE(group.name == "Name");
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

    Result result;
    Blueprint blueprint;
    SourceData source = "0123";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "API Name", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/resource", 1, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p2", 0, MakeSourceDataBlock(3, 1)));
    
    BlueprintParser::Parse(source, markdown, 0, result, blueprint);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(blueprint.resourceGroups.size() == 1);
    
    ResourceGroup group = blueprint.resourceGroups.front();
    REQUIRE(group.name.empty());
    REQUIRE(group.description.empty());
    REQUIRE(group.resources.size() == 1);
    
    Resource resource = group.resources.front();
    REQUIRE(resource.uriTemplate == "/resource");
    REQUIRE(resource.description == "3");
}

TEST_CASE("bpparser/parse-nameless-description", "Parse nameless blueprint description")
{
    // Blueprint in question:
    //R"(A
    //# B
    //");
    
    Result result;
    Blueprint blueprint;
    SourceData source = "01";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "A", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "B", 0, MakeSourceDataBlock(1, 1)));
    
    BlueprintParser::Parse(source, markdown, 0, result, blueprint);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1); // expected API name
    REQUIRE(result.warnings.front().location.size() == 1);
    REQUIRE(result.warnings.front().location.front().location == 0);
    REQUIRE(result.warnings.front().location.front().length == 1);
    REQUIRE(blueprint.resourceGroups.size() == 0);
    REQUIRE(blueprint.description == "01");
    
}

TEST_CASE("bpparser/parse-list-only", "Parse nameless blueprint with a list description")
{
    // Blueprint in question:
    //R"(+ list
    //");
    
    Result result;
    Blueprint blueprint;
    SourceData source = "01";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "list", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(0, 1)));
    
    BlueprintParser::Parse(source, markdown, 0, result, blueprint);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1); // expected API name
    REQUIRE(blueprint.resourceGroups.size() == 0);
    REQUIRE(blueprint.description == "0");
}

TEST_CASE("bpparser/parse-multi-groups", "Parse nameless group after defined resource")
{
    // Blueprint in question:
    //# /1
    //# GET
    //+ request
    //	+ body
    //
    //			{ ... }
    //
    //# Group Name
    //
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/1", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 2, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "request", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "body", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "{ ... }", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(7, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(8, 1)));
    
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Group Name", 0, MakeSourceDataBlock(9, 1)));
    
    Blueprint blueprint;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = BlueprintParserInner::Parse(markdown.begin(), markdown.end(), parser, blueprint);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // groups with same name & expected group name & no response
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 14);

    REQUIRE(blueprint.resourceGroups.size() == 2);
    
    REQUIRE(blueprint.resourceGroups[0].name.empty());
    REQUIRE(blueprint.resourceGroups[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    
    REQUIRE(blueprint.resourceGroups[1].name == "Name");
    REQUIRE(blueprint.resourceGroups[1].description.empty());
}

TEST_CASE("bpparser/parse-resource", "Parse simple resource.")
{
    // Blueprint in question:
    //R"(
    //# GET /resource
    //p1
    //+ Response 200
    //
    //        body
    //");
    
    Result result;
    Blueprint blueprint;
    
    SourceData source = "012345";

    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET /resource", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 1, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 1, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 1, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Response 200", 1, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "body", 1, MakeSourceDataBlock(3, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 1, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 1, MakeSourceDataBlock(5, 1)));
    
    BlueprintParser::Parse(source, markdown, 0, result, blueprint);

    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(blueprint.name.empty());
    REQUIRE(blueprint.description.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);

    ResourceGroup group = blueprint.resourceGroups.front();
    REQUIRE(group.name.empty());
    REQUIRE(group.description.empty());
    REQUIRE(group.resources.size() == 1);
    
    Resource resource = group.resources.front();
    REQUIRE(resource.uriTemplate == "/resource");
    REQUIRE(resource.description.empty());
    REQUIRE(resource.headers.empty());
    REQUIRE(resource.parameters.empty());
    REQUIRE(resource.methods.size() == 1);
    
    Method method = resource.methods.front();
    REQUIRE(method.method == "GET");
    REQUIRE(method.description == "1");
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
}

TEST_CASE("bpparser/parse-metadata", "Parse blueprint that starts with metadata")
{
    // Blueprint in question:
    //R"(
    //meta: data
    //foo: bar
    //
    //# API
    //");
    
    Result result;
    Blueprint blueprint;
    SourceData source = "meta: data\nfoo:bar\n";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "meta: data\nfoo:bar\n", 1, MakeSourceDataBlock(0, 19)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "API", 0, MakeSourceDataBlock(1, 1)));
    
    BlueprintParser::Parse(source, markdown, 0, result, blueprint);
    
    REQUIRE(result.error.code == Error::OK);
    CHECK(result.warnings.empty());
    REQUIRE(blueprint.name == "API");
    REQUIRE(blueprint.description.empty());
    REQUIRE(blueprint.resourceGroups.size() == 0);
    REQUIRE(blueprint.metadata.size() == 2);
    REQUIRE(blueprint.metadata[0].first == "meta");
    REQUIRE(blueprint.metadata[0].second == "data");
    REQUIRE(blueprint.metadata[1].first == "foo");
    REQUIRE(blueprint.metadata[1].second == "bar");
}

TEST_CASE("bpparser/parser-options-name", "Test parser options - required blueprint name")
{
    // Blueprint in question:
    //R"(
    //# GET /resource
    //");
    
    Result result;
    Blueprint blueprint;
    SourceData source = "01";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET /resource", 0, MakeSourceDataBlock(0, 1)));
    
    BlueprintParser::Parse(source, markdown, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1); // no response
    
    BlueprintParser::Parse(source, markdown, RequireBlueprintNameOption, result, blueprint);
    REQUIRE(result.error.code != Error::OK);
}

TEST_CASE("bpparser/empty-blueprint-required-name", "Test required blueprint name on empty blueprint")
{
    Result result;
    Blueprint blueprint;
    SourceData source = "01";
    
    MarkdownBlock::Stack markdown;
    
    BlueprintParser::Parse(SourceDataFixture, markdown, RequireBlueprintNameOption, result, blueprint);
    REQUIRE(result.error.code != Error::OK);
}

TEST_CASE("bpparser/metadatarequired-name", "Test required blueprint name on blueprint that starts with metadata")
{
    // Blueprint in question:
    //R"(
    //meta: data
    //foo: bar
    //");
    
    Result result;
    Blueprint blueprint;
    SourceData source = "meta: data\nfoo:bar\n";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "meta: data\nfoo:bar\n", 1, MakeSourceDataBlock(0, 19)));
    
    BlueprintParser::Parse(source, markdown, RequireBlueprintNameOption, result, blueprint);
    
    REQUIRE(result.error.code != Error::OK);
    CHECK(result.warnings.empty());
}

TEST_CASE("Incorrect warning about duplicate resources", "[blueprint][issue][3]")
{
    // Blueprint in question:
    //R"(
    //FORMAT: X-1A
    //
    //# API Name
    //
    //# Resource 1 [/1]
    //## Retrieve Resource 1 [GET]
    //+ Response 200
    //
    //        ...
    //
    //# Group Section Header
    //## Resource 2 [/2]
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "API Name", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Resource 1 [/1]", 1, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Retrieve Resource 1 [GET]", 1, MakeSourceDataBlock(2, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Response 200\n", 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "...\n", 0, MakeSourceDataBlock(4, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Group Section Header", 1, MakeSourceDataBlock(7, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Resource 2 [/2]", 1, MakeSourceDataBlock(8, 1)));
    
    Result result;
    Blueprint blueprint;
    BlueprintParser::Parse(SourceDataFixture, markdown, RequireBlueprintNameOption, result, blueprint);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());

    REQUIRE(blueprint.resourceGroups.size() == 2);
}

TEST_CASE("Fail to parse nested lists in description", "[blueprint][issue][#16]")
{
    // Blueprint in question:
    //R"(
    //# API
    //+ List
    //    + Nested Item
    //");
    
    const std::string source = \
"# API\n\
+ List\n\
    + Nested Item\n\
";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "API", 1, MakeSourceDataBlock(0, 6)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));

    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Nested Item\n", 0, MakeSourceDataBlock(19, 12)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(17, 14)));
    
    SourceDataBlock block;
    SourceDataRange r;
    r.location = 8;
    r.length = 5;
    block.push_back(r);

    r.location = 17;
    r.length = 14;
    block.push_back(r);
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "List\n", 0, block));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 25)));
    
    Blueprint blueprint;
    BlueprintParserCore parser(0, source, Blueprint());
    ParseSectionResult result = BlueprintParserInner::Parse(markdown.begin(), markdown.end(), parser, blueprint);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 9);

    REQUIRE(blueprint.name == "API");
    REQUIRE(blueprint.description == \
"+ List\n\
    + Nested Item\n\
");
    REQUIRE(blueprint.resourceGroups.empty());
}

