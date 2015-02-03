//
//  test-Indentation.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 10/31/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrash.h"
#include "snowcrashtest.h"
#include "ActionParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

void ReportDebugMessage(const std::string& msg)
{
#ifdef DEBUG
    std::cout << "MESSAGE: ";
    std::cout << msg << std::endl << std::endl;
#endif
}

TEST_CASE("Correct indentation", "[indentation]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "\n"\
    "        { ... }\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.empty());

    REQUIRE(action.node.description.empty());
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body == "{ ... }\n");
}

TEST_CASE("No Indentation & No Newline", "[indentation]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "{ ... }\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);
    REQUIRE(action.report.warnings[0].code == IndentationWarning);
    ReportDebugMessage(action.report.warnings[0].message);

    REQUIRE(action.node.description.empty());
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body == "{ ... }\n\n");
}

TEST_CASE("No Indentation", "[indentation]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "\n"\
    "{ ... }\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);
    REQUIRE(action.report.warnings[0].code == IndentationWarning);
    ReportDebugMessage(action.report.warnings[0].message);

    REQUIRE(action.node.description.empty());
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body == "{ ... }\n\n");
}

TEST_CASE("Poor Indentation & No Newline", "[indentation]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "    { ... }\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);
    REQUIRE(action.report.warnings[0].code == IndentationWarning);
    ReportDebugMessage(action.report.warnings[0].message);

    REQUIRE(action.node.description.empty());
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body == "{ ... }\n\n");
}

TEST_CASE("Poor Indentation", "[indentation]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "\n"\
    "    { ... }\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);
    REQUIRE(action.report.warnings[0].code == IndentationWarning);
    ReportDebugMessage(action.report.warnings[0].message);

    REQUIRE(action.node.description.empty());
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body == "{ ... }\n");
}

TEST_CASE("OK Indentation & No Newline", "[indentation]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "        { ... }\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);
    REQUIRE(action.report.warnings[0].code == IndentationWarning);
    ReportDebugMessage(action.report.warnings[0].message);

    REQUIRE(action.node.description.empty());
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body == "    { ... }\n\n");
}

TEST_CASE("Full syntax - correct", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "    + Body\n"\
    "\n"\
    "            { ... }\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.empty());

    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body == "{ ... }\n");
}

TEST_CASE("Full syntax - Poor Body Indentation", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "\n"\
    "        { ... }\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);
    REQUIRE(action.report.warnings[0].code == IgnoringWarning);
    ReportDebugMessage(action.report.warnings[0].message);

    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body.empty());
}

TEST_CASE("Full syntax - Poor Body & Body Asset Indentation", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "\n"\
    "    { ... }\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);
    REQUIRE(action.report.warnings[0].code == IgnoringWarning);
    ReportDebugMessage(action.report.warnings[0].message);

    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body.empty());
}

TEST_CASE("Full syntax - Poor Body & Body Asset Indentation & No Newline", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "    { ... }\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);
    REQUIRE(action.report.warnings[0].code == IgnoringWarning);
    ReportDebugMessage(action.report.warnings[0].message);

    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body.empty());
}

TEST_CASE("Full syntax - No Indentation", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "\n"\
    "{ ... }\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 2);
    REQUIRE(action.report.warnings[0].code == IgnoringWarning);
    REQUIRE(action.report.warnings[1].code == IndentationWarning);
    ReportDebugMessage(action.report.warnings[0].message);
    ReportDebugMessage(action.report.warnings[1].message);

    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body == "{ ... }\n\n");
}

TEST_CASE("Full syntax - No Indentation & No Newline", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "{ ... }\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);
    REQUIRE(action.report.warnings[0].code == IgnoringWarning);
    ReportDebugMessage(action.report.warnings[0].message);

    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body.empty());
}

TEST_CASE("Full syntax - Extra indentation", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "\n"\
    "        + Body\n"\
    "\n"\
    "                { ... }\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 1);
    REQUIRE(action.report.warnings[0].code == IndentationWarning);
    ReportDebugMessage(action.report.warnings[0].message);

    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body == "+ Body\n\n        { ... }\n");
}

TEST_CASE("No Indentation & No Newline multi-line", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "{\n"\
    "\n"\
    "    Hello\n"\
    "}\n";

    ParseResult<Action> action;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, action);

    REQUIRE(action.report.error.code == Error::OK);
    REQUIRE(action.report.warnings.size() == 2);
    REQUIRE(action.report.warnings[0].code == IndentationWarning);
    ReportDebugMessage(action.report.warnings[0].message);

    REQUIRE(action.report.warnings[1].code == IndentationWarning);
    ReportDebugMessage(action.report.warnings[1].message);

    REQUIRE(action.node.examples.size() == 1);
    REQUIRE(action.node.examples[0].responses.size() == 1);
    REQUIRE(action.node.examples[0].responses[0].name == "200");
    REQUIRE(action.node.examples[0].responses[0].body == "{\nHello\n}\n");
}
