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
    SectionType sectionType = SectionProcessor<Parameters, ParametersSM>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ParametersSectionType);
}

TEST_CASE("Parse canonical parameters", "[parameters]")
{
    Parameters parameters;
    Report report;
    SectionParserHelper<Parameters, ParametersSM, ParametersParser>::parse(ParametersFixture, ParametersSectionType, report, parameters);

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
    SectionParserHelper<Parameters, ParametersSM, ParametersParser>::parse(source, ParametersSectionType, report, parameters);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 2);
    REQUIRE(report.warnings[0].code == IgnoringWarning);
    REQUIRE(report.warnings[1].code == FormattingWarning);

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
    SectionParserHelper<Parameters, ParametersSM, ParametersParser>::parse(source, ParametersSectionType, report, parameters);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IgnoringWarning);

    REQUIRE(parameters.size() == 2);
    REQUIRE(parameters[0].name == "OK_1");
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
    SectionParserHelper<Parameters, ParametersSM, ParametersParser>::parse(source, ParametersSectionType, report, parameters);

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
    SectionParserHelper<Parameters, ParametersSM, ParametersParser>::parse(source, ParametersSectionType, report, parameters);

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
    SectionParserHelper<Parameters, ParametersSM, ParametersParser>::parse(source, ParametersSectionType, report, parameters);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == RedefinitionWarning);

    REQUIRE(parameters.size() == 1);
    REQUIRE(parameters[0].name == "id");
    REQUIRE(parameters[0].exampleValue == "43");
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

    Parameters parameters;
    Report report;
    SectionParserHelper<Parameters, ParametersSM, ParametersParser>::parse(source, ParametersSectionType, report, parameters);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(parameters.size() == 2);
    REQUIRE(parameters[0].name == "id");
    REQUIRE(parameters[0].type == "number");
    REQUIRE(parameters[0].description == "The ID number of the car");

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
