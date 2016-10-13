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

TEST_CASE("Recognize parameter with just parameter name", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONParameterSectionType);
}

TEST_CASE("Recognize parameter with escaped identifier for new syntax", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ `user-name`", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONParameterSectionType);
}

TEST_CASE("Recognize parameter with parameter name and without any values or description", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id (optional, string)", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONParameterSectionType);
}

TEST_CASE("Recognize parameter with parameter type as first trait", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id (string, optional)", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONParameterSectionType);
}

TEST_CASE("Recognize parameter with new syntax example value", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id : ``1`0`` (number)", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONParameterSectionType);
}

TEST_CASE("Recognize parameter with only new syntax example value", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id: 10", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONParameterSectionType);
}

TEST_CASE("Recognize parameter with new syntax description which has old description identifier", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id (string) - This is nice and ... awesome", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONParameterSectionType);
}

TEST_CASE("Recognize parameter with new syntax description", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id (string) - This is nice and awesome", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONParameterSectionType);
}

TEST_CASE("Recognize parameter with only old syntax description", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id ... The user id", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ParameterSectionType);
}

TEST_CASE("Recognize parameter with brackets in old syntax example value", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id (optional, oData, `substringof('homer', id)`) ... test", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ParameterSectionType);
}

TEST_CASE("Recognize escaped parameter with brackets in old syntax example value", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ `id` (optional, oData, `example`)", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ParameterSectionType);
}

TEST_CASE("Recognize parameter with old syntax description after attributes", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id (optional, string) ... test", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ParameterSectionType);
}

TEST_CASE("Recognize parameter with sample value in attributes", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id (string, `10`)", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ParameterSectionType);
}

TEST_CASE("Recognize parameter with enum in attributes", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id (enum[string])", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONParameterSectionType);
}

TEST_CASE("Recognize parameter with both sample value and enum in attributes", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id (enum[string], `10`)", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONParameterSectionType);
}

TEST_CASE("Recognize parameter with old syntax default value but have enum in attributes", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse("+ id = 10 (enum[number])", markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ParameterSectionType);
}

TEST_CASE("Recognize parameter with ambiguous signature but uses MSON syntax for default value", "[parameter][336]")
{
    mdp::ByteBuffer source = \
    "+ id (optional)\n"\
    "    + Default: 1";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONParameterSectionType);
}

TEST_CASE("Recognize parameter with ambiguous signature but uses MSON syntax for sample value", "[parameter][336]")
{
    mdp::ByteBuffer source = \
    "+ id (optional)\n"\
    "    + Sample: 1";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONParameterSectionType);
}

TEST_CASE("Recognize parameter with ambiguous signature but uses MSON syntax for values", "[parameter][336]")
{
    mdp::ByteBuffer source = \
    "+ id (optional)\n"\
    "    + Members";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONParameterSectionType);
}

TEST_CASE("Recognize parameter with ambiguous signature but uses old syntax for values", "[parameter][336]")
{
    mdp::ByteBuffer source = \
    "+ id (optional)\n"\
    "    + Values";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Parameter>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ParameterSectionType);
}

TEST_CASE("Parse canonical parameter definition", "[parameter]")
{
    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(ParameterFixture, ParameterSectionType, parameter, ExportSourcemapOption);

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

    SourceMapHelper::check(parameter.sourceMap.name.sourceMap, 2, 40);
    SourceMapHelper::check(parameter.sourceMap.type.sourceMap, 2, 40);
    SourceMapHelper::check(parameter.sourceMap.defaultValue.sourceMap, 2, 40);
    SourceMapHelper::check(parameter.sourceMap.exampleValue.sourceMap, 2, 40);
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 46, 12);
    SourceMapHelper::check(parameter.sourceMap.use.sourceMap, 2, 40);
    REQUIRE(parameter.sourceMap.values.collection.size() == 3);
    SourceMapHelper::check(parameter.sourceMap.values.collection[0].sourceMap, 80, 9);
    SourceMapHelper::check(parameter.sourceMap.values.collection[1].sourceMap, 97, 9);
    SourceMapHelper::check(parameter.sourceMap.values.collection[2].sourceMap, 114, 9);
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
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == RedefinitionWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.values.size() == 1);
    REQUIRE(parameter.node.values[0] == "Hello");
    REQUIRE(parameter.node.use == UndefinedParameterUse);

    SourceMapHelper::check(parameter.sourceMap.name.sourceMap, 2, 3);
    REQUIRE(parameter.sourceMap.values.collection.size() == 1);
    SourceMapHelper::check(parameter.sourceMap.values.collection[0].sourceMap, 56, 10);
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
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
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    CHECK(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "limit");
    REQUIRE(parameter.node.description == "This is a limit" );
    REQUIRE(parameter.node.defaultValue == "20");
    REQUIRE(parameter.node.exampleValue == "42");
    REQUIRE(parameter.node.type == "number");
    REQUIRE(parameter.node.use == OptionalParameterUse);
    REQUIRE(parameter.node.values.empty());

    SourceMapHelper::check(parameter.sourceMap.name.sourceMap, 2, 58);
    SourceMapHelper::check(parameter.sourceMap.type.sourceMap, 2, 58);
    SourceMapHelper::check(parameter.sourceMap.defaultValue.sourceMap, 2, 58);
    SourceMapHelper::check(parameter.sourceMap.exampleValue.sourceMap, 2, 58);
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 2, 58);
    SourceMapHelper::check(parameter.sourceMap.use.sourceMap, 2, 58);
    REQUIRE(parameter.sourceMap.values.collection.empty());
}

TEST_CASE("Warn on error in abbreviated syntax attribute bracket", "[parameter]")
{
    mdp::ByteBuffer source = "+ limit (string1, string2, string3) ... This is a limit\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter, ExportSourcemapOption);

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

    SourceMapHelper::check(parameter.sourceMap.name.sourceMap, 2, 54);
    REQUIRE(parameter.sourceMap.type.sourceMap.empty());
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 2, 54);
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.collection.empty());
}

TEST_CASE("Warn about required vs default clash", "[parameter]")
{
    mdp::ByteBuffer source = "+ id = `42` (required)\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.use == RequiredParameterUse);
    REQUIRE(parameter.node.defaultValue == "42");

    SourceMapHelper::check(parameter.sourceMap.name.sourceMap, 2, 21);
    REQUIRE(parameter.sourceMap.type.sourceMap.empty());
    SourceMapHelper::check(parameter.sourceMap.defaultValue.sourceMap, 2, 21);
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.description.sourceMap.empty());
    SourceMapHelper::check(parameter.sourceMap.use.sourceMap, 2, 21);
    REQUIRE(parameter.sourceMap.values.collection.empty());
}

TEST_CASE("Warn about implicit required vs default clash", "[parameter_definition][source]")
{
    mdp::ByteBuffer source = "+ id = `42`\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.use == UndefinedParameterUse);
    REQUIRE(parameter.node.defaultValue == "42");

    SourceMapHelper::check(parameter.sourceMap.name.sourceMap, 2, 10);
    REQUIRE(parameter.sourceMap.type.sourceMap.empty());
    SourceMapHelper::check(parameter.sourceMap.defaultValue.sourceMap, 2, 10);
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.description.sourceMap.empty());
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.collection.empty());
}

TEST_CASE("Unrecognized 'values' keyword", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ param\n"\
    "    + Values:\n"\
    "        + `lorem`\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "param");
    REQUIRE(parameter.node.description == "+ Values:\n    + `lorem`\n");
    REQUIRE(parameter.node.use == UndefinedParameterUse);
    REQUIRE(parameter.node.values.empty());

    SourceMapHelper::check(parameter.sourceMap.name.sourceMap, 2, 6);
    REQUIRE(parameter.sourceMap.type.sourceMap.empty());
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 12, 10, 26, 14);
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.collection.empty());
}

TEST_CASE("Warn missing example item in values", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id = `Value2` (optional, string, `Value1`)\n"\
    "    + Values\n"\
    "        + `Value2`\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.exampleValue == "Value1");
    REQUIRE(parameter.node.defaultValue == "Value2");

    REQUIRE(parameter.sourceMap.values.collection.size() == 1);
    SourceMapHelper::check(parameter.sourceMap.values.collection[0].sourceMap, 66, 11);
}

TEST_CASE("Warn missing default value in values", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id = `Value1` (optional, string, `Value2`)\n"\
    "    + Values\n"\
    "        + `Value2`\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.exampleValue == "Value2");
    REQUIRE(parameter.node.defaultValue == "Value1");

    REQUIRE(parameter.sourceMap.values.collection.size() == 1);
    SourceMapHelper::check(parameter.sourceMap.values.collection[0].sourceMap, 66, 11);
}

TEST_CASE("Parse parameters with dot in its name", "[parameter]")
{
    mdp::ByteBuffer source = "+ product.id ... Hello\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "product.id");
    REQUIRE(parameter.node.description == "Hello");

    SourceMapHelper::check(parameter.sourceMap.name.sourceMap, 2, 21);
    REQUIRE(parameter.sourceMap.type.sourceMap.empty());
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 2, 21);
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.collection.empty());
}

TEST_CASE("Parentheses in parameter description", "[parameter]")
{
    mdp::ByteBuffer source = "+ id (string) ... lorem (ipsum)\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.type == "string");
    REQUIRE(parameter.node.description == "lorem (ipsum)");

    SourceMapHelper::check(parameter.sourceMap.name.sourceMap, 2, 30);
    SourceMapHelper::check(parameter.sourceMap.type.sourceMap, 2, 30);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 2, 30);
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.collection.empty());
}

TEST_CASE("Parameter with additional description", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id (string) ... lorem (ipsum)\n\n"\
    "  Additional description";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.type == "string");
    REQUIRE(parameter.node.description == "lorem (ipsum)\n\nAdditional description");

    SourceMapHelper::check(parameter.sourceMap.name.sourceMap, 2, 31);
    SourceMapHelper::check(parameter.sourceMap.type.sourceMap, 2, 31);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 2, 31, 35, 22);
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.collection.empty());
}

TEST_CASE("Parameter with additional description as continuation of signature", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id (string) ... lorem (ipsum)\n"\
    "  Additional description\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.type == "string");
    REQUIRE(parameter.node.description == "lorem (ipsum)\nAdditional description\n\n");

    SourceMapHelper::check(parameter.sourceMap.name.sourceMap, 0, 57);
    SourceMapHelper::check(parameter.sourceMap.type.sourceMap, 0, 57);
    REQUIRE(parameter.sourceMap.defaultValue.sourceMap.empty());
    REQUIRE(parameter.sourceMap.exampleValue.sourceMap.empty());
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 0, 57);
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
    REQUIRE(parameter.sourceMap.values.collection.empty());
}

TEST_CASE("Parameter with list in description", "[parameter]")
{
    mdp::ByteBuffer source = \
    "+ id (optional, string) ... lorem (ipsum)\n"\
    "  dolor sit amet\n\n"\
    "  + Ut pulvinar\n"\
    "  + Mauris condimentum\n";

    ParseResult<Parameter> parameter;
    SectionParserHelper<Parameter, ParameterParser>::parse(source, ParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.type == "string");
    REQUIRE(parameter.node.description == "lorem (ipsum)\ndolor sit amet\n\n+ Ut pulvinar\n\n+ Mauris condimentum\n");;

    SourceMapHelper::check(parameter.sourceMap.name.sourceMap, 2, 40, 44, 15);
    REQUIRE(parameter.sourceMap.description.sourceMap.size() == 4);
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 2, 40, 1);
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 44, 15, 2);
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 62, 14, 3);
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 78, 21, 4);
    REQUIRE(parameter.sourceMap.values.collection.empty());
}
