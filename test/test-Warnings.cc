//
//  test-Warnings.c
//  snowcrash
//
//  Created by Zdenek Nemec on 11/12/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrash.h"
#include "snowcrashtest.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Warn about keywords in API name", "[warnings][#31]")
{
    mdp::ByteBuffer source = \
    "FORMAT: 1A\n"\
    "\n"\
    "# POST\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);

//TODO:
//    REQUIRE(blueprint.report.warnings.size() == 1);
//    REQUIRE(blueprint.report.warnings[0].code == AmbiguityWarning);

//    REQUIRE(blueprint.name.empty());
    REQUIRE(blueprint.node.content.elements().empty());
}

TEST_CASE("Warn about brackets in URI template", "[warnings][#79]")
{
    mdp::ByteBuffer source = \
    "FORMAT: 1A\n"\
    "\n"\
    "# test api\n"\
    "this is a description of the test api\n"\
    "# Group test\n"\
    "A test group\n"\
    "## test [/test/{id}[2]]\n"\
    "A test uri template\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == URIWarning);
}

TEST_CASE("Warn about unsupported uri template label expansion", "[warnings][#78]")
{
    mdp::ByteBuffer source = \
    "FORMAT: 1A\n"\
    "\n"\
    "# test api\n"\
    "this is a description of the test api\n"\
    "# Group test\n"\
    "A test group\n"\
    "## test [/test/{.varone}]\n"\
    "A test uri template\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == URIWarning);
}

TEST_CASE("Warn about unsupported uri template in abbreviated blueprint", "[warnings][#78]")
{
    mdp::ByteBuffer source = \
    "FORMAT: 1A\n"\
    "\n"\
    "# test api\n"\
    "this is a description of the test api\n"\
    "# Group test\n"\
    "A test group\n"\
    "## GET /res/{id}{?a,    b}\n"\
    "+ Response 200";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == URIWarning);
}

