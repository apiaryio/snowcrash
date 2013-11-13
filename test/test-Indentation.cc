//
//  test-Indentation.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 10/31/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iterator>
#include <iostream>
#include "catch.hpp"
#include "Fixture.h"
#include "Parser.h"

using namespace snowcrash;
using namespace snowcrashtest;

void ReportDebugMessage(const std::string& msg)
{
#ifdef DEBUG
    std::cout << "MESSAGE: ";
    std::cout << msg << std::endl << std::endl;
#endif
}

TEST_CASE("Correct indentation", "[abbreviated][indentation]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Response 200
    //
    //        { ... }
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "\n"\
    "        { ... }\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "{ ... }\n");
}

TEST_CASE("No Indentation & No Newline", "[abbreviated][indentation]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Response 200
    //{ ... }
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "{ ... }\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IndentationWarning);
    ReportDebugMessage(result.warnings[0].message);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "{ ... }\n");
}

TEST_CASE("No Indentation", "[abbreviated][indentation]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Response 200
    //
    //{ ... }
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "\n"\
    "{ ... }\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IndentationWarning);
    ReportDebugMessage(result.warnings[0].message);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "{ ... }\n");
}

TEST_CASE("Poor Indentation & No Newline", "[abbreviated][indentation]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Response 200
    //    { ... }
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "    { ... }\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IndentationWarning);
    ReportDebugMessage(result.warnings[0].message);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "{ ... }\n");
}

TEST_CASE("Poor Indentation", "[abbreviated][indentation]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Response 200
    //
    //    { ... }
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "\n"\
    "    { ... }\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IndentationWarning);
    ReportDebugMessage(result.warnings[0].message);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "{ ... }\n");
}

TEST_CASE("OK Indentation & No Newline", "[abbreviated][indentation]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Response 200
    //        { ... }
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Response 200\n"\
    "        { ... }\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IndentationWarning);
    ReportDebugMessage(result.warnings[0].message);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "    { ... }\n");
}

TEST_CASE("Full syntax - correct", "[indentation]")
{
    // Blueprint in question:
    //R"(
    //## GET /1
    //+ Response 200
    //    + Body
    //
    //            { ... }
    //");
    const std::string blueprintSource = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "    + Body\n"\
    "\n"\
    "            { ... }\n";
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(!blueprint.resourceGroups[0].resources[0].actions[0].examples.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "{ ... }\n");
}

TEST_CASE("Full syntax - Poor Body Indentation", "[indentation]")
{
    // Blueprint in question:
    //R"(
    //## GET /1
    //+ Response 200
    //+ Body
    //
    //        { ... }
    //");
    const std::string blueprintSource = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "\n"\
    "        { ... }\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    ReportDebugMessage(result.warnings[0].message);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(!blueprint.resourceGroups[0].resources[0].actions[0].examples.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body.empty());
}

TEST_CASE("Full syntax - Poor Body & Body Asset Indentation", "[indentation]")
{
    // Blueprint in question:
    //R"(
    //## GET /1
    //+ Response 200
    //+ Body
    //
    //    { ... }
    //");
    const std::string blueprintSource = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "\n"\
    "    { ... }\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    ReportDebugMessage(result.warnings[0].message);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(!blueprint.resourceGroups[0].resources[0].actions[0].examples.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body.empty());
}

TEST_CASE("Full syntax - Poor Body & Body Asset Indentation & No Newline", "[indentation]")
{
    // Blueprint in question:
    //R"(
    //## GET /1
    //+ Response 200
    //+ Body
    //    { ... }
    //");
    const std::string blueprintSource = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "    { ... }\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    ReportDebugMessage(result.warnings[0].message);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(!blueprint.resourceGroups[0].resources[0].actions[0].examples.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body.empty());
}

TEST_CASE("Full syntax - No Indentation", "[indentation]")
{
    // Blueprint in question:
    //R"(
    //## GET /1
    //+ Response 200
    //+ Body
    //
    //{ ... }
    //");
    const std::string blueprintSource = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "\n"\
    "{ ... }\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == BusinessError);
    ReportDebugMessage(result.error.message);
    
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    ReportDebugMessage(result.warnings[0].message);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(!blueprint.resourceGroups[0].resources[0].actions[0].examples.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body.empty());
}

TEST_CASE("Full syntax - No Indentation & No Newline", "[indentation]")
{
    // Blueprint in question:
    //R"(
    //## GET /1
    //+ Response 200
    //+ Body
    //{ ... }
    //");
    const std::string blueprintSource = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "+ Body\n"\
    "{ ... }\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    ReportDebugMessage(result.warnings[0].message);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(!blueprint.resourceGroups[0].resources[0].actions[0].examples.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body.empty());
}

TEST_CASE("Full syntax - Extra indentation", "[indentation]")
{
    // Blueprint in question:
    //R"(
    //## GET /1
    //+ Response 200
    //
    //        + Body
    //
    //                { ... }
    //");
    const std::string blueprintSource = \
    "## GET /1\n"\
    "+ Response 200\n"\
    "\n"\
    "        + Body\n"\
    "\n"\
    "                { ... }\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IndentationWarning);
    ReportDebugMessage(result.warnings[0].message);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(!blueprint.resourceGroups[0].resources[0].actions[0].examples.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "+ Body\n\n        { ... }\n");
}

