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
    sectionType = SectionProcessor<Action, ActionSM>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ActionSectionType);

    // Nameless method
    markdownAST.children().front().text = "GET";
    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<Action, ActionSM>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ActionSectionType);
}

TEST_CASE("Parsing action", "[action]")
{
    Action action;
    Report report;
    SectionParserHelper<Action, ActionSM, ActionParser>::parse(ActionFixture, ActionSectionType, report, action);

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
    SectionParserHelper<Action, ActionSM, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    CHECK(report.warnings.empty());

    REQUIRE(action.method == "GET");
    REQUIRE(action.description == "Small Description\n\n+ A\n\n+ B\n");

    REQUIRE(action.examples.size() == 1);
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
    SectionParserHelper<Action, ActionSM, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // warn responses with the same name

    REQUIRE(action.name.empty());
    REQUIRE(action.method == "PUT");
    REQUIRE(action.description.empty());
    REQUIRE(action.parameters.empty());

    REQUIRE(action.examples.size() == 1);
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

TEST_CASE("Parse method with multiple incomplete requests", "[action][blocks]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Request A\n"\
    "+ Request B\n"\
    "  C\n"\
    "+ Response 200\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionSM, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 2); // empty asset & preformatted asset
    REQUIRE(report.warnings[0].code == EmptyDefinitionWarning);
    REQUIRE(report.warnings[1].code == IndentationWarning);

    REQUIRE(action.name.empty());
    REQUIRE(action.method == "GET");
    REQUIRE(action.description.empty());
    REQUIRE(action.parameters.empty());

    REQUIRE(action.examples.front().requests.size() == 2);
    REQUIRE(action.examples.front().requests[0].name == "A");
    REQUIRE(action.examples.front().requests[0].body.empty());
    REQUIRE(action.examples.front().requests[0].schema.empty());
    REQUIRE(action.examples.front().requests[0].parameters.empty());
    REQUIRE(action.examples.front().requests[0].headers.empty());

    REQUIRE(action.examples.front().requests[1].name == "B");
    REQUIRE(action.examples.front().requests[1].description.empty());
    REQUIRE(action.examples.front().requests[1].body == "C\n\n");
    REQUIRE(action.examples.front().requests[1].schema.empty());
    REQUIRE(action.examples.front().requests[1].parameters.empty());
    REQUIRE(action.examples.front().requests[1].headers.empty());

    REQUIRE(action.examples.front().responses.size() == 1);
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

    Action action;
    Report report;
    SectionParserHelper<Action, ActionSM, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IgnoringWarning);

    REQUIRE(action.name.empty());
    REQUIRE(action.method == "MKCOL");
    REQUIRE(action.description.empty());
    REQUIRE(action.parameters.empty());

    REQUIRE(action.examples.size() == 1);
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
    "# PATCH /1\n\n"\
    "A\n"\
    "---\n"\
    "B\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionSM, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // no response

    REQUIRE(action.name.empty());
    REQUIRE(action.method == "PATCH");
    REQUIRE(action.description == "A\n---\n\nB\n");
    REQUIRE(action.examples.empty());
}

TEST_CASE("Parse method without name", "[action]")
{
    mdp::ByteBuffer source = "# GET";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionSM, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // no response

    REQUIRE(action.name.empty());
    REQUIRE(action.method == "GET");
    REQUIRE(action.description.empty());
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
    SectionParserHelper<Action, ActionSM, ActionParser>::parse(source, ActionSectionType, report, action);

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

TEST_CASE("Give a warning when 2xx CONNECT has a body", "[action]")
{
    mdp::ByteBuffer source = \
    "# CONNECT /1\n"\
    "+ Response 201\n\n"\
    "        {}\n\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionSM, ActionParser>::parse(source, ActionSectionType, report, action);

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
    "+ Response 200\n\n"\
    "        {}\n\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionSM, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == EmptyDefinitionWarning);

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
    SectionParserHelper<Action, ActionSM, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].requests.empty());
    REQUIRE(action.examples[0].responses.size() == 1);
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

    Action action;
    Report report;
    SectionParserHelper<Action, ActionSM, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(action.examples.size() == 2);
    REQUIRE(action.examples[0].requests.size() == 0);
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[1].requests.size() == 1);
    REQUIRE(action.examples[1].responses.size() == 0);
}
