//
//  test-ParametersParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 9/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "ParametersParser.h"
#include "snowcrash.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer ParametersFixture = \
"+ Parameters\n"\
"    + id = `1234` (optional, number, `0000`)\n\n"\
"        Lorem ipsum\n"\
"        + Values\n"\
"            + `1234`\n"\
"            + `0000`\n"\
"            + `beef`\n"\
"    + name\n";

TEST_CASE("Recognize Parameters section block", "[parameters]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(ParametersFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameters>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ParametersSectionType);
}

TEST_CASE("Parse canonical parameters", "[parameters]")
{
    ParseResult<Parameters> parameters;
    SectionParserHelper<Parameters, ParametersParser>::parse(ParametersFixture, ParametersSectionType, parameters);

    REQUIRE(parameters.report.error.code == Error::OK);
    REQUIRE(parameters.report.warnings.empty());

    REQUIRE(parameters.node.size() == 2);

    REQUIRE(parameters.node[0].name == "id");
    REQUIRE(parameters.node[0].description == "Lorem ipsum\n");

    REQUIRE(parameters.node[1].name == "name");
    REQUIRE(parameters.node[1].description.empty());
}

TEST_CASE("Parse ilegal parameter", "[parameters]")
{
    mdp::ByteBuffer source = \
    "+ Parameters\n"\
    "    + i:legal\n\n";

    ParseResult<Parameters> parameters;
    SectionParserHelper<Parameters, ParametersParser>::parse(source, ParametersSectionType, parameters);

    REQUIRE(parameters.report.error.code == Error::OK);
    REQUIRE(parameters.report.warnings.size() == 2);
    REQUIRE(parameters.report.warnings[0].code == IgnoringWarning);
    REQUIRE(parameters.report.warnings[1].code == FormattingWarning);

    REQUIRE(parameters.node.empty());
}

TEST_CASE("Parse illegal parameter among legal ones", "[parameters]")
{
    mdp::ByteBuffer source = \
    "+ Parameters\n"\
    "    + OK_1\n"\
    "    + i:legal\n"\
    "    + OK-2\n";

    ParseResult<Parameters> parameters;
    SectionParserHelper<Parameters, ParametersParser>::parse(source, ParametersSectionType, parameters);

    REQUIRE(parameters.report.error.code == Error::OK);
    REQUIRE(parameters.report.warnings.size() == 1);
    REQUIRE(parameters.report.warnings[0].code == IgnoringWarning);

    REQUIRE(parameters.node.size() == 2);
    REQUIRE(parameters.node[0].name == "OK_1");
    REQUIRE(parameters.node[0].description.empty());
    REQUIRE(parameters.node[1].name == "OK-2");
    REQUIRE(parameters.node[1].description.empty());
}

TEST_CASE("Warn about additional content in parameters section", "[parameters]")
{
    mdp::ByteBuffer source = \
    "+ Parameters\n"\
    "  extra-1\n\n"\
    "    + id\n";

    ParseResult<Parameters> parameters;
    SectionParserHelper<Parameters, ParametersParser>::parse(source, ParametersSectionType, parameters);

    REQUIRE(parameters.report.error.code == Error::OK);
    REQUIRE(parameters.report.warnings.size() == 1);
    REQUIRE(parameters.report.warnings[0].code == IgnoringWarning);

    REQUIRE(parameters.node.size() == 1);
    REQUIRE(parameters.node[0].name == "id");
    REQUIRE(parameters.node[0].description.empty());
}

TEST_CASE("Warn about additional content block in parameters section", "[parameters]")
{
    mdp::ByteBuffer source = \
    "+ Parameters\n\n"\
    "  extra-1\n\n"\
    "    + id\n";

    ParseResult<Parameters> parameters;
    SectionParserHelper<Parameters, ParametersParser>::parse(source, ParametersSectionType, parameters);

    REQUIRE(parameters.report.error.code == Error::OK);
    REQUIRE(parameters.report.warnings.size() == 1);
    REQUIRE(parameters.report.warnings[0].code == IgnoringWarning);

    REQUIRE(parameters.node.size() == 1);
    REQUIRE(parameters.node[0].name == "id");
    REQUIRE(parameters.node[0].description.empty());
}

TEST_CASE("Warn about multiple parameters with the same name", "[parameters]")
{
    mdp::ByteBuffer source = \
    "+ Parameters\n"\
    "    + id (`42`)\n"\
    "    + id (`43`)\n";

    ParseResult<Parameters> parameters;
    SectionParserHelper<Parameters, ParametersParser>::parse(source, ParametersSectionType, parameters);

    REQUIRE(parameters.report.error.code == Error::OK);
    REQUIRE(parameters.report.warnings.size() == 1);
    REQUIRE(parameters.report.warnings[0].code == RedefinitionWarning);

    REQUIRE(parameters.node.size() == 2);
    REQUIRE(parameters.node[0].name == "id");
    REQUIRE(parameters.node[0].exampleValue == "42");

    REQUIRE(parameters.node[1].name == "id");
    REQUIRE(parameters.node[1].exampleValue == "43");
}

TEST_CASE("Recognize parameter when there is no description on its signature and remaining description is not a new node", "[parameters]")
{
    mdp::ByteBuffer source = \
    "+ Parameters\n\n"\
    "    + id (number) ... The ID number of the car\n"\
    "    + state (string)\n"\
    "        The desired state of the panoramic roof. The approximate percent open values for each state are `open` = 100%, `close` = 0%, `comfort` = 80%, and `vent` = ~15%\n"\
    "        + Values\n"\
    "            + `open`\n"\
    "            + `close`\n"\
    "            + `comfort`\n"\
    "            + `vent`";

    ParseResult<Parameters> parameters;
    SectionParserHelper<Parameters, ParametersParser>::parse(source, ParametersSectionType, parameters);

    REQUIRE(parameters.report.error.code == Error::OK);
    REQUIRE(parameters.report.warnings.empty());

    REQUIRE(parameters.node.size() == 2);
    REQUIRE(parameters.node[0].name == "id");
    REQUIRE(parameters.node[0].type == "number");
    REQUIRE(parameters.node[0].description == "The ID number of the car");

    Parameter parameter = parameters[1];
    REQUIRE(parameter.name == "state");
    REQUIRE(parameter.type == "string");
    REQUIRE(parameter.description == "\nThe desired state of the panoramic roof. The approximate percent open values for each state are `open` = 100%, `close` = 0%, `comfort` = 80%, and `vent` = ~15%\n\n");
    REQUIRE(parameter.values.size() == 4);
    REQUIRE(parameter.values[0] == "open");
    REQUIRE(parameter.values[1] == "close");
    REQUIRE(parameter.values[2] == "comfort");
    REQUIRE(parameter.values[3] == "vent");
}

TEST_CASE("Parentheses in parameter example ", "[parameters][issue][#109]")
{
    // Blueprint in question:
    //R"(
    //# GET /{id}
    //+ Parameters
    //  + id (optional, oData, `substringof('homer', id)`) ... test
    //
    //+ response 204
    //");
    mdp::ByteBuffer source = \
    "+ Parameters\n"\
    "  + id (optional, oData, `substringof('homer', id)`) ... test\n"\
    "\n";

    ParseResult<Parameters> parameters;
    SectionParserHelper<Parameters, ParametersParser>::parse(source, ParametersSectionType, parameters);
    
    REQUIRE(parameters.report.error.code == Error::OK);
    REQUIRE(parameters.report.warnings.empty());

    REQUIRE(parameters.node.size() == 1);
    REQUIRE(parameters.node[0].name == "id");
    REQUIRE(parameters.node[0].type == "oData");
    REQUIRE(parameters.node[0].exampleValue == "substringof('homer', id)");
    REQUIRE(parameters.node[0].description == "test");
}

TEST_CASE("Percentage encoded characters in parameter name ", "[parameters][percentageencoding][issue][#107]")
{
    // Blueprint in question:
    //R"(
    //# GET /{id%5b%5d}
    //+ Parameters
    //  + id%5b%5d (optional, oData, `substringof('homer', id)`) ... test
    //
    //+ response 204
    //");
    mdp::ByteBuffer source = \
    "# GET /{id%5b%5d}\n"\
    "+ Parameters\n"\
    "  + id%5b%5d (optional, oData, `substringof('homer', id)`) ... test\n"\
    "\n"\
    "+ response 204\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    
    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters[0].name == "id%5b%5d");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters[0].type == "oData");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters[0].exampleValue == "substringof('homer', id)");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters[0].description == "test");
}

TEST_CASE("Invalid percentage encoded characters in parameter name ", "[invalid][parameters][percentageencoding][issue][#107]")
{
    // Blueprint in question:
    //R"(
    //# GET /{id%5z}
    //+ Parameters
    //  + id%5z (optional, oData, `substringof('homer', id)`) ... test
    //
    //+ response 204
    //");
    mdp::ByteBuffer source = \
    "# GET /{id%5z}\n"\
    "+ Parameters\n"\
    "  + id%5z (optional, oData, `substringof('homer', id)`) ... test\n"\
    "\n"\
    "+ response 204\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 3);
}

TEST_CASE("Incomplete percentage encoded characters in parameter name ", "[incomplete][parameters][percentageencoding][issue][#107]")
{
    // Blueprint in question:
    //R"(
    //# GET /{id%}
    //+ Parameters
    //  + id% (optional, oData, `substringof('homer', id)`) ... test
    //
    //+ response 204
    //");
    mdp::ByteBuffer source = \
    "# GET /{id%}\n"\
    "+ Parameters\n"\
    "  + id% (optional, oData, `substringof('homer', id)`) ... test\n"\
    "\n"\
    "+ response 204\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 3);
}
