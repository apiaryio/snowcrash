//
//  test-Indentation.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 10/31/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "Fixture.h"
#include "Parser.h"

using namespace snowcrash;
using namespace snowcrashtest;

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
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "{ ... }\n");
}

TEST_CASE("No Indentation", "[abbreviated][indentation][now]")
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
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "{ ... }\n");
}
