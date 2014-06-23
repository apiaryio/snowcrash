//
//  test-ActionParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/6/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "ActionParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer ActionFixture = \
"# My Method [GET]\n\n"\
"Method Description\n\n"\
"+ Response 200 (text/plain)\n\n"\
"        OK.";

TEST_CASE("Method block classifier", "[action]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(ActionFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<Action>::sectionType(markdownAST.children().begin()) == ActionSectionType);

    // Nameless method
    markdownAST.children().front().text = "GET";
    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<Action>::sectionType(markdownAST.children().begin()) == ActionSectionType);
}

TEST_CASE("Parsing action", "[action]")
{
    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(ActionFixture, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    CHECK(report.warnings.empty());

    REQUIRE(action.name == "My Method");
    REQUIRE(action.method == "GET");
    REQUIRE(action.description == "Method Description\n\n");

    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples.front().requests.size() == 0);
    REQUIRE(action.examples.front().responses.size() == 1);

    REQUIRE(action.examples.front().responses[0].name == "200");
    REQUIRE(action.examples.front().responses[0].body == "OK.\n");
    REQUIRE(action.examples.front().responses[0].headers.size() == 1);
    REQUIRE(action.examples.front().responses[0].headers[0].first == "Content-Type");
    REQUIRE(action.examples.front().responses[0].headers[0].second == "text/plain");
}

TEST_CASE("Parse Action description with list", "[action]")
{
    mdp::ByteBuffer source = \
    "# GET\n"\
    "Small Description\n"\
    "+ A\n"\
    "+ B\n"\
    "+ Response 204\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    CHECK(report.warnings.empty());

    REQUIRE(action.method == "GET");
    REQUIRE(action.description == "Small Description\n\n+ A\n\n+ B\n");

    REQUIRE(action.examples.front().responses.size() == 1);
    REQUIRE(action.examples.front().requests.empty());
}

TEST_CASE("Parse method with multiple requests and responses", "[action]")
{
    mdp::ByteBuffer source = \
    "# PUT\n"\
    "+ Request A\n"\
    "  B\n"\
    "  + Body\n\n"\
    "            C\n\n"\
    "+ Request D\n"\
    "  E\n"\
    "  + Body\n\n"\
    "            F\n\n"\
    "+ Response 200\n"\
    "  G\n"\
    "  + Body\n\n"\
    "            H\n\n"\
    "+ Response 200\n"\
    "  I\n"\
    "  + Body\n\n"\
    "            J\n\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // warn responses with the same name

    REQUIRE(action.name.empty());
    REQUIRE(action.method == "PUT");
    REQUIRE(action.description.empty());
    REQUIRE(action.headers.empty());
    REQUIRE(action.parameters.empty());

    REQUIRE(action.examples.front().requests.size() == 2);

    REQUIRE(action.examples.front().requests[0].name == "A");
    REQUIRE(action.examples.front().requests[0].description == "B");
    REQUIRE(action.examples.front().requests[0].body == "C\n");
    REQUIRE(action.examples.front().requests[0].schema.empty());
    REQUIRE(action.examples.front().requests[0].parameters.empty());
    REQUIRE(action.examples.front().requests[0].headers.empty());

    REQUIRE(action.examples.front().requests[1].name == "D");
    REQUIRE(action.examples.front().requests[1].description == "E");
    REQUIRE(action.examples.front().requests[1].body == "F\n");
    REQUIRE(action.examples.front().requests[1].schema.empty());
    REQUIRE(action.examples.front().requests[1].parameters.empty());
    REQUIRE(action.examples.front().requests[1].headers.empty());

    REQUIRE(action.examples.front().responses.size() == 2);

    REQUIRE(action.examples.front().responses[0].name == "200");
    REQUIRE(action.examples.front().responses[0].description == "G");
    REQUIRE(action.examples.front().responses[0].body == "H\n");
    REQUIRE(action.examples.front().responses[0].schema.empty());
    REQUIRE(action.examples.front().responses[0].parameters.empty());
    REQUIRE(action.examples.front().responses[0].headers.empty());

    REQUIRE(action.examples.front().responses[1].name == "200");
    REQUIRE(action.examples.front().responses[1].description == "I");
    REQUIRE(action.examples.front().responses[1].body == "J\n");
    REQUIRE(action.examples.front().responses[1].schema.empty());
    REQUIRE(action.examples.front().responses[1].parameters.empty());
    REQUIRE(action.examples.front().responses[1].headers.empty());
}

// TODO: parser->finalize
//TEST_CASE("Parse method with multiple incomplete requests", "[action][blocks]")
//{
//    // Blueprint in question:
//    //R"(
//    //# /1
//    //# HEAD
//    //+ Request A
//    //+ Request B
//    //  C
//    //");
//
//    MarkdownBlock::Stack markdown;
//    markdown.push_back(MarkdownBlock(HeaderBlockType, "HEAD", 1, MakeSourceDataBlock(0, 1)));
//
//    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
//
//    // Request A
//    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
//    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request A", 0, MakeSourceDataBlock(1, 1)));
//
//    // Request B
//    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
//    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request B", 0, MakeSourceDataBlock(2, 1)));
//    markdown.push_back(MarkdownBlock(ParagraphBlockType, "C", 0, MakeSourceDataBlock(3, 1)));
//    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
//
//    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
//
//    Action action;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
//
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.size() == 2); // empty asset & preformatted asset
//    REQUIRE(result.first.warnings[0].code == EmptyDefinitionWarning);
//    REQUIRE(result.first.warnings[1].code == IndentationWarning);
//
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 9);
//
//    REQUIRE(action.name.empty());
//    REQUIRE(action.method == "HEAD");
//    REQUIRE(action.description.empty());
//    REQUIRE(action.headers.empty());
//    REQUIRE(action.parameters.empty());
//
//    REQUIRE(action.examples.front().requests.size() == 2);
//    REQUIRE(action.examples.front().requests[0].name == "A");
//    REQUIRE(action.examples.front().requests[0].body.empty());
//    REQUIRE(action.examples.front().requests[0].schema.empty());
//    REQUIRE(action.examples.front().requests[0].parameters.empty());
//    REQUIRE(action.examples.front().requests[0].headers.empty());
//
//    REQUIRE(action.examples.front().requests[1].name == "B");
//    REQUIRE(action.examples.front().requests[1].description.empty());
//    REQUIRE(action.examples.front().requests[1].body == "3");
//    REQUIRE(action.examples.front().requests[1].schema.empty());
//    REQUIRE(action.examples.front().requests[1].parameters.empty());
//    REQUIRE(action.examples.front().requests[1].headers.empty());
//}

TEST_CASE("Parse method with foreign item", "[action]")
{
    mdp::ByteBuffer source = \
    "# MKCOL\n"\
    "+ Request\n"\
    "  + Body\n\n"\
    "            Foo\n\n"\
    "+ Bar\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IgnoringWarning);

    REQUIRE(action.name.empty());
    REQUIRE(action.method == "MKCOL");
    REQUIRE(action.description.empty());
    REQUIRE(action.headers.empty());
    REQUIRE(action.parameters.empty());

    REQUIRE(action.examples.front().requests.size() == 1);
    REQUIRE(action.examples.front().requests[0].name.empty());
    REQUIRE(action.examples.front().requests[0].body == "Foo\n");
    REQUIRE(action.examples.front().requests[0].schema.empty());
    REQUIRE(action.examples.front().requests[0].parameters.empty());
    REQUIRE(action.examples.front().requests[0].headers.empty());
}

TEST_CASE("Parse method with a HR", "[action]")
{
    mdp::ByteBuffer source = \
    "# PATCH /1\n"\
    "---\n"\
    "A\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    CHECK(report.warnings.empty());

    REQUIRE(action.name.empty());
    REQUIRE(action.method == "PATCH");
    REQUIRE(action.description == "---\n\nA\n");
    REQUIRE(action.examples.empty());
}

// TODO: ResourceParser
//TEST_CASE( "Parse incomplete method followed by another resource", "[action][blocks]")
//{
//    // Blueprint in question:
//    //R"(
//    //# /1
//    //## GET
//    //# /2
//
//    MarkdownBlock::Stack markdown;
//    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 2, MakeSourceDataBlock(0, 1)));
//    markdown.push_back(MarkdownBlock(HeaderBlockType, "/2", 1, MakeSourceDataBlock(1, 1)));
//
//    Action action;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
//
//    REQUIRE(result.first.error.code == Error::OK);
//    CHECK(result.first.warnings.empty());
//
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 1);
//
//    REQUIRE(action.name.empty());
//    REQUIRE(action.method == "GET");
//    REQUIRE(action.description.empty());
//}

TEST_CASE("Parse method without name", "[action]")
{
    mdp::ByteBuffer source = "# GET";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(action.name.empty());
    REQUIRE(action.method == "GET");
    REQUIRE(action.description.empty());
}

TEST_CASE("Make sure method followed by a group does not eat the group", "[action][blocks]")
{
    mdp::ByteBuffer source = \
    "## POST\n"\
    "# Group Two\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    CHECK(report.warnings.empty());

    REQUIRE(action.method == "POST");
    REQUIRE(action.description == "# Group Two\n");

}

TEST_CASE("Parse action with parameters", "[action]")
{
    mdp::ByteBuffer source = \
    "# GET /resrouce/{id}\n"\
    "+ Parameters\n"\
    "    + id (required, number, `42`) ... Resource Id\n"\
    "\n"\
    "+ Response 204\n"\
    "\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(action.parameters.size() == 1);
    REQUIRE(action.parameters[0].name == "id");
    REQUIRE(action.parameters[0].description == "Resource Id");
    REQUIRE(action.parameters[0].type == "number");
    REQUIRE(action.parameters[0].defaultValue.empty());
    REQUIRE(action.parameters[0].exampleValue == "42");
    REQUIRE(action.parameters[0].values.empty());
}

TEST_CASE("Do not report empty message body for requests", "[action]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Request\n"\
    "    + Headers \n"\
    "\n"\
    "            Accept: application/json, application/javascript\n\n"\
    "+ Response 204\n\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].requests.size() == 1);
    REQUIRE(action.examples[0].requests[0].headers.size() == 1);
    REQUIRE(action.examples[0].requests[0].headers[0].first == "Accept");
    REQUIRE(action.examples[0].requests[0].headers[0].second == "application/json, application/javascript");
    REQUIRE(action.examples[0].requests[0].body.empty());
    REQUIRE(action.examples[0].responses.size() == 1);
}

TEST_CASE("Give a warning of empty message body for requests with certain headers", "[action]")
{
    mdp::ByteBuffer source = \
    "# POST /1\n"\
    "+ Request\n"\
    "    + Headers \n"\
    "\n"\
    "            Content-Length: 100\n\n"\
    "+ Response 204\n\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);

    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].requests.size() == 1);
    REQUIRE(action.examples[0].requests[0].headers.size() == 1);
    REQUIRE(action.examples[0].requests[0].headers[0].first == "Content-Length");
    REQUIRE(action.examples[0].requests[0].headers[0].second == "100");
    REQUIRE(action.examples[0].requests[0].body.empty());
    REQUIRE(action.examples[0].responses.size() == 1);
}

TEST_CASE("Give a warning when 100 response has a body", "[action]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 100\n\n"\
    "        {}\n\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);

    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].body == "{}\n");
}

TEST_CASE("Give a warning when 2xx CONNECT has a body", "[action]")
{
    mdp::ByteBuffer source = \
    "# CONNECT /1\n"\
    "+ Response 201\n\n"\
    "        {}\n\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);

    REQUIRE(action.method == "CONNECT");
    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].body == "{}\n");
}

TEST_CASE("Give a warning when response to HEAD has a body", "[action]")
{
    mdp::ByteBuffer source = \
    "# HEAD /1\n"\
    "+ Response 204\n\n"\
    "        {}\n\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 2);

    REQUIRE(action.method == "HEAD");
    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].body == "{}\n");
}

TEST_CASE("Missing 'LINK' HTTP request method", "[action]")
{
    mdp::ByteBuffer source = \
    "# LINK /1\n"\
    "+ Response 204\n\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].requests.empty());
    REQUIRE(action.examples[0].responses.size() == 1);
}

// TODO: parser->finalize
//TEST_CASE("Warn when request is not followed by a response", "[pairing][action][issue][#53][source]")
//{
//    // Blueprint in question:
//    //R"(
//    //# GET /1
//    //+ response 200
//    //
//    //        200
//    //
//    //+ request A
//    //
//    //        A
//    //);
//    const std::string blueprintSource = \
//    "# GET /1\n"\
//    "+ response 200 \n"\
//    "\n"\
//    "        200\n"\
//    "\n"\
//    "+ request A\n"\
//    "\n"\
//    "        A\n";
//
//    Parser parser;
//    Result result;
//    Blueprint blueprint;
//    parser.parse(blueprintSource, 0, result, blueprint);
//    REQUIRE(result.error.code == Error::OK);
//    REQUIRE(result.warnings.size() == 1);
//    REQUIRE(result.warnings[0].code == EmptyDefinitionWarning);
//
//    REQUIRE(blueprint.resourceGroups.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 2);
//}

