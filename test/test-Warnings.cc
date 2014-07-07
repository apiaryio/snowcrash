//
//  test-Warnings.c
//  snowcrash
//
//  Created by Zdenek Nemec on 11/12/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iterator>
#include <iostream>
#include "catch.hpp"
#include "Fixture.h"
#include "Parser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Warn about keywords in API name", "[warnings][#31]")
{
    // Blueprint in question:
    //R"(
    //FORMAT: X-1A
    //
    //# POST
    //");
    const std::string blueprintSource = \
    "FORMAT: 1A\n"\
    "\n"\
    "## POST\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    // TODO:
//    REQUIRE(result.warnings.size() == 1);
//    REQUIRE(result.warnings[0].code == AmbiguityWarning);
//    
//    REQUIRE(blueprint.resourceGroups.empty());
}

TEST_CASE("Warn about brackets in URI template", "[bracketwarnings][issue][#79]")
{
    // Blueprint in question:
    //R"(
    //"FORMAT: 1A\n"\
    //
    //# test api
    //this is a description of the test api
    //# Group test
    //A test group
    //## test [/test/{id}[2]]
    //"A test uri template
    //");
    const std::string blueprintSource = \
        "FORMAT: 1A\n"\
        "\n"\
        "# test api\n"\
        "this is a description of the test api\n"\
        "# Group test\n"\
        "A test group\n"\
        "## test [/test/{id}[2]]\n"\
        "A test uri template\n";

    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.warnings.size() == 1);
}

TEST_CASE("Warn about unsupported uri template label expansion", "[unsupportedlabelexpansion][issue][#78]")
{
    // Blueprint in question:
    //R"(
    //"FORMAT: 1A\n"\
        //
    //# test api
    //this is a description of the test api
    //# Group test
    //A test group
    //## test [/test/{.varone}]
    //"A test uri template
    //");
    const std::string blueprintSource = \
        "FORMAT: 1A\n"\
        "\n"\
        "# test api\n"\
        "this is a description of the test api\n"\
        "# Group test\n"\
        "A test group\n"\
        "## test [/test/{.varone}]\n"\
        "A test uri template\n";

    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.warnings.size() == 1);
}

TEST_CASE("Warn about unsupported uri template in abbreviated blueprint","[abbreviatedblueprint][issue][#78]")
{
    const std::string blueprintSource = \
        "FORMAT: 1A\n"\
        "\n"\
        "# GET /res/{id}{?a,    b}\n"\
        "+ Response 200";

    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.warnings.size() == 1);
}

