//
//  test-ParametersParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 9/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "Fixture.h"
#include "Parser.h"
#include "ParametersParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

MarkdownBlock::Stack snowcrashtest::CanonicalParametersFixture()
{
    //R"(
    //+ Parameters
    //
    //  <see CanonicalParameterDefinitionFixture()>
    //)";
    
    MarkdownBlock::Stack markdown;
    // Parameters BEGIN
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Parameters", 0, MakeSourceDataBlock(0, 1)));
    
    // Inject parameter definiton
    MarkdownBlock::Stack parameterDefinition = CanonicalParameterDefinitionFixture();
    markdown.insert(markdown.end(), parameterDefinition.begin(), parameterDefinition.end());
    
    parameterDefinition[2].content = "limit";
    markdown.insert(markdown.end(), parameterDefinition.begin(), parameterDefinition.end());
    
    // Parameters END
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(15, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0,MakeSourceDataBlock(16, 1)));
    
    return markdown;
}

TEST_CASE("Parameters block classifier", "[parameters][classifier]")
{
    MarkdownBlock::Stack markdown = CanonicalParametersFixture();
    
    REQUIRE(markdown.size() == 57);
    
    BlockIterator cur = markdown.begin();
    
    // ListBlockBeginType
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), UndefinedSection) == ParametersSection);
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), ParametersSection) == ForeignSection);
    
    ++cur; // ListItemBlockBeginType
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), UndefinedSection) == ParametersSection);
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), ParametersSection) == UndefinedSection);
    
    ++cur; // ParagraphBlockType
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), ParametersSection) == ParametersSection);
    
    ++cur; // ListBlockBeginType
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), ParametersSection) == ParameterDefinitionSection);
}

TEST_CASE("Parse canonical parameters", "[parameters]")
{
    MarkdownBlock::Stack markdown = CanonicalParametersFixture();
    ParameterCollection parameters;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = ParametersParser::Parse(markdown.begin(), markdown.end(), parser, parameters);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 57);
    
    REQUIRE(parameters.size() == 2);

    REQUIRE(parameters[0].name == "id");
    REQUIRE(parameters[0].description == "2");
    
    REQUIRE(parameters[1].name == "limit");
    REQUIRE(parameters[1].description == "2");
}

TEST_CASE("Parse description parameter only", "[parameters]")
{
    //# GET /1
    //
    //+ Parameters
    //
    //    + param1
    //
    //      A
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Parameters", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Param1", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "A", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    
    
    ParameterCollection parameters;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = ParametersParser::Parse(markdown.begin(), markdown.end(), parser, parameters);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 11);
    
    REQUIRE(parameters.size() == 1);
    
    REQUIRE(parameters[0].name == "Param1");
    REQUIRE(parameters[0].description == "2");
    REQUIRE(parameters[0].use == UndefinedParameterUse);
    REQUIRE(parameters[0].type.empty());
    REQUIRE(parameters[0].defaultValue.empty());
    REQUIRE(parameters[0].exampleValue.empty());
    REQUIRE(parameters[0].values.empty());
}

TEST_CASE("Parse multiple parameters", "[parameters]")
{
    //+ Parameters
    //
    //    + param1
    //
    //        A
    //
    //        + Optional
    //
    //
    //    + param2
    //
    //        B
    //
    //        + Example: `B-2`
    //
    //    + param3
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Parameters", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    // Param 1
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Param1", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "A", 0, MakeSourceDataBlock(2, 1)));

    // Optional
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Optional\n", 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    
    // Param 2
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Param2", 0, MakeSourceDataBlock(6, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "B", 0, MakeSourceDataBlock(7, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Example: `B-2`\n", 0, MakeSourceDataBlock(8, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(9, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(10, 1)));
    
    // Param 2
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Param3", 0, MakeSourceDataBlock(11, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(12, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(13, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(14, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(15, 1)));
    
    ParameterCollection parameters;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = ParametersParser::Parse(markdown.begin(), markdown.end(), parser, parameters);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 26);
    
    REQUIRE(parameters.size() == 3);
    
    REQUIRE(parameters[0].name == "Param1");
    REQUIRE(parameters[0].description == "2");
    REQUIRE(parameters[0].use == OptionalParameterUse);
    REQUIRE(parameters[0].type.empty());
    REQUIRE(parameters[0].defaultValue.empty());
    REQUIRE(parameters[0].exampleValue.empty());
    REQUIRE(parameters[0].values.empty());
    
    REQUIRE(parameters[1].name == "Param2");
    REQUIRE(parameters[1].description == "7");
    REQUIRE(parameters[1].use == UndefinedParameterUse);
    REQUIRE(parameters[1].type.empty());
    REQUIRE(parameters[1].defaultValue.empty());
    REQUIRE(parameters[1].exampleValue == "B-2");
    REQUIRE(parameters[1].values.empty());
    
    REQUIRE(parameters[2].name == "Param3");
    REQUIRE(parameters[2].description.empty());
    REQUIRE(parameters[2].use == UndefinedParameterUse);
    REQUIRE(parameters[2].type.empty());
    REQUIRE(parameters[2].defaultValue.empty());
    REQUIRE(parameters[2].exampleValue.empty());
    REQUIRE(parameters[2].values.empty());
}

TEST_CASE("Parse ilegal parameter", "[parameters]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //
    //+ Parameters
    //    + i:legal
    //
    //+ Response 200
    //
    //        Ok.
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Parameters\n"\
    "    + i:legal\n\n"\
    "+ Response 200\n"\
    "\n"\
    "        Ok.\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 2);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    REQUIRE(result.warnings[1].code == FormattingWarning); // no parameters specified
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.empty());
}

TEST_CASE("Parse ilegal parameter among legal ones", "[parameters]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Parameters
    //    + OK-1
    //    + i:legal
    //    + OK-2
    //
    //+ Response 200
    //
    //        Ok.
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Parameters\n"\
    "    + OK-1\n"\
    "    + i:legal\n"\
    "    + OK-2\n"\
    "\n"\
    "+ Response 200\n"\
    "\n"\
    "        Ok.\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 2);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "OK-1");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[1].name == "OK-2");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[1].description.empty());
}

TEST_CASE("Warn about additional content in parameters section", "[parameters]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Parameters
    //  extra-1
    //
    //    + id
    //
    //+ Response 204
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Parameters\n"\
    "  extra-1\n"\
    "\n"\
    "    + id\n"\
    "\n"\
    "+ Response 204\n\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].description.empty());
}


TEST_CASE("Warn about additional content block in parameters section", "[parameters]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Parameters
    //
    //  extra-1
    //
    //    + id
    //
    //+ Response 204
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Parameters\n"\
    "  \n"\
    "   extra-1\n"\
    "\n"\
    "    + id\n"\
    "\n"\
    "+ Response 204\n\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].description.empty());
}

TEST_CASE("Warn about required vs default clash", "[parameters]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Parameters
    //    + id
    //        + Required
    //        + Default: `42`
    //
    //+ Response 204
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Parameters\n"\
    "    + id\n"\
    "        + Required\n"\
    "        + Default: `42`\n"\
    "\n"\
    "+ Response 204\n\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == LogicalErrorWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].use == RequiredParameterUse);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].defaultValue == "42");
}

