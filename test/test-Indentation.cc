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

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(action.description.empty());
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].name == "200");
    REQUIRE(action.examples[0].responses[0].body == "{ ... }\n");
}

TEST_CASE("No Indentation & No Newline", "[indentation]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "{ ... }\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IndentationWarning);
    ReportDebugMessage(report.warnings[0].message);

    REQUIRE(action.description.empty());
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].name == "200");
    REQUIRE(action.examples[0].responses[0].body == "{ ... }\n\n");
}

TEST_CASE("No Indentation", "[indentation]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "\n"\
    "{ ... }\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
//    REQUIRE(report.warnings.size() == 1);
//    REQUIRE(report.warnings[0].code == IndentationWarning);
//    ReportDebugMessage(report.warnings[0].message);

    REQUIRE(action.description.empty());
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].name == "200");
//    REQUIRE(action.examples[0].responses[0].body == "{ ... }\n\n");
}

TEST_CASE("Poor Indentation & No Newline", "[indentation]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "    { ... }\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IndentationWarning);
    ReportDebugMessage(report.warnings[0].message);

    REQUIRE(action.description.empty());
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].name == "200");
    REQUIRE(action.examples[0].responses[0].body == "{ ... }\n\n");
}

TEST_CASE("Poor Indentation", "[indentation]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "\n"\
    "    { ... }\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IndentationWarning);
    ReportDebugMessage(report.warnings[0].message);

    REQUIRE(action.description.empty());
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].name == "200");
    REQUIRE(action.examples[0].responses[0].body == "{ ... }\n");
}

TEST_CASE("OK Indentation & No Newline", "[indentation]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "        { ... }\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IndentationWarning);
    ReportDebugMessage(report.warnings[0].message);

    REQUIRE(action.description.empty());
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].name == "200");
    REQUIRE(action.examples[0].responses[0].body == "    { ... }\n\n");
}

TEST_CASE("Full syntax - correct", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "    + Body\n"\
    "\n"\
    "            { ... }\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].name == "200");
    REQUIRE(action.examples[0].responses[0].body == "{ ... }\n");
}

TEST_CASE("Full syntax - Poor Body Indentation", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "\n"\
    "        { ... }\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
//    REQUIRE(report.warnings.size() == 1);
//    REQUIRE(report.warnings[0].code == IgnoringWarning);
//    ReportDebugMessage(report.warnings[0].message);

    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].name == "200");
    REQUIRE(action.examples[0].responses[0].body.empty());
}

TEST_CASE("Full syntax - Poor Body & Body Asset Indentation", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "\n"\
    "    { ... }\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
//    REQUIRE(report.warnings.size() == 1);
//    REQUIRE(report.warnings[0].code == IgnoringWarning);
//    ReportDebugMessage(report.warnings[0].message);

    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].name == "200");
    REQUIRE(action.examples[0].responses[0].body.empty());
}

TEST_CASE("Full syntax - Poor Body & Body Asset Indentation & No Newline", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "    { ... }\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IgnoringWarning);
    ReportDebugMessage(report.warnings[0].message);

    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].name == "200");
    REQUIRE(action.examples[0].responses[0].body.empty());
}

TEST_CASE("Full syntax - No Indentation", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "\n"\
    "{ ... }\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
//    REQUIRE(report.warnings.size() == 1);
//    REQUIRE(report.warnings[0].code == IgnoringWarning);
//    ReportDebugMessage(report.warnings[0].message);

    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].name == "200");
    REQUIRE(action.examples[0].responses[0].body.empty());
}

TEST_CASE("Full syntax - No Indentation & No Newline", "[indentation]")
{
    mdp::ByteBuffer source = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "{ ... }\n";

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IgnoringWarning);
    ReportDebugMessage(report.warnings[0].message);

    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].name == "200");
    REQUIRE(action.examples[0].responses[0].body.empty());
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

    Action action;
    Report report;
    SectionParserHelper<Action, ActionParser>::parse(source, ActionSectionType, report, action);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IndentationWarning);
    ReportDebugMessage(report.warnings[0].message);

    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples[0].responses.size() == 1);
    REQUIRE(action.examples[0].responses[0].name == "200");
    REQUIRE(action.examples[0].responses[0].body == "+ Body\n\n        { ... }\n");
}
