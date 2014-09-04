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
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ParameterSectionType);
}

TEST_CASE("Parse canonical parameter definition", "[parameter]")
{
    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(ParameterFixture, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    CHECK(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.description == "Lorem ipsum\n");
    REQUIRE(parameter.node.type == "number");
    REQUIRE(parameter.node.use == OptionalParameterUse);
    REQUIRE(parameter.node.defaultValue == "1234");
    REQUIRE(parameter.node.exampleValue == "0000");
    REQUIRE(parameter.node.values.size() == 3);
    REQUIRE(parameter.node.values[0] == "1234");
    REQUIRE(parameter.node.values[1] == "0000");
    REQUIRE(parameter.node.values[2] == "beef");

    REQUIRE(parameter.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].length == 40);
    REQUIRE(parameter.sourceMap.type.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.type.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.type.sourceMap[0].length == 40);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap[0].length == 40);
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap[0].length == 40);
    REQUIRE(parameter.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].location == 46);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].length == 12);
    REQUIRE(parameter.sourceMap.use.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.use.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.use.sourceMap[0].length == 40);
    REQUIRE(parameter.sourceMap.values.list.size() == 3);
    REQUIRE(parameter.sourceMap.values.list[0].sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.values.list[0].sourceMap[0].location == 80);
    REQUIRE(parameter.sourceMap.values.list[0].sourceMap[0].length == 9);
    REQUIRE(parameter.sourceMap.values.list[1].sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.values.list[1].sourceMap[0].location == 97);
    REQUIRE(parameter.sourceMap.values.list[1].sourceMap[0].length == 9);
    REQUIRE(parameter.sourceMap.values.list[2].sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.values.list[2].sourceMap[0].location == 114);
    REQUIRE(parameter.sourceMap.values.list[2].sourceMap[0].length == 9);
}

TEST_CASE("Warn when re-setting the values attribute", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id\n"\
    "    + Values\n"\
    "        + `Ahoy`\n"\
    "    + Values\n"\
    "        + `Hello`\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == RedefinitionWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.values.size() == 1);
    REQUIRE(parameter.node.values[0] == "Hello");
    REQUIRE(parameter.node.use == UndefinedParameterUse);

    REQUIRE(parameter.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].length == 3);
    REQUIRE(parameter.sourceMap.values.list.size() == 1);
    REQUIRE(parameter.sourceMap.values.list[0].sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.values.list[0].sourceMap[0].location == 56);
    REQUIRE(parameter.sourceMap.values.list[0].sourceMap[0].length == 10);
    REQUIRE(parameter.sourceMap.use.sourceMap.size() == 0);
}

TEST_CASE("Warn when there are no values in the values attribute", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id\n"\
    "    + Values\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.values.empty());
}

TEST_CASE("Parse full abbreviated syntax", "[parameter]")
{
    mdp::ByteBuffer source = "+ limit = `20` (optional, number, `42`) ... This is a limit\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    CHECK(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "limit");
    REQUIRE(parameter.node.description == "This is a limit" );
    REQUIRE(parameter.node.defaultValue == "20");
    REQUIRE(parameter.node.exampleValue == "42");
    REQUIRE(parameter.node.type == "number");
    REQUIRE(parameter.node.use == OptionalParameterUse);
    REQUIRE(parameter.node.values.empty());

    REQUIRE(parameter.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].length == 58);
    REQUIRE(parameter.sourceMap.type.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.type.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.type.sourceMap[0].length == 58);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap[0].length == 58);
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap[0].length == 58);
    REQUIRE(parameter.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].length == 58);
    REQUIRE(parameter.sourceMap.use.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.use.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.use.sourceMap[0].length == 58);
    REQUIRE(parameter.sourceMap.values.list.empty());
}

TEST_CASE("Warn on error in  abbreviated syntax attribute bracket", "[parameter]")
{
    mdp::ByteBuffer source = "+ limit (string1, string2, string3) ... This is a limit\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == FormattingWarning);

    REQUIRE(parameter.node.name == "limit");
    REQUIRE(parameter.node.description == "This is a limit" );
    REQUIRE(parameter.node.defaultValue.empty());
    REQUIRE(parameter.node.exampleValue.empty());
    REQUIRE(parameter.node.type.empty());
    REQUIRE(parameter.node.use == UndefinedParameterUse);
    REQUIRE(parameter.node.values.empty());

    REQUIRE(parameter.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].length == 54);
    REQUIRE(parameter.sourceMap.type.sourceMap.empty());
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].length == 54);
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.list.empty());
}

TEST_CASE("Warn about required vs default clash", "[parameter]")
{
    mdp::ByteBuffer source = "+ id = `42` (required)\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.use == RequiredParameterUse);
    REQUIRE(parameter.node.defaultValue == "42");

    REQUIRE(parameter.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].length == 21);
    REQUIRE(parameter.sourceMap.type.sourceMap.empty());
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap[0].length == 21);
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.description.sourceMap.empty());
    REQUIRE(parameter.sourceMap.use.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.use.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.use.sourceMap[0].length == 21);
    REQUIRE(parameter.sourceMap.values.list.empty());
}

TEST_CASE("Warn about implicit required vs default clash", "[parameter_definition][source]")
{
    mdp::ByteBuffer source = "+ id = `42`\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.use == UndefinedParameterUse);
    REQUIRE(parameter.node.defaultValue == "42");

    REQUIRE(parameter.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].length == 10);
    REQUIRE(parameter.sourceMap.type.sourceMap.empty());
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap[0].length == 10);
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.description.sourceMap.empty());
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.list.empty());
}

TEST_CASE("Unrecognized 'values' keyword", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ param\n"\
    "    + Values:\n"\
    "        + `lorem`\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "param");
    REQUIRE(parameter.node.description == "+ Values:\n    + `lorem`\n");
    REQUIRE(parameter.node.use == UndefinedParameterUse);
    REQUIRE(parameter.node.values.empty());

    REQUIRE(parameter.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].length == 6);
    REQUIRE(parameter.sourceMap.type.sourceMap.empty());
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.description.sourceMap.size() == 2);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].location == 12);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].length == 10);
    REQUIRE(parameter.sourceMap.description.sourceMap[1].location == 26);
    REQUIRE(parameter.sourceMap.description.sourceMap[1].length == 14);
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.list.empty());
}

TEST_CASE("warn missing example item in values", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id = `Value2` (optional, string, `Value1`)\n"\
    "    + Values\n"\
    "        + `Value2`\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.exampleValue == "Value1");
    REQUIRE(parameter.node.defaultValue == "Value2");

    REQUIRE(parameter.sourceMap.values.list.size() == 1);
    REQUIRE(parameter.sourceMap.values.list[0].sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.values.list[0].sourceMap[0].location == 66);
    REQUIRE(parameter.sourceMap.values.list[0].sourceMap[0].length == 11);
}

TEST_CASE("warn missing default value in values", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id = `Value1` (optional, string, `Value2`)\n"\
    "    + Values\n"\
    "        + `Value2`\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.exampleValue == "Value2");
    REQUIRE(parameter.node.defaultValue == "Value1");

    REQUIRE(parameter.sourceMap.values.list.size() == 1);
    REQUIRE(parameter.sourceMap.values.list[0].sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.values.list[0].sourceMap[0].location == 66);
    REQUIRE(parameter.sourceMap.values.list[0].sourceMap[0].length == 11);
}

TEST_CASE("Parse parameters with dot in its name", "[parameter]")
{
    mdp::ByteBuffer source = "+ product.id ... Hello\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "product.id");
    REQUIRE(parameter.node.description == "Hello");

    REQUIRE(parameter.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].length == 21);
    REQUIRE(parameter.sourceMap.type.sourceMap.empty());
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].length == 21);
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.list.empty());
}

TEST_CASE("Parentheses in parameter description", "[parameter]")
{
    mdp::ByteBuffer source = "+ id (string) ... lorem (ipsum)\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.type == "string");
    REQUIRE(parameter.node.description == "lorem (ipsum)");

    REQUIRE(parameter.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].length == 30);
    REQUIRE(parameter.sourceMap.type.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.type.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.type.sourceMap[0].length == 30);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].length == 30);
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.list.empty());
}

TEST_CASE("Parameter with additional description", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id (string) ... lorem (ipsum)\n\n"\
    "  Additional description";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.type == "string");
    REQUIRE(parameter.node.description == "lorem (ipsum)\n\nAdditional description");

    REQUIRE(parameter.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].length == 31);
    REQUIRE(parameter.sourceMap.type.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.type.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.type.sourceMap[0].length == 31);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.description.sourceMap.size() == 2);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].length == 31);
    REQUIRE(parameter.sourceMap.description.sourceMap[1].location == 35);
    REQUIRE(parameter.sourceMap.description.sourceMap[1].length == 22);
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.list.empty());
}

TEST_CASE("Parameter with additional description as continuation of signature", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id (string) ... lorem (ipsum)\n"\
    "  Additional description\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.type == "string");
    REQUIRE(parameter.node.description == "lorem (ipsum)\nAdditional description\n\n");

    REQUIRE(parameter.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].location == 0);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].length == 57);
    REQUIRE(parameter.sourceMap.type.sourceMap.size() == 1);
    REQUIRE(parameter.sourceMap.type.sourceMap[0].location == 0);
    REQUIRE(parameter.sourceMap.type.sourceMap[0].length == 57);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.description.sourceMap.size() == 2);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].location == 0);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].length == 57);
    REQUIRE(parameter.sourceMap.description.sourceMap[1].location == 0);
    REQUIRE(parameter.sourceMap.description.sourceMap[1].length == 57);
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.list.empty());
}

TEST_CASE("Parameter with list in description", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id (optional, string) ... lorem (ipsum)\n"\
    "  dolor sit amet\n\n"\
    "  + Ut pulvinar\n"\
    "  + Mauris condimentum\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.type == "string");
    REQUIRE(parameter.node.description == "lorem (ipsum)\ndolor sit amet\n\n+ Ut pulvinar\n\n+ Mauris condimentum\n");;

    REQUIRE(parameter.sourceMap.name.sourceMap.size() == 2);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.name.sourceMap[0].length == 40);
    REQUIRE(parameter.sourceMap.name.sourceMap[1].location == 44);
    REQUIRE(parameter.sourceMap.name.sourceMap[1].length == 15);
    REQUIRE(parameter.sourceMap.description.sourceMap.size() == 6);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].location == 2);
    REQUIRE(parameter.sourceMap.description.sourceMap[0].length == 40);
    REQUIRE(parameter.sourceMap.description.sourceMap[1].location == 44);
    REQUIRE(parameter.sourceMap.description.sourceMap[1].length == 15);
    REQUIRE(parameter.sourceMap.description.sourceMap[2].location == 2);
    REQUIRE(parameter.sourceMap.description.sourceMap[2].length == 40);
    REQUIRE(parameter.sourceMap.description.sourceMap[3].location == 44);
    REQUIRE(parameter.sourceMap.description.sourceMap[3].length == 15);
    REQUIRE(parameter.sourceMap.description.sourceMap[4].location == 62);
    REQUIRE(parameter.sourceMap.description.sourceMap[4].length == 14);
    REQUIRE(parameter.sourceMap.description.sourceMap[5].location == 78);
    REQUIRE(parameter.sourceMap.description.sourceMap[5].length == 21);
    REQUIRE(parameter.sourceMap.values.list.empty());
}
