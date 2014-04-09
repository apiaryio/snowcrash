//
//  test-ResourceParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "ResourceParser.h"
#include "ResourceGroupParser.h"
#include "Parser.h"
#include "Fixture.h"

using namespace snowcrash;
using namespace snowcrashtest;

MarkdownBlock::Stack snowcrashtest::CanonicalResourceFixture()
{
    // Blueprint in question:
    //R"(
    //# My Resource [/resource/{id}]
    //Resource Description
    //
    //+ Model (text/plain)
    //
    //        X.O.
    //
    //  <see CanonicalParametersFixture()>
    //
    //
    // <see CanonicalActionFixture()>
    //
    //)";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "My Resource [/resource/{id}{?limit}]", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Resource Description", 0, MakeSourceDataBlock(1, 1)));    

    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));

    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Model (text/plain)", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "X.O.", 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
    
    // Inject parameters
    MarkdownBlock::Stack parameters = CanonicalParametersFixture();
    MarkdownBlock::Stack::iterator begin = parameters.begin();
    ++begin;
    MarkdownBlock::Stack::iterator end = parameters.end();
    --end;
    markdown.insert(markdown.end(), begin, end);
    
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(8, 1)));
    
    MarkdownBlock::Stack blocksFixture = CanonicalActionFixture();
    markdown.insert(markdown.end(), blocksFixture.begin(), blocksFixture.end());
    
    return markdown;
}

TEST_CASE("Resource block classifier", "[resource][block]")
{
    MarkdownBlock::Stack markdown = CanonicalResourceFixture();
    
    BlockIterator cur = markdown.begin();
    // Named resource: "My Resource [/resource]"
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), UndefinedSectionType) == ResourceSectionType);
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), ResourceSectionType) == UndefinedSectionType);
    
    ++cur; // "Resource Description"
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), ResourceSectionType) == ResourceSectionType);

    ++cur; // ListBlockBeginType - "Model"
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), UndefinedSectionType) == ModelSectionType);
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), ResourceSectionType) == ModelSectionType);
    
    ++cur; // ListItemBlockBeginType - "My Resource Object"
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), UndefinedSectionType) == ModelSectionType);
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), ModelSectionType) == ModelSectionType);
    
    std::advance(cur, 4); // ListItemBlockBeginType - "Parameters"
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), UndefinedSectionType) == ParametersSectionType);
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), ResourceSectionType) == ParametersSectionType);

    std::advance(cur, 40); // Method
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), UndefinedSectionType) == ActionSectionType);
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), ResourceSectionType) == ActionSectionType);
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), HeadersSectionType) == ActionSectionType);
    
    // Nameless resource: "/resource"
    markdown[0].content = "/resource";
    cur = markdown.begin();
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), UndefinedSectionType) == ResourceSectionType);
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), ResourceSectionType) == UndefinedSectionType);
    
    // Keyword "group"
    markdown[0].content = "Group A";
    cur = markdown.begin();
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), ResourceSectionType) == UndefinedSectionType);
}

TEST_CASE("Abbreviated Resource Method block classifier", "[resource][block]")
{
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET /resource", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "POST", 1, MakeSourceDataBlock(1, 1)));
    
    BlockIterator cur = markdown.begin();
    // "GET /resource"
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), UndefinedSectionType) == ResourceMethodSectionType);
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), ResourceMethodSectionType) == UndefinedSectionType);
    
    ++cur; // "POST"
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), ResourceSectionType) == ActionSectionType);
    REQUIRE(ClassifyBlock<Resource>(cur, markdown.end(), ResourceMethodSectionType) == UndefinedSectionType);
}

TEST_CASE("Parse resource", "[resource][block]")
{
    MarkdownBlock::Stack markdown = CanonicalResourceFixture();
    Resource resource;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resource);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 42 + 31);
    
    REQUIRE(resource.name == "My Resource");
    REQUIRE(resource.uriTemplate == "/resource/{id}{?limit}");
    REQUIRE(resource.model.name == "My Resource");
    REQUIRE(resource.model.body == "X.O.");
    REQUIRE(resource.model.headers.size() == 1);
    REQUIRE(resource.model.headers[0].first == "Content-Type");
    REQUIRE(resource.model.headers[0].second == "text/plain");
    REQUIRE(resource.description == "1");
    REQUIRE(resource.parameters.size() == 2);
    REQUIRE(resource.parameters[0].name == "id");
    REQUIRE(resource.parameters[1].name == "limit");
    REQUIRE(resource.headers.empty());
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions.front().method == "GET");
}

TEST_CASE("Parse partially defined resource", "[resource][block]")
{
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/1", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(3, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    
    Resource resource;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resource);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 2); // no response & preformatted asset
    REQUIRE(result.first.warnings[0].code == IndentationWarning);
    REQUIRE(result.first.warnings[1].code == EmptyDefinitionWarning);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 8);

    REQUIRE(resource.name.empty());
    REQUIRE(resource.uriTemplate == "/1");
    REQUIRE(resource.description.empty());
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions.front().method == "GET");
    REQUIRE(resource.actions.front().description.empty());
    REQUIRE(!resource.actions.front().examples.empty());
    REQUIRE(resource.actions.front().examples.front().requests.size() == 1);
    REQUIRE(resource.actions.front().examples.front().requests.front().name.empty());
    REQUIRE(resource.actions.front().examples.front().requests.front().description.empty());
    REQUIRE(resource.actions.front().examples.front().requests.front().body == "3");
}

TEST_CASE("Parse multiple method descriptions", "[resource][block]")
{
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/1", 1, MakeSourceDataBlock(0, 1)));    
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "POST", 1, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p2", 0, MakeSourceDataBlock(3, 1)));
    
    Resource resource;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resource);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 2); // 2x no response
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 5);
    
    REQUIRE(resource.uriTemplate == "/1");
    REQUIRE(resource.description.empty());
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.size() == 2);
    REQUIRE(resource.actions[0].method == "GET");
    REQUIRE(resource.actions[0].description == "1");
    REQUIRE(resource.actions[1].method == "POST");
    REQUIRE(resource.actions[1].description == "3");
}

TEST_CASE("Parse multiple methods", "[resource][block]")
{    
    // Blueprint in question:
    //R"(
    //# /1
    //A
    //
    //## GET
    //B
    //
    //+ Response 200
    //    + Body
    //
    //            Code 1
    //
    //## HEAD
    //C
    //
    //+ Request D
    //+ Response 200
    //    + Body
    //
    //
    //## PUT
    //E
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/1", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "A", 0, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 2, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "B", 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Response 200", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(5, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Code 1", 0, MakeSourceDataBlock(6, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(7, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(8, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(9, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(10, 1)));
    
    markdown.push_back(MarkdownBlock(HeaderBlockType, "HEAD", 2, MakeSourceDataBlock(11, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "C", 0, MakeSourceDataBlock(12, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request D", 0, MakeSourceDataBlock(17, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Response 200", 0, MakeSourceDataBlock(13, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Body", 0, MakeSourceDataBlock(14, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(15, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(16, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(18, 1)));
    
    markdown.push_back(MarkdownBlock(HeaderBlockType, "PUT", 2, MakeSourceDataBlock(19, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "E", 0, MakeSourceDataBlock(20, 1)));
    
    Resource resource;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resource);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 2); // empty body asset & no response
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 30);
    
    REQUIRE(resource.uriTemplate == "/1");
    REQUIRE(resource.description == "1");
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.size() == 3);
    REQUIRE(resource.actions[0].method == "GET");
    REQUIRE(resource.actions[0].description == "3");
    REQUIRE(!resource.actions[0].examples.empty());
    REQUIRE(resource.actions[0].examples[0].requests.empty());
    REQUIRE(resource.actions[0].examples[0].responses.size() == 1);
    REQUIRE(resource.actions[0].examples[0].responses[0].name == "200");
    REQUIRE(resource.actions[0].examples[0].responses[0].description.empty());
    REQUIRE(resource.actions[0].examples[0].responses[0].body == "Code 1");
    
    REQUIRE(resource.actions[1].method == "HEAD");
    REQUIRE(resource.actions[1].description == "C");
    REQUIRE(!resource.actions[1].examples.empty());
    REQUIRE(resource.actions[1].examples[0].requests.size() == 1);
    REQUIRE(resource.actions[1].examples[0].requests[0].name == "D");
    REQUIRE(resource.actions[1].examples[0].requests[0].description.empty());
    REQUIRE(resource.actions[1].examples[0].requests[0].description.empty());
    
    REQUIRE(resource.actions[1].examples[0].responses.size() == 1);
    REQUIRE(resource.actions[1].examples[0].responses[0].name == "200");
    REQUIRE(resource.actions[1].examples[0].responses[0].description.empty());
    REQUIRE(resource.actions[1].examples[0].responses[0].body.empty());
    
    REQUIRE(resource.actions[2].method == "PUT");
    REQUIRE(resource.actions[2].description == "K");
    REQUIRE(resource.actions[2].examples.empty());
}

TEST_CASE("Parse description with list", "[resource][block]")
{
    // Blueprint in question:
    //R"(
    //# /1
    //+ A
    //+ B
    //
    //p1
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/1", 1, MakeSourceDataBlock(0, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));

    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "A", 0, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "B", 0, MakeSourceDataBlock(2, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(4, 1)));
    
    Resource resource;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resource);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 8);
    
    REQUIRE(resource.uriTemplate == "/1");
    REQUIRE(resource.description == "34");
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.empty());
}

TEST_CASE("Parse resource with a HR", "[resource][block]")
{
    
    // Blueprint in question:
    //R"(
    //# /1
    //---
    //A
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/1", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HRuleBlockType, SourceData(), 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "A", 0, MakeSourceDataBlock(2, 1)));
    
    Resource resource;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resource);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(resource.uriTemplate == "/1");
    REQUIRE(resource.description == "12");
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.empty());
}

TEST_CASE("Parse resource method abbreviation", "[resource][block]")
{
    // Blueprint in question:
    //R"(
    //# GET /resource
    //Description
    //
    //+ Response 200
    //    + Body
    //
    //            { ... }
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET /resource", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Description", 1, MakeSourceDataBlock(1, 1)));

    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Response 200", 0, MakeSourceDataBlock(2, 1)));

    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "{ ... }", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(7, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(8, 1)));
    
    Resource resource;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resource);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 13);
    
    REQUIRE(resource.name.empty());
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions[0].method == "GET");
    REQUIRE(resource.actions[0].description == "1");
    REQUIRE(resource.actions[0].examples[0].responses.size() == 1);
}

TEST_CASE("Parse resource method abbreviation followed by a foreign method", "[resource][block]")
{
    // Blueprint in question:
    //R"(
    //# GET /resource
    //# POST
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET /resource", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "POST", 1, MakeSourceDataBlock(1, 1)));
    
    Resource resource;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resource);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 2); // no response & ignoring possible resource method
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 1);
    
    REQUIRE(resource.name.empty());
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions[0].method == "GET");
}

TEST_CASE("Parse resource without name", "[resource][block]")
{
    // Blueprint in question:
    //R"(
    //# /resource
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/resource", 1, MakeSourceDataBlock(0, 1)));
    
    Resource resource;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resource);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 0);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 1);

    REQUIRE(resource.uriTemplate == "/resource");
    REQUIRE(resource.name.empty());
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.size() == 0);
}

TEST_CASE("Warn about parameters not in URI template", "[resource][source]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //# /resource/{id}
    //+ Parameters
    //    + olive
    //
    //## GET
    //+ Parameters
    //    + cheese
    //    + id
    //
    //+ Response 204
    //");
    const std::string blueprintSource = \
    "# /resource/{id}\n"\
    "+ Parameters\n"\
    "    + olive\n"\
    "\n"\
    "## GET\n"\
    "+ Parameters\n"\
    "    + cheese\n"\
    "    + id\n"\
    "\n"\
    "+ Response 204\n\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 2);
    REQUIRE(result.warnings[0].code == LogicalErrorWarning);
    REQUIRE(result.warnings[1].code == LogicalErrorWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].name == "olive");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 2);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "cheese");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[1].name == "id");
}

TEST_CASE("Parse nameless resource with named model", "[resource][model][source]")
{
    // Blueprint in question:
    //R"(
    //# /message
    //+ Super Model
    //  
    //        AAA
    //
    //");
    const std::string blueprintSource = \
    "# /message\n"\
    "+ Super Model\n"\
    "\n"\
    "        AAA\n"\
    "\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].model.name == "Super");
    REQUIRE(blueprint.resourceGroups[0].resources[0].model.body == "AAA\n");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
}

TEST_CASE("Parse nameless resource with nameless model", "[resource][model][source]")
{
    // Blueprint in question:
    //R"(
    //# /message
    //+ Model
    //
    //        AAA
    //
    //");
    const std::string blueprintSource = \
    "# /message\n"\
    "+ Model\n"\
    "\n"\
    "        AAA\n"\
    "\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == SymbolError);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.empty());
}

TEST_CASE("Parse named resource with nameless model", "[resource][model][source]")
{
    // Blueprint in question:
    //R"(
    //# Message [/message]
    //+ Model
    //  
    //        AAA
    //
    //## Retrieve a message [GET]
    //+ Response 200
    //    
    //    [Message][]
    //");
    
    const std::string blueprintSource = \
    "# Message [/message]\n"\
    "+ Model\n"\
    "  \n"\
    "        AAA\n"\
    "\n"\
    "## Retrieve a message [GET]\n"\
    "+ Response 200\n"\
    "    \n"\
    "    [Message][]\n\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].model.name == "Message");
    REQUIRE(blueprint.resourceGroups[0].resources[0].model.body == "AAA\n");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].name == "Retrieve a message");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].method == "GET");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "AAA\n");
}

TEST_CASE("Parse root resource", "[resource][source][issue][#40]")
{
    // Blueprint in question:
    //R"(
    //# API Root [/]
    //");
    
    const std::string blueprintSource = \
    "# API Root [/]\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].name == "API Root");
    REQUIRE(blueprint.resourceGroups[0].resources[0].uriTemplate == "/");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
}

TEST_CASE("Deprecated resource headers", "[resource][source][deprecated]")
{
    // Blueprint in question:
    //R"(
    //# /
    //+ Headers
    //
    //        header1: value1
    //
    //# GET
    //+ Headers
    //
    //        header2: value2
    //
    //+ Response 200
    //    + Headers
    //
    //            header3: value3
    //
    //
    //");
    
    const std::string blueprintSource = \
    "# /\n"\
    "+ Headers\n"\
    "\n"\
    "        header1: value1\n"\
    "\n"\
    "# GET\n"\
    "+ Headers\n"\
    "\n"\
    "        header2: value2\n"\
    "\n"\
    "+ Response 200\n"\
    "    + Headers\n"\
    "\n"\
    "            header3: value3\n"\
    "\n"\
    "\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 2);
    REQUIRE(result.warnings[0].code == DeprecatedWarning);
    REQUIRE(result.warnings[1].code == DeprecatedWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].headers.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].headers.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers.size() == 3);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[0].first == "header1");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[0].second == "value1");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[1].first == "header2");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[1].second == "value2");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[2].first == "header3");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[2].second == "value3");
}
