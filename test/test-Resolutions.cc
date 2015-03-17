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

TEST_CASE("Resolve square bracket warning", "[resolutions][square brackets]")
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
    parse(source, snowcrash::ResolveWarningsAndErrorsOption, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == URIWarning);
    REQUIRE(blueprint.report.warnings[0].subCode == SquareBracketWarningUriTemplateWarningSubCode);
    REQUIRE(blueprint.report.warnings[0].resolutions.size() == 2);
    REQUIRE(blueprint.report.warnings[0].resolutions[0].message == "use %5B instead of '['");
    REQUIRE(blueprint.report.warnings[0].resolutions[0].resolvedSource == "%5B");
    REQUIRE(blueprint.report.warnings[0].resolutions[1].message == "use %5D instead of ']'");
    REQUIRE(blueprint.report.warnings[0].resolutions[1].resolvedSource == "%5D");
    REQUIRE(source[blueprint.report.warnings[0].resolutions[0].location.location] == '[');
    REQUIRE(source[blueprint.report.warnings[0].resolutions[1].location.location] == ']');
}


TEST_CASE("Resolve expression contains spaces warning", "[resolutions][expression contains spaces]")
{
    mdp::ByteBuffer source = \
        "FORMAT: 1A\n"\
        "\n"\
        "# test api\n"\
        "this is a description of the test api\n"\
        "# Group test\n"\
        "A test group\n"\
        "## test [/test/{id, id2}]\n"\
        "A test uri template\n";

    ParseResult<Blueprint> blueprint;
    parse(source, snowcrash::ResolveWarningsAndErrorsOption, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == URIWarning);
    REQUIRE(blueprint.report.warnings[0].subCode == ContainsSpacesWarningUriTemplateWarningSubCode);
    REQUIRE(blueprint.report.warnings[0].resolutions.size() == 1);
    REQUIRE(blueprint.report.warnings[0].resolutions[0].message == "remove spaces from expressions");
    REQUIRE(blueprint.report.warnings[0].resolutions[0].resolvedSource == "{id,id2}");
}


TEST_CASE("Resolve invalid characters warning", "[resolutions][invalid characters]")
{
    mdp::ByteBuffer source = \
        "FORMAT: 1A\n"\
        "\n"\
        "# test api\n"\
        "this is a description of the test api\n"\
        "# Group test\n"\
        "A test group\n"\
        "## test [/test/{val1,~val}]\n"\
        "A test uri template\n";

    ParseResult<Blueprint> blueprint;
    parse(source, snowcrash::ResolveWarningsAndErrorsOption, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == URIWarning);
    REQUIRE(blueprint.report.warnings[0].subCode == InvalidCharactersWarningUriTemplateWarningSubCode);
    REQUIRE(blueprint.report.warnings[0].resolutions.size() == 1);
    REQUIRE(blueprint.report.warnings[0].resolutions[0].message == "replace '~' with '%7e'");
    REQUIRE(blueprint.report.warnings[0].resolutions[0].resolvedSource == "%7e");
    REQUIRE(source[blueprint.report.warnings[0].resolutions[0].location.location] == '~');
}

TEST_CASE("Resolve multiple invalid characters warning", "[resolutions][invalid characters]")
{
    mdp::ByteBuffer source = \
        "FORMAT: 1A\n"\
        "\n"\
        "# test api\n"\
        "this is a description of the test api\n"\
        "# Group test\n"\
        "A test group\n"\
        "## test [/test/{val1,~val~}]\n"\
        "A test uri template\n";

    ParseResult<Blueprint> blueprint;
    parse(source, snowcrash::ResolveWarningsAndErrorsOption, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == URIWarning);
    REQUIRE(blueprint.report.warnings[0].subCode == InvalidCharactersWarningUriTemplateWarningSubCode);
    REQUIRE(blueprint.report.warnings[0].resolutions.size() == 2);
    REQUIRE(blueprint.report.warnings[0].resolutions[0].message == "replace '~' with '%7e'");
    REQUIRE(blueprint.report.warnings[0].resolutions[0].resolvedSource == "%7e");
    REQUIRE(source[blueprint.report.warnings[0].resolutions[0].location.location] == '~');
    REQUIRE(blueprint.report.warnings[0].resolutions[1].message == "replace '~' with '%7e'");
    REQUIRE(blueprint.report.warnings[0].resolutions[1].resolvedSource == "%7e");
    REQUIRE(source[blueprint.report.warnings[0].resolutions[1].location.location] == '~');
}
