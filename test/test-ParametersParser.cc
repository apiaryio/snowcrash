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

TEST_CASE("Parse multiple parameters", "[parameters]")
{
    mdp::ByteBuffer source = \
    "+ Parameters\n"\
    "    + id (`42`)\n"\
    "    + id2 (`43`)\n";
    
    Parameters parameters;
    Report report;
    SectionParserHelper<Parameters, ParametersParser>::parse(source, ParametersSectionType, report, parameters);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(parameters.size() == 2);
    REQUIRE(parameters[0].name == "id");
    REQUIRE(parameters[0].exampleValue == "42");
    
    REQUIRE(parameters[1].name == "id2");
    REQUIRE(parameters[1].exampleValue == "43");
}
