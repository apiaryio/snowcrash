//
//  test-SymbolIdentifier.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 2/7/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrash.h"
#include "snowcrashtest.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Punctuation in identifiers", "[symbol_identifier]")
{
    mdp::ByteBuffer source = "# Parcel's sticker @#!$%^&*=-?><,.~`\"' [/]\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).element == Element::ResourceElement);

    Resource resource = blueprint.node.content.elements().at(0).content.elements().at(0).content.resource;
    REQUIRE(resource.name == "Parcel's sticker @#!$%^&*=-?><,.~`\"'");
    REQUIRE(resource.uriTemplate == "/");
    REQUIRE(resource.actions.empty());
}

TEST_CASE("Non ASCII characters in identifiers", "[symbol_identifier]")
{
    // Blueprint in question:
    //R"(
    //# Kategorii [/]
    //");

    // "Kategorii in Russian"
    mdp::ByteBuffer source = "# \xD0\x9A\xD0\xB0\xD1\x82\xD0\xB5\xD0\xB3\xD0\xBE\xD1\x80\xD0\xB8\xD0\xB8 [/]\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).element == Element::ResourceElement);

    Resource resource = blueprint.node.content.elements().at(0).content.elements().at(0).content.resource;
    REQUIRE(resource.name == "\xD0\x9A\xD0\xB0\xD1\x82\xD0\xB5\xD0\xB3\xD0\xBE\xD1\x80\xD0\xB8\xD0\xB8");
    REQUIRE(resource.uriTemplate == "/");
    REQUIRE(resource.actions.empty());
}
