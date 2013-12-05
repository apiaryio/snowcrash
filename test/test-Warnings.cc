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
    "FORMAT: X-1A\n"\
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
