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
    
    REQUIRE(markdown.size() == 41);
    
    BlockIterator cur = markdown.begin();
    
    // ListBlockBeginType
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), UndefinedSectionType) == ParametersSectionType);
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), ParametersSectionType) == ForeignSectionType);
    
    ++cur; // ListItemBlockBeginType
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), UndefinedSectionType) == ParametersSectionType);
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), ParametersSectionType) == UndefinedSectionType);
    
    ++cur; // ParagraphBlockType
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), ParametersSectionType) == ParametersSectionType);
    
    ++cur; // ListBlockBeginType
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), ParametersSectionType) == ParameterDefinitionSectionType);
}

TEST_CASE("Parse canonical parameters", "[parameters]")
{
    MarkdownBlock::Stack markdown = CanonicalParametersFixture();
    ParameterCollection parameters;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ParametersParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, parameters);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 41);
    
    REQUIRE(parameters.size() == 2);

    REQUIRE(parameters[0].name == "id");
    REQUIRE(parameters[0].description == "2");
    
    REQUIRE(parameters[1].name == "limit");
    REQUIRE(parameters[1].description == "2");
}

TEST_CASE("Parse description parameter only", "[parameters]")
{
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
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ParametersParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, parameters);
    
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
    //    + param1 (Optional)
    //
    //        A
    //
    //
    //    + param2 (`B-2`)
    //
    //        B
    //
    //    + param3
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Parameters", 0, MakeSourceDataBlock(0, 1)));

    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    // Param 1
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Param1 (optional)", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "A", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    
    // Param 2
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Param2 (`B-2`)", 0, MakeSourceDataBlock(6, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "B", 0, MakeSourceDataBlock(7, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(10, 1)));
    
    // Param 3
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Param3", 0, MakeSourceDataBlock(11, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(12, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(13, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(14, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(15, 1)));

    ParameterCollection parameters;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ParametersParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, parameters);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 18);
    
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
    //# /1
    //+ Parameters
    //    + i:legal
    //");
    const std::string blueprintSource = \
    "# /1\n"\
    "+ Parameters\n"\
    "    + i:legal\n\n";
    
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
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.empty());
}

TEST_CASE("Parse ilegal parameter among legal ones", "[parameters]")
{
    // Blueprint in question:
    //R"(
    //# /1/{OK_1}/{OK_2}
    //+ Parameters
    //    + OK_1
    //    + i:legal
    //    + OK_2
    //");
    const std::string blueprintSource = \
    "# /1/{OK_1}/{OK_2}\n"\
    "+ Parameters\n"\
    "    + OK_1\n"\
    "    + i:legal\n"\
    "    + OK_2\n"\
    "\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.size() == 2);
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].name == "OK_1");
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[1].name == "OK_2");
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[1].description.empty());
}

TEST_CASE("Warn about additional content in parameters section", "[parameters]")
{
    // Blueprint in question:
    //R"(
    //# /1/{id}
    //+ Parameters
    //  extra-1
    //
    //    + id
    //
    //+ Response 204
    //");
    const std::string blueprintSource = \
    "# /1/{id}\n"\
    "+ Parameters\n"\
    "  extra-1\n"\
    "\n"\
    "    + id\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].description.empty());
}


TEST_CASE("Warn about additional content block in parameters section", "[parameters]")
{
    // Blueprint in question:
    //R"(
    //# /1/{id}
    //+ Parameters
    //
    //  extra-1
    //
    //    + id
    //
    //");
    const std::string blueprintSource = \
    "# /1/{id}\n"\
    "+ Parameters\n"\
    "\n"\
    "  extra-1\n"\
    "\n"\
    "    + id\n"\
    "\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].description.empty());
}

TEST_CASE("Warn about multiple parameters with the same name", "[parameters]")
{
    // Blueprint in question:
    //R"(
    //# /1/{id}
    //+ Parameters
    //    + id (`42`)
    //    + id (`43`)
    //");
    const std::string blueprintSource = \
    "# /1/{id}\n"\
    "+ Parameters\n"\
    "    + id (`42`)\n"\
    "    + id (`43`)\n"\
    "\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == RedefinitionWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters[0].exampleValue == "43");
}

TEST_CASE("Parse parameters with dot in its name", "[parameters][issue][#47][source]")
{
    // Blueprint in question:
    //R"(
    //# GET /contracts?product.id=4
    //
    //+ Parameters
    //    + product.id ... Hello
    //
    //+ Response 204
    //");
    const std::string blueprintSource = \
    "# GET /contracts?product.id=4\n"\
    "\n"\
    "+ Parameters\n"\
    "    + product.id ... Hello\n"\
    "\n"\
    "+ Response 204\n\n";
    
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
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "product.id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].description == "Hello");
}

TEST_CASE("Parentheses in parameter description ", "[parameters][issue][#49][source]")
{
    // Blueprint in question:
    //R"(
    //# GET /{id}
    //+ Parameters
    //  + id (string) ... lorem (ipsum)
    //
    //+ response 204
    //");
    const std::string blueprintSource = \
    "# GET /{id}\n"\
    "+ Parameters\n"\
    "  + id (string) ... lorem (ipsum)\n"\
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
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].type == "string");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].description == "lorem (ipsum)");
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