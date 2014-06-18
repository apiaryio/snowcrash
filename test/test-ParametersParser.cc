//
//  test-ParametersParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 9/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "ParametersParser.h"

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
    REQUIRE(SectionProcessor<Parameters>::sectionType(markdownAST.children().begin()) == ParametersSectionType);
}

TEST_CASE("Parse canonical parameters", "[parameters]")
{
    Parameters parameters;
    Report report;
    SectionParserHelper<Parameters, ParametersParser>::parse(ParametersFixture, ParametersSectionType, report, parameters);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(parameters.size() == 2);

    REQUIRE(parameters[0].name == "id");
    REQUIRE(parameters[0].description == "Lorem ipsum\n");

    REQUIRE(parameters[1].name == "name");
    REQUIRE(parameters[1].description.empty());
}

TEST_CASE("Parse ilegal parameter", "[parameters]")
{
    mdp::ByteBuffer source = \
    "+ Parameters\n"\
    "    + i:legal\n\n";

    Parameters parameters;
    Report report;
    SectionParserHelper<Parameters, ParametersParser>::parse(source, ParametersSectionType, report, parameters);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IgnoringWarning);

    REQUIRE(parameters.empty());
}

TEST_CASE("Parse illegal parameter among legal ones", "[parameters]")
{
    mdp::ByteBuffer source = \
    "+ Parameters\n"\
    "    + OK_1\n"\
    "    + i:legal\n"\
    "    + OK-2\n";

    Parameters parameters;
    Report report;
    SectionParserHelper<Parameters, ParametersParser>::parse(source, ParametersSectionType, report, parameters);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IgnoringWarning);

    REQUIRE(parameters.size() == 2);
    REQUIRE(parameters[0].name == "OK-1");
    REQUIRE(parameters[0].description.empty());
    REQUIRE(parameters[1].name == "OK-2");
    REQUIRE(parameters[1].description.empty());
}

TEST_CASE("Warn about additional content in parameters section", "[parameters]")
{
    mdp::ByteBuffer source = \
    "+ Parameters\n"\
    "  extra-1\n\n"\
    "    + id\n";

    Parameters parameters;
    Report report;
    SectionParserHelper<Parameters, ParametersParser>::parse(source, ParametersSectionType, report, parameters);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IgnoringWarning);

    REQUIRE(parameters.size() == 1);
    REQUIRE(parameters[0].name == "id");
    REQUIRE(parameters[0].description.empty());
}

TEST_CASE("Warn about additional content block in parameters section", "[parameters]")
{
    mdp::ByteBuffer source = \
    "+ Parameters\n\n"\
    "  extra-1\n\n"\
    "    + id\n";

    Parameters parameters;
    Report report;
    SectionParserHelper<Parameters, ParametersParser>::parse(source, ParametersSectionType, report, parameters);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IgnoringWarning);

    REQUIRE(parameters.size() == 1);
    REQUIRE(parameters[0].name == "id");
    REQUIRE(parameters[0].description.empty());
}

TEST_CASE("Warn about multiple parameters with the same name", "[parameters]")
{
    mdp::ByteBuffer source = \
    "+ Parameters\n"\
    "    + id (`42`)\n"\
    "    + id (`43`)\n";

    Parameters parameters;
    Report report;
    SectionParserHelper<Parameters, ParametersParser>::parse(source, ParametersSectionType, report, parameters);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == RedefinitionWarning);

    REQUIRE(parameters.size() == 1);
    REQUIRE(parameters[0].name == "id");
    REQUIRE(parameters[0].exampleValue == "43");
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
    const std::string blueprintSource = \
        "# GET /{id}\n"\
        "+ Parameters\n"\
        "  + id (optional, oData, `substringof('homer', id)`) ... test\n"\
        "\n"\
        "+ response 204\n";

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
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].type == "oData");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].exampleValue == "substringof('homer', id)");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].description == "test");
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
    const std::string blueprintSource = \
        "# GET /{id%5b%5d}\n"\
        "+ Parameters\n"\
        "  + id%5b%5d (optional, oData, `substringof('homer', id)`) ... test\n"\
        "\n"\
        "+ response 204\n";

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
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "id%5b%5d");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].type == "oData");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].exampleValue == "substringof('homer', id)");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].description == "test");
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
    const std::string blueprintSource = \
        "# GET /{id%5z}\n"\
        "+ Parameters\n"\
        "  + id%5z (optional, oData, `substringof('homer', id)`) ... test\n"\
        "\n"\
        "+ response 204\n";

    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size()==3);
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
    const std::string blueprintSource = \
        "# GET /{id%}\n"\
        "+ Parameters\n"\
        "  + id% (optional, oData, `substringof('homer', id)`) ... test\n"\
        "\n"\
        "+ response 204\n";

    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 3);
}
