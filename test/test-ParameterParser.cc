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
"        + `beef`\n"\
"";

TEST_CASE("Recognize parameter definition signature", "[parameter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(ParameterFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<Parameter>::sectionType(markdownAST.children().begin()) == ParameterSectionType);
}

//TEST_CASE("Parse canonical parameter definition", "[parameter][block]")
//{
//    Parameter parameter;
//    Report report;
//    SectionParserHelper<Parameter, ParameterParser>::parse(ParameterFixture, ParameterDefinitionSectionType, report, parameter);
//
//    REQUIRE(report.error.code == Error::OK);
//    CHECK(report.warnings.empty());
//
//    REQUIRE(parameter.name == "id");
//    REQUIRE(parameter.description == "2");
//    REQUIRE(parameter.type == "number");
//    REQUIRE(parameter.use == OptionalParameterUse);
//    REQUIRE(parameter.defaultValue == "1234");
//    REQUIRE(parameter.exampleValue == "0000");
//    REQUIRE(parameter.values.size() == 3);
//    REQUIRE(parameter.values[0] == "1234");
//    REQUIRE(parameter.values[1] == "0000");
//    REQUIRE(parameter.values[2] == "beef");
//}

//TEST_CASE("Parse canonical definition followed by another definition", "[parameter_definition][block]")
//{
//    MarkdownBlock::Stack markdown = CanonicalParameterDefinitionFixture();
//
//    MarkdownBlock::Stack parameterBlocks;
//    parameterBlocks.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
//    parameterBlocks.push_back(MarkdownBlock(ParagraphBlockType, "additional_parameter", 0, MakeSourceDataBlock(1, 1)));
//    parameterBlocks.push_back(MarkdownBlock(ParagraphBlockType, "Hello World", 0, MakeSourceDataBlock(2, 1)));
//    parameterBlocks.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
//
//    MarkdownBlock::Stack::iterator it = markdown.end();
//    --it;
//    markdown.insert(it, parameterBlocks.begin(), parameterBlocks.end());
//
//
//    Parameter parameter;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = ParameterDefinitionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, parameter);
//
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.empty());
//
//    const MarkdownBlock::Stack &blocks = markdown;
//    CHECK(std::distance(blocks.begin(), result.second) == 17);
//
//    REQUIRE(parameter.name == "id");
//    REQUIRE(parameter.description == "2");
//}
//
//TEST_CASE("Parse canonical definition followed by ilegal one", "[parameter_definition][block]")
//{
//    MarkdownBlock::Stack markdown = CanonicalParameterDefinitionFixture();
//
//    MarkdownBlock::Stack parameterBlocks;
//    parameterBlocks.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
//    parameterBlocks.push_back(MarkdownBlock(ListItemBlockBeginType, "i:legal", 0, MakeSourceDataBlock(1, 1)));
//
//    MarkdownBlock::Stack::iterator it = markdown.end();
//    --it;
//    markdown.insert(it, parameterBlocks.begin(), parameterBlocks.end());
//
//    Parameter parameter;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = ParameterDefinitionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, parameter);
//
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.empty());
//
//    const MarkdownBlock::Stack &blocks = markdown;
//    CHECK(std::distance(blocks.begin(), result.second) == 17);
//
//    REQUIRE(parameter.name == "id");
//    REQUIRE(parameter.description == "2");
//}
//
//TEST_CASE("Parse ilegal parameter trait at the begining", "[parameter_definition][source]")
//{
//    // Blueprint in question:
//    //R"(
//    //# /1/{4}
//    //+ Parameters
//    //    + 4
//    //       + ilegal
//    //
//    //");
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
//
//TEST_CASE("Warn when re-setting the values attribute", "[parameter_definition][source]")
//{
//    // Blueprint in question:
//    //R"(
//    //# /1/{id}
//    //+ Parameters
//    //    + id
//    //        + Values
//    //            + `Ahoy`
//    //        + Values
//    //            + `Hello`
//    //");
//    const std::string blueprintSource = \
//    "# /1/{id}\n"\
//    "+ Parameters\n"\
//    "    + id\n"\
//    "        + Values\n"\
//    "            + `Ahoy`\n"\
//    "        + Values\n"\
//    "            + `Hello`\n"\
//    "\n";
//
//    Parser parser;
//    Result result;
//    Blueprint blueprint;
//    parser.parse(blueprintSource, 0, result, blueprint);
//    REQUIRE(result.error.code == Error::OK);
//    REQUIRE(result.warnings.size() == 1);
//    REQUIRE(result.warnings[0].code == RedefinitionWarning);
//
//    REQUIRE(blueprint.resourceGroups.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].description.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].name == "id");
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].values.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].values[0] == "Hello");
//}
//
//TEST_CASE("Warn when there are no values in the values attribute", "[parameter_definition][source]")
//{
//    // Blueprint in question:
//    //R"(
//    //# /1/{id}
//    //+ Parameters
//    //    + id
//    //        + Values
//    //
//    //");
//    const std::string blueprintSource = \
//    "# /1/{id}\n"\
//    "+ Parameters\n"\
//    "    + id\n"\
//    "        + Values\n"\
//    "\n";
//
//    Parser parser;
//    Result result;
//    Blueprint blueprint;
//    parser.parse(blueprintSource, 0, result, blueprint);
//    REQUIRE(result.error.code == Error::OK);
//    REQUIRE(result.warnings.size() == 1);
//    REQUIRE(result.warnings[0].code == EmptyDefinitionWarning);
//
//    REQUIRE(blueprint.resourceGroups.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].description.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].name == "id");
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].values.empty());
//}
//
//TEST_CASE("Parse full abbreviated syntax", "[parameter_definition][source]")
//{
//    // Blueprint in question:
//    //R"(
//    //# /machine{?limit}
//    //+ Parameters
//    //    + limit = `20` (optional, number, `42`) ... This is a limit
//    //");
//    const std::string blueprintSource = \
//    "# /machine{?limit}\n"\
//    "+ Parameters\n"\
//    "    + limit = `20` (optional, number, `42`) ... This is a limit\n"\
//    "\n";
//
//    Parser parser;
//    Result result;
//    Blueprint blueprint;
//    parser.parse(blueprintSource, 0, result, blueprint);
//    REQUIRE(result.error.code == Error::OK);
//    REQUIRE(result.warnings.empty());
//
//    REQUIRE(blueprint.resourceGroups.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].description.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].name == "limit");
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].description == "This is a limit" );
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].defaultValue == "20");
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].exampleValue == "42");
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].type == "number");
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].use == OptionalParameterUse);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].values.empty());
//}
//
//TEST_CASE("Warn on error in  abbreviated syntax attribute bracket", "[parameter_definition][source]")
//{
//    // Blueprint in question:
//    //R"(
//    //# /machine{?limit}
//    //+ Parameters
//    //    + limit (string1, string2, string3) ... This is a limit
//    //");
//    const std::string blueprintSource = \
//    "# /machine{?limit}\n"\
//    "+ Parameters\n"\
//    "    + limit (string1, string2, string3) ... This is a limit\n"\
//    "\n";
//
//    Parser parser;
//    Result result;
//    Blueprint blueprint;
//    parser.parse(blueprintSource, 0, result, blueprint);
//    REQUIRE(result.error.code == Error::OK);
//    REQUIRE(result.warnings.size() == 1);
//    REQUIRE(result.warnings[0].code == FormattingWarning);
//
//    REQUIRE(blueprint.resourceGroups.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].description.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].name == "limit");
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].description == "This is a limit" );
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].defaultValue.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].exampleValue.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].type.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].use == UndefinedParameterUse);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].values.empty());
//}
//
//TEST_CASE("Warn about required vs default clash", "[parameter_definition][source]")
//{
//    // Blueprint in question:
//    //R"(
//    //# /1/{id}
//    //+ Parameters
//    //   + id = `42` (required)
//    //");
//    const std::string blueprintSource = \
//    "# /1/{id}\n"\
//    "+ Parameters\n"\
//    "    + id = `42` (required)\n"\
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
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].use == RequiredParameterUse);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].defaultValue == "42");
//}
//
//TEST_CASE("Warn about implicit required vs default clash", "[parameter_definition][source]")
//{
//    // Blueprint in question:
//    //R"(
//    //# /1/{id}
//    //+ Parameters
//    //   + id = `42`
//    //");
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
//
//
//TEST_CASE("Unrecognized 'values' keyword", "[parameter_definition][issue][#44][source]")
//{
//    // Blueprint in question:
//    //R"(
//    //FORMAT: X-1A
//    //HOST: xxxxxxxxxxxxxxxxxxxxxxx
//    //
//    //# A very long API name
//    //
//    //# Resource [/1/{param}]
//    //
//    //## GET
//    //
//    //+ Parameters
//    //    + param
//    //        + Values:
//    //            + `lorem`
//    //
//    //+ Response 204
//    //");
//    const std::string blueprintSource = \
//    "FORMAT: X-1A\n"\
//    "HOST: xxxxxxxxxxxxxxxxxxxxxxx\n"\
//    "\n"\
//    "# A very long API name\n"\
//    "\n"\
//    "# Resource [/1/{param}]\n"\
//    "\n"\
//    "## GET\n"\
//    "\n"\
//    "+ Parameters\n"\
//    "    + param\n"\
//    "        + Values:\n"\
//    "            + `lorem`\n"\
//    "\n"\
//    "+ Response 204\n";
//
//    Parser parser;
//    Result result;
//    Blueprint blueprint;
//    parser.parse(blueprintSource, 0, result, blueprint);
//    REQUIRE(result.error.code == Error::OK);
//    REQUIRE(result.warnings.size() == 1);
//    REQUIRE(result.warnings[0].code == IgnoringWarning);
//
//    REQUIRE(result.warnings[0].location.size() == 2);
//    REQUIRE(result.warnings[0].location[0].location == 134);
//    REQUIRE(result.warnings[0].location[0].length == 10);
//    REQUIRE(result.warnings[0].location[1].location == 152);
//    REQUIRE(result.warnings[0].location[1].length == 14);
//
//    REQUIRE(blueprint.resourceGroups.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].description.empty());
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "param");
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].use == UndefinedParameterUse);
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].values.empty());
//}
//
//TEST_CASE("warn missing example item in values", "[parameters][issue][#67]")
//{
//    // Blueprint in question:
//    //R"(
//    //# /1/{id}
//    //+ Parameters
//    //    + id = `Value2` (optional, string, `Value1`);
//    //        + Values
//    //            + `Value2`
//    //");
//    const std::string blueprintSource = \
//    "# /1/{id}\n"\
//    "+ Parameters\n"\
//    "    + id = `Value2` (optional, string, `Value1`)\n"\
//    "        + Values\n"\
//    "            + `Value2`\n"\
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
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].exampleValue == "Value1");
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].defaultValue == "Value2");
//}
//
//TEST_CASE("warn missing default value in values", "[parameters][issue][#67]")
//{
//    // Blueprint in question:
//    //R"(
//    //# /1/{id}
//    //+ Parameters
//    //    + id = `Value1` (optional, string, `Value2`);
//    //        + Values
//    //            + `Value2`
//    //");
//    const std::string blueprintSource = \
//    "# /1/{id}\n"\
//    "+ Parameters\n"\
//    "    + id = `Value1` (optional, string, `Value2`)\n"\
//    "        + Values\n"\
//    "            + `Value2`\n"\
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
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].exampleValue == "Value2");
//    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].defaultValue == "Value1");
//}
