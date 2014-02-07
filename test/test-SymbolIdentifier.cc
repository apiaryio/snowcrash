//
//  test-SymbolIdentifier.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 2/7/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "Fixture.h"
#include "Parser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Punctuation in identifiers", "[symbol][identifier][source]")
{
    // Blueprint in question:
    //R"(
    //# Parcel's sticker @#!$%^&*=-?><,.~`\"' [/]
    //");
    
    const std::string blueprintSource = \
    "# Parcel's sticker @#!$%^&*=-?><,.~`\"' [/]\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].name == "Parcel's sticker @#!$%^&*=-?><,.~`\"'");
    REQUIRE(blueprint.resourceGroups[0].resources[0].uriTemplate == "/");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
}

TEST_CASE("Non ASCII characters in identifiers", "[symbol][identifier][issue][#66][source]")
{
    // Blueprint in question:
    //R"(
    //# Kategorii [/]
    //");
    
    // "Kategorii in Russian"
    const std::string blueprintSource = \
    "# \xD0\x9A\xD0\xB0\xD1\x82\xD0\xB5\xD0\xB3\xD0\xBE\xD1\x80\xD0\xB8\xD0\xB8 [/]\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].name == "\xD0\x9A\xD0\xB0\xD1\x82\xD0\xB5\xD0\xB3\xD0\xBE\xD1\x80\xD0\xB8\xD0\xB8");
    REQUIRE(blueprint.resourceGroups[0].resources[0].uriTemplate == "/");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
}
