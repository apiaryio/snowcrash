//
//  test-ParameterParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 9/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "ParameterParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer ParameterFixture = \
"+ id = `1234` (optional, number, `0000`)\n\n"\
"    Lorem ipsum\n\n"\
"    + Values\n"\
"        + `1234`\n"\
"        + `0000`\n"\
"        + `beef`\n";

TEST_CASE("Recognize parameter definition signature", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(ParameterFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<Parameter>::sectionType(markdownAST.children().begin()) == ParameterSectionType);
}

TEST_CASE("Parse canonical parameter definition", "[parameter]")
{
    Parameter parameter;
    Report report;
    SectionParserHelper<Parameter, ParameterParser>::parse(ParameterFixture, ParameterSectionType, report, parameter);

    REQUIRE(report.error.code == Error::OK);
    CHECK(report.warnings.empty());

    REQUIRE(parameter.name == "id");
    REQUIRE(parameter.description == "Lorem ipsum\n");
    REQUIRE(parameter.type == "number");
    REQUIRE(parameter.use == OptionalParameterUse);
    REQUIRE(parameter.defaultValue == "1234");
    REQUIRE(parameter.exampleValue == "0000");
    REQUIRE(parameter.values.size() == 3);
    REQUIRE(parameter.values[0] == "1234");
    REQUIRE(parameter.values[1] == "0000");
    REQUIRE(parameter.values[2] == "beef");
}

// TODO: Move to test-ParametersParser.cc
//TEST_CASE("Parse ilegal parameter trait at the begining", "[parameter_definition][source]")
//{
//    const std::string blueprintSource = \
//    "# /1/{4}\n"\
//    "+ Parameters\n"\
//    "    + 4\n"\
//    "        + ilegal\n"\
//    "\n";
//
//    Parser parser;
//    Result result;
//    Blueprint blueprint;
//    parser.parse(blueprintSource, 0, result, blueprint);
//    REQUIRE(result.error.code == Error::OK);
//    REQUIRE(result.warnings.size() == 1);
//    REQUIRE(result.warnings[0].code == IgnoringWarning);
//
//    REQUIRE(blueprint.resourceGroups.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].description.empty());
//}

TEST_CASE("Warn when re-setting the values attribute", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id\n"\
    "    + Values\n"\
    "        + `Ahoy`\n"\
    "    + Values\n"\
    "        + `Hello`\n";

    Parameter parameter;
    Report report;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, report, parameter);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == RedefinitionWarning);

    REQUIRE(parameter.name == "id");
    REQUIRE(parameter.values.size() == 1);
    REQUIRE(parameter.values[0] == "Hello");
}

TEST_CASE("Warn when there are no values in the values attribute", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id\n"\
    "    + Values\n";

    Parameter parameter;
    Report report;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, report, parameter);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(parameter.name == "id");
    REQUIRE(parameter.values.empty());
}

TEST_CASE("Parse full abbreviated syntax", "[parameter]")
{
    mdp::ByteBuffer source = "+ limit = `20` (optional, number, `42`) ... This is a limit\n";

    Parameter parameter;
    Report report;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, report, parameter);

    REQUIRE(report.error.code == Error::OK);
    CHECK(report.warnings.empty());

    REQUIRE(parameter.name == "limit");
    REQUIRE(parameter.description == "This is a limit" );
    REQUIRE(parameter.defaultValue == "20");
    REQUIRE(parameter.exampleValue == "42");
    REQUIRE(parameter.type == "number");
    REQUIRE(parameter.use == OptionalParameterUse);
    REQUIRE(parameter.values.empty());
}

TEST_CASE("Warn on error in  abbreviated syntax attribute bracket", "[parameter]")
{
    mdp::ByteBuffer source = "+ limit (string1, string2, string3) ... This is a limit\n";

    Parameter parameter;
    Report report;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, report, parameter);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == FormattingWarning);

    REQUIRE(parameter.name == "limit");
    REQUIRE(parameter.description == "This is a limit" );
    REQUIRE(parameter.defaultValue.empty());
    REQUIRE(parameter.exampleValue.empty());
    REQUIRE(parameter.type.empty());
    REQUIRE(parameter.use == UndefinedParameterUse);
    REQUIRE(parameter.values.empty());
}

TEST_CASE("Warn about required vs default clash", "[parameter]")
{
    mdp::ByteBuffer source = "+ id = `42` (required)\n";

    Parameter parameter;
    Report report;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, report, parameter);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.name == "id");
    REQUIRE(parameter.use == RequiredParameterUse);
    REQUIRE(parameter.defaultValue == "42");
}

// TODO: Move to uri parser?
//TEST_CASE("Warn about implicit required vs default clash", "[parameter_definition][source]")
//{
//    const std::string blueprintSource = \
//    "# /1/{id}\n"\
//    "+ Parameters\n"\
//    "    + id = `42`\n"\
//    "\n";
//
//    Parser parser;
//    Result result;
//    Blueprint blueprint;
//    parser.parse(blueprintSource, 0, result, blueprint);
//    REQUIRE(result.error.code == Error::OK);
//    REQUIRE(result.warnings.size() == 1);
//    REQUIRE(result.warnings[0].code == LogicalErrorWarning);
//
//    REQUIRE(blueprint.resourceGroups.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].description.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].name == "id");
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].use == UndefinedParameterUse);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].defaultValue == "42");
//}

// TODO: How to warn for this?
//TEST_CASE("Unrecognized 'values' keyword", "[parameter]")
//{
//    mdp::ByteBuffer source = \
//    "+ param\n"\
//    "    + Values:\n"\
//    "        + `lorem`\n";
//
//    Parameter parameter;
//    Report report;
//    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, report, parameter);
//
//    REQUIRE(report.error.code == Error::OK);
//    REQUIRE(report.warnings.size() == 1);
//    REQUIRE(report.warnings[0].code == IgnoringWarning);
//
//    REQUIRE(parameter.name == "param");
//    REQUIRE(parameter.use == UndefinedParameterUse);
//    REQUIRE(parameter.values.empty());
//}

TEST_CASE("warn missing example item in values", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id = `Value2` (optional, string, `Value1`)\n"\
    "    + Values\n"\
    "        + `Value2`\n";

    Parameter parameter;
    Report report;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, report, parameter);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.name == "id");
    REQUIRE(parameter.exampleValue == "Value1");
    REQUIRE(parameter.defaultValue == "Value2");
}

TEST_CASE("warn missing default value in values", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id = `Value1` (optional, string, `Value2`)\n"\
    "    + Values\n"\
    "        + `Value2`\n";

    Parameter parameter;
    Report report;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, report, parameter);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.name == "id");
    REQUIRE(parameter.exampleValue == "Value2");
    REQUIRE(parameter.defaultValue == "Value1");
}
