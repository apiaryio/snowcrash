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
    SectionType sectionType;
    markdownParser.parse(ActionFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<Action>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ActionSectionType);

    // Nameless method
    markdownAST.children().front().text = "GET";
    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<Action>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ActionSectionType);
}

TEST_CASE("Parsing action", "[action]")
{
    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(ActionFixture, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    CHECK(action.report.warnings.empty());

    REQUIRE(action.node.name == "My Method");
    REQUIRE(action.node.method == "GET");
    REQUIRE(action.node.description == "Method Description\n\n");

    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples.front().requests.size() == 0);
    REQUIRE(action.node.examples.front().responses.size() == 1);

    REQUIRE(action.node.examples.front().responses[0].name == "200");
    REQUIRE(action.node.examples.front().responses[0].body == "OK.\n");
    REQUIRE(action.node.examples.front().responses[0].headers.size() == 1);
    REQUIRE(action.node.examples.front().responses[0].headers[0].first == "Content-Type");
    REQUIRE(action.node.examples.front().responses[0].headers[0].second == "text/plain");

    REQUIRE(action.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.name.sourceMap[0].location == 0);
    REQUIRE(action.sourceMap.name.sourceMap[0].length == 19);
    REQUIRE(action.sourceMap.method.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.method.sourceMap[0].location == 0);
    REQUIRE(action.sourceMap.method.sourceMap[0].length == 19);
    REQUIRE(action.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.description.sourceMap[0].location == 19);
    REQUIRE(action.sourceMap.description.sourceMap[0].length == 20);
    REQUIRE(action.sourceMap.examples.list.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].requests.list.size() == 0);
    REQUIRE(action.sourceMap.examples.list[0].responses.list.size() == 1);

    REQUIRE(action.sourceMap.examples.list[0].responses.list[0].body.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].responses.list[0].body.sourceMap[0].location == 72);
    REQUIRE(action.sourceMap.examples.list[0].responses.list[0].body.sourceMap[0].length == 7);
    REQUIRE(action.sourceMap.examples.list[0].responses.list[0].name.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].responses.list[0].name.sourceMap[0].location == 41);
    REQUIRE(action.sourceMap.examples.list[0].responses.list[0].name.sourceMap[0].length == 27);
    REQUIRE(action.sourceMap.examples.list[0].responses.list[0].headers.list.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].responses.list[0].headers.list[0].sourceMap.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].responses.list[0].headers.list[0].sourceMap[0].location == 41);
    REQUIRE(action.sourceMap.examples.list[0].responses.list[0].headers.list[0].sourceMap[0].length == 27);
}

TEST_CASE("Parse Action description with list", "[action]")
{
    mdp::ByteBuffer source = \
    "# GET\n"\
    "Small Description\n"\
    "+ A\n"\
    "+ B\n"\
    "+ Response 204\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    CHECK(action.report.warnings.empty());

    REQUIRE(action.node.method == "GET");
    REQUIRE(action.node.description == "Small Description\n\n+ A\n\n+ B\n");

    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples.front().responses.size() == 1);
    REQUIRE(action.node.examples.front().requests.empty());

    REQUIRE(action.sourceMap.name.sourceMap.empty());
    REQUIRE(action.sourceMap.method.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.method.sourceMap[0].location == 0);
    REQUIRE(action.sourceMap.method.sourceMap[0].length == 6);
    REQUIRE(action.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.description.sourceMap[0].location == 6);
    REQUIRE(action.sourceMap.description.sourceMap[0].length == 26);
    REQUIRE(action.sourceMap.examples.list.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].requests.list.size() == 0);
    REQUIRE(action.sourceMap.examples.list[0].responses.list.size() == 1);
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

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1); // warn responses with the same name

    REQUIRE(action.node.name.empty());
    REQUIRE(action.node.method == "PUT");
    REQUIRE(action.node.description.empty());
    REQUIRE(action.node.parameters.empty());

    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples.front().requests.size() == 2);

    REQUIRE(action.node.examples.front().requests[0].name == "A");
    REQUIRE(action.node.examples.front().requests[0].description == "B");
    REQUIRE(action.node.examples.front().requests[0].body == "C\n");
    REQUIRE(action.node.examples.front().requests[0].schema.empty());
    REQUIRE(action.node.examples.front().requests[0].parameters.empty());
    REQUIRE(action.node.examples.front().requests[0].headers.empty());

    REQUIRE(action.node.examples.front().requests[1].name == "D");
    REQUIRE(action.node.examples.front().requests[1].description == "E");
    REQUIRE(action.node.examples.front().requests[1].body == "F\n");
    REQUIRE(action.node.examples.front().requests[1].schema.empty());
    REQUIRE(action.node.examples.front().requests[1].parameters.empty());
    REQUIRE(action.node.examples.front().requests[1].headers.empty());

    REQUIRE(action.node.examples.front().responses.size() == 2);

    REQUIRE(action.node.examples.front().responses[0].name == "200");
    REQUIRE(action.node.examples.front().responses[0].description == "G");
    REQUIRE(action.node.examples.front().responses[0].body == "H\n");
    REQUIRE(action.node.examples.front().responses[0].schema.empty());
    REQUIRE(action.node.examples.front().responses[0].parameters.empty());
    REQUIRE(action.node.examples.front().responses[0].headers.empty());

    REQUIRE(action.node.examples.front().responses[1].name == "200");
    REQUIRE(action.node.examples.front().responses[1].description == "I");
    REQUIRE(action.node.examples.front().responses[1].body == "J\n");
    REQUIRE(action.node.examples.front().responses[1].schema.empty());
    REQUIRE(action.node.examples.front().responses[1].parameters.empty());
    REQUIRE(action.node.examples.front().responses[1].headers.empty());

    REQUIRE(action.sourceMap.name.sourceMap.empty());
    REQUIRE(action.sourceMap.method.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.method.sourceMap[0].location == 0);
    REQUIRE(action.sourceMap.method.sourceMap[0].length == 6);
    REQUIRE(action.sourceMap.description.sourceMap.empty());
    REQUIRE(action.sourceMap.examples.list.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].requests.list.size() == 2);
    REQUIRE(action.sourceMap.examples.list[0].responses.list.size() == 2);
}

TEST_CASE("Parse method with multiple incomplete requests", "[action][blocks]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Request A\n"\
    "+ Request B\n"\
    "  C\n"\
    "+ Response 200\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 2); // empty asset & preformatted asset
    REQUIRE(action.report.warnings[0].code == EmptyDefinitionWarning);
    REQUIRE(action.report.warnings[1].code == IndentationWarning);

    REQUIRE(action.node.name.empty());
    REQUIRE(action.node.method == "GET");
    REQUIRE(action.node.description.empty());
    REQUIRE(action.node.parameters.empty());

    REQUIRE(action.node.examples.front().requests.size() == 2);
    REQUIRE(action.node.examples.front().requests[0].name == "A");
    REQUIRE(action.node.examples.front().requests[0].body.empty());
    REQUIRE(action.node.examples.front().requests[0].schema.empty());
    REQUIRE(action.node.examples.front().requests[0].parameters.empty());
    REQUIRE(action.node.examples.front().requests[0].headers.empty());

    REQUIRE(action.node.examples.front().requests[1].name == "B");
    REQUIRE(action.node.examples.front().requests[1].description.empty());
    REQUIRE(action.node.examples.front().requests[1].body == "C\n\n");
    REQUIRE(action.node.examples.front().requests[1].schema.empty());
    REQUIRE(action.node.examples.front().requests[1].parameters.empty());
    REQUIRE(action.node.examples.front().requests[1].headers.empty());

    REQUIRE(action.node.examples.front().responses.size() == 1);

    REQUIRE(action.sourceMap.name.sourceMap.empty());
    REQUIRE(action.sourceMap.method.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.method.sourceMap[0].location == 0);
    REQUIRE(action.sourceMap.method.sourceMap[0].length == 9);
    REQUIRE(action.sourceMap.description.sourceMap.empty());
    REQUIRE(action.sourceMap.examples.list.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].requests.list.size() == 2);
    REQUIRE(action.sourceMap.examples.list[0].responses.list.size() == 1);
}

TEST_CASE("Parse method with foreign item", "[action]")
{
    mdp::ByteBuffer source = \
    "# MKCOL\n"\
    "+ Request\n"\
    "  + Body\n\n"\
    "            Foo\n\n"\
    "+ Bar\n"\
    "+ Response 200\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);
    REQUIRE(action.report.warnings[0].code == IgnoringWarning);

    REQUIRE(action.node.name.empty());
    REQUIRE(action.node.method == "MKCOL");
    REQUIRE(action.node.description.empty());
    REQUIRE(action.node.parameters.empty());

    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples.front().requests.size() == 1);
    REQUIRE(action.node.examples.front().requests[0].name.empty());
    REQUIRE(action.node.examples.front().requests[0].body == "Foo\n");
    REQUIRE(action.node.examples.front().requests[0].schema.empty());
    REQUIRE(action.node.examples.front().requests[0].parameters.empty());
    REQUIRE(action.node.examples.front().requests[0].headers.empty());
    REQUIRE(action.node.examples.front().responses.size() == 1);

    REQUIRE(action.sourceMap.name.sourceMap.empty());
    REQUIRE(action.sourceMap.method.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.method.sourceMap[0].location == 0);
    REQUIRE(action.sourceMap.method.sourceMap[0].length == 8);
    REQUIRE(action.sourceMap.description.sourceMap.empty());
    REQUIRE(action.sourceMap.examples.list.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].requests.list.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].responses.list.size() == 1);
}

TEST_CASE("Parse method with a HR", "[action]")
{
    mdp::ByteBuffer source = \
    "# PATCH /1\n\n"\
    "A\n"\
    "---\n"\
    "B\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1); // no response

    REQUIRE(action.node.name.empty());
    REQUIRE(action.node.method == "PATCH");
    REQUIRE(action.node.description == "A\n---\n\nB\n");
    REQUIRE(action.node.examples.empty());

    REQUIRE(action.sourceMap.name.sourceMap.empty());
    REQUIRE(action.sourceMap.method.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.method.sourceMap[0].location == 0);
    REQUIRE(action.sourceMap.method.sourceMap[0].length == 12);
    REQUIRE(action.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.description.sourceMap[0].location == 12);
    REQUIRE(action.sourceMap.description.sourceMap[0].length == 8);
    REQUIRE(action.sourceMap.examples.list.size() == 0);
}

TEST_CASE("Parse method without name", "[action]")
{
    mdp::ByteBuffer source = "# GET";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1); // no response

    REQUIRE(action.node.name.empty());
    REQUIRE(action.node.method == "GET");
    REQUIRE(action.node.description.empty());

    REQUIRE(action.sourceMap.name.sourceMap.empty());
    REQUIRE(action.sourceMap.method.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.method.sourceMap[0].location == 0);
    REQUIRE(action.sourceMap.method.sourceMap[0].length == 5);
    REQUIRE(action.sourceMap.description.sourceMap.empty());
    REQUIRE(action.sourceMap.examples.list.size() == 0);
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

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.empty());

    REQUIRE(action.node.parameters.size() == 1);
    REQUIRE(action.node.parameters[0].name == "id");
    REQUIRE(action.node.parameters[0].description == "Resource Id");
    REQUIRE(action.node.parameters[0].type == "number");
    REQUIRE(action.node.parameters[0].defaultValue.empty());
    REQUIRE(action.node.parameters[0].exampleValue == "42");
    REQUIRE(action.node.parameters[0].values.empty());

    REQUIRE(action.sourceMap.name.sourceMap.empty());
    REQUIRE(action.sourceMap.method.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.method.sourceMap[0].location == 0);
    REQUIRE(action.sourceMap.method.sourceMap[0].length == 21);
    REQUIRE(action.sourceMap.description.sourceMap.empty());
    REQUIRE(action.sourceMap.parameters.list.size() == 1);
    REQUIRE(action.sourceMap.parameters.list[0].name.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.parameters.list[0].name.sourceMap[0].location == 40);
    REQUIRE(action.sourceMap.parameters.list[0].name.sourceMap[0].length == 44);
    REQUIRE(action.sourceMap.examples.list.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].requests.list.size() == 0);
    REQUIRE(action.sourceMap.examples.list[0].responses.list.size() == 1);
}

TEST_CASE("Give a warning when 2xx CONNECT has a body", "[action]")
{
    mdp::ByteBuffer source = \
    "# CONNECT /1\n"\
    "+ Response 201\n\n"\
    "        {}\n\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);

    REQUIRE(action.node.method == "CONNECT");
    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].body == "{}\n");

    REQUIRE(action.sourceMap.name.sourceMap.empty());
    REQUIRE(action.sourceMap.method.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.method.sourceMap[0].location == 0);
    REQUIRE(action.sourceMap.method.sourceMap[0].length == 13);
    REQUIRE(action.sourceMap.description.sourceMap.empty());
    REQUIRE(action.sourceMap.examples.list.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].requests.list.size() == 0);
    REQUIRE(action.sourceMap.examples.list[0].responses.list.size() == 1);
}

TEST_CASE("Give a warning when response to HEAD has a body", "[action]")
{
    mdp::ByteBuffer source = \
    "# HEAD /1\n"\
    "+ Response 200\n\n"\
    "        {}\n\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);
    REQUIRE(action.report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(action.node.method == "HEAD");
    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].body == "{}\n");

    REQUIRE(action.sourceMap.name.sourceMap.empty());
    REQUIRE(action.sourceMap.method.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.method.sourceMap[0].location == 0);
    REQUIRE(action.sourceMap.method.sourceMap[0].length == 10);
    REQUIRE(action.sourceMap.description.sourceMap.empty());
    REQUIRE(action.sourceMap.examples.list.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].requests.list.size() == 0);
    REQUIRE(action.sourceMap.examples.list[0].responses.list.size() == 1);
}

TEST_CASE("Missing 'LINK' HTTP request method", "[action]")
{
    mdp::ByteBuffer source = \
    "# LINK /1\n"\
    "+ Response 204\n\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.empty());

    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples[0].requests.empty());
    REQUIRE(action.node.examples[0].responses.size() == 1);

    REQUIRE(action.sourceMap.name.sourceMap.empty());
    REQUIRE(action.sourceMap.method.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.method.sourceMap[0].location == 0);
    REQUIRE(action.sourceMap.method.sourceMap[0].length == 10);
    REQUIRE(action.sourceMap.description.sourceMap.empty());
    REQUIRE(action.sourceMap.examples.list.size() == 1);
    REQUIRE(action.sourceMap.examples.list[0].requests.list.size() == 0);
    REQUIRE(action.sourceMap.examples.list[0].responses.list.size() == 1);
}

TEST_CASE("Warn when request is not followed by a response", "[action]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ response 200 \n"\
    "\n"\
    "        200\n"\
    "\n"\
    "+ request A\n"\
    "\n"\
    "        A\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);
    REQUIRE(action.report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(action.node.examples.size() == 2);
    REQUIRE(action.node.examples[0].requests.size() == 0);
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[1].requests.size() == 1);
    REQUIRE(action.node.examples[1].responses.size() == 0);

    REQUIRE(action.sourceMap.name.sourceMap.empty());
    REQUIRE(action.sourceMap.method.sourceMap.size() == 1);
    REQUIRE(action.sourceMap.method.sourceMap[0].location == 0);
    REQUIRE(action.sourceMap.method.sourceMap[0].length == 9);
    REQUIRE(action.sourceMap.description.sourceMap.empty());
    REQUIRE(action.sourceMap.examples.list.size() == 2);
    REQUIRE(action.sourceMap.examples.list[0].requests.list.size() == 0);
    REQUIRE(action.sourceMap.examples.list[0].responses.list.size() == 1);
    REQUIRE(action.sourceMap.examples.list[1].requests.list.size() == 1);
    REQUIRE(action.sourceMap.examples.list[1].responses.list.size() == 0);
}
