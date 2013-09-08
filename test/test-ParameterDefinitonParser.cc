//
//  test-ParameterDefinitonParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 9/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "Fixture.h"
#include "ParametersParser.h"
#include "Parser.h"

using namespace snowcrash;
using namespace snowcrashtest;

MarkdownBlock::Stack snowcrashtest::CanonicalParameterDefinitionFixture()
{
    //R"(
    //+ id
    //
    //    Lorem ipsum.
    //
    //    + Type: number
    //    + Optional
    //    + Default: `1234`
    //    + Example: `0000`
    //    + Values:
    //        + `1234`
    //        + `0000`
    //        + `beef`
    //)";
    
    MarkdownBlock::Stack markdown;
    
    // id BEGIN
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "id", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Lorem ipsum.", 0, MakeSourceDataBlock(2, 1)));
    
    // traits BEGIN
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));

    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Type: number\n", 0, MakeSourceDataBlock(3, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Optional\n", 0, MakeSourceDataBlock(4, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Default: `1234`\n", 0, MakeSourceDataBlock(5, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Example: `0000`\n", 0, MakeSourceDataBlock(6, 1)));
    
    // Values BEGIN
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "`1234`\n", 0, MakeSourceDataBlock(7, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "`0000`\n", 0, MakeSourceDataBlock(8, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "`beef`\n", 0, MakeSourceDataBlock(9, 1)));
    
    // Values END
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0,MakeSourceDataBlock(10, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Values:\n", 0, MakeSourceDataBlock(11, 1)));
    
    // traits END
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0,MakeSourceDataBlock(12, 1)));
    
    // id END
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(13, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0,MakeSourceDataBlock(14, 1)));
    
    return markdown;
}

TEST_CASE("Parameter definition block classifier", "[parameter_definition][classifier]")
{
    MarkdownBlock::Stack markdown = CanonicalParameterDefinitionFixture();
    
    REQUIRE(markdown.size() == 26);
    
    BlockIterator cur = markdown.begin();
    
    // ListBlockBeginType
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), UndefinedSection) == ParameterDefinitionSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefinitionSection) == ForeignSection);
    
    ++cur; // ListItemBlockBeginType
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), UndefinedSection) == ParameterDefinitionSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefinitionSection) == UndefinedSection);
    
    ++cur; // ParagraphBlockType
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefinitionSection) == ParameterDefinitionSection);
    
    ++cur; // ParagraphBlockType
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefinitionSection) == ParameterDefinitionSection);
    
    ++cur; // type trait BEGIN
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefinitionSection) == ParameterTypeSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterTypeSection) == ParameterTypeSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterRequiredSection) == ParameterTypeSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterOptionalSection) == ParameterTypeSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefaultSection) == ParameterTypeSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterExampleSection) == ParameterTypeSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterValuesSection) == ParameterTypeSection);
    
    std::advance(cur, 3); // optional trait BEGIN
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefinitionSection) == ParameterOptionalSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterOptionalSection) == ParameterOptionalSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterRequiredSection) == ParameterOptionalSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterOptionalSection) == ParameterOptionalSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefaultSection) == ParameterOptionalSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterExampleSection) == ParameterOptionalSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterValuesSection) == ParameterOptionalSection);
    
    std::advance(cur, 2); // default trait BEGIN
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefinitionSection) == ParameterDefaultSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterOptionalSection) == ParameterDefaultSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterRequiredSection) == ParameterDefaultSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterOptionalSection) == ParameterDefaultSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefaultSection) == ParameterDefaultSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterExampleSection) == ParameterDefaultSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterValuesSection) == ParameterDefaultSection);
    
    std::advance(cur, 2); // example trait BEGIN
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefinitionSection) == ParameterExampleSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterOptionalSection) == ParameterExampleSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterRequiredSection) == ParameterExampleSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterOptionalSection) == ParameterExampleSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefaultSection) == ParameterExampleSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterExampleSection) == ParameterExampleSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterValuesSection) == ParameterExampleSection);
    
    std::advance(cur, 2); // values trait BEGIN
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefinitionSection) == ParameterValuesSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterOptionalSection) == ParameterValuesSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterRequiredSection) == ParameterValuesSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterOptionalSection) == ParameterValuesSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterDefaultSection) == ParameterValuesSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterExampleSection) == ParameterValuesSection);
    REQUIRE(ClassifyBlock<Parameter>(cur, markdown.end(), ParameterValuesSection) == ParameterValuesSection);
}

TEST_CASE("Parse canonical parameter definition", "[parameter_definition]")
{
    MarkdownBlock::Stack markdown = CanonicalParameterDefinitionFixture();
    Parameter parameter;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = ParameterDefinitionParser::Parse(markdown.begin(), markdown.end(), parser, parameter);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    CHECK(std::distance(blocks.begin(), result.second) == 26);
        
    REQUIRE(parameter.name == "id");
    REQUIRE(parameter.description == "2");
    REQUIRE(parameter.type == "number");
    REQUIRE(parameter.use == OptionalParameterUse);
    REQUIRE(parameter.defaultValue == "1234");
    REQUIRE(parameter.exampleValue == "0000");
    REQUIRE(parameter.values.size() == 3);
    REQUIRE(parameter.values[0] == "1234");
    REQUIRE(parameter.values[1] == "0000");
    REQUIRE(parameter.values[2] == "beef");
}

TEST_CASE("Parse canonical definition followed by another definition", "[parameter_definition]")
{
    MarkdownBlock::Stack markdown = CanonicalParameterDefinitionFixture();
    
    MarkdownBlock::Stack parameterBlocks;
    parameterBlocks.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    parameterBlocks.push_back(MarkdownBlock(ParagraphBlockType, "additional_parameter", 0, MakeSourceDataBlock(1, 1)));
    parameterBlocks.push_back(MarkdownBlock(ParagraphBlockType, "Hello World", 0, MakeSourceDataBlock(2, 1)));
    parameterBlocks.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, MakeSourceDataBlock(3, 1)));

    MarkdownBlock::Stack::iterator it = markdown.end();
    --it;
    markdown.insert(it, parameterBlocks.begin(), parameterBlocks.end());

    
    Parameter parameter;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = ParameterDefinitionParser::Parse(markdown.begin(), markdown.end(), parser, parameter);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    CHECK(std::distance(blocks.begin(), result.second) == 25);
    
    REQUIRE(parameter.name == "id");
    REQUIRE(parameter.description == "2");
}

TEST_CASE("Parse canonical definition followed by ilegal one", "[parameter_definition]")
{
    MarkdownBlock::Stack markdown = CanonicalParameterDefinitionFixture();
    
    MarkdownBlock::Stack parameterBlocks;
    parameterBlocks.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    parameterBlocks.push_back(MarkdownBlock(ListItemBlockBeginType, "i:legal", 0, MakeSourceDataBlock(1, 1)));
    
    MarkdownBlock::Stack::iterator it = markdown.end();
    --it;
    markdown.insert(it, parameterBlocks.begin(), parameterBlocks.end());
    
    Parameter parameter;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = ParameterDefinitionParser::Parse(markdown.begin(), markdown.end(), parser, parameter);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    CHECK(std::distance(blocks.begin(), result.second) == 25);
    
    REQUIRE(parameter.name == "id");
    REQUIRE(parameter.description == "2");
}

TEST_CASE("Parse ilegal parameter trait at the begining", "[parameter_definition]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Parameters
    //    + 4
    //        + ilegal
    //
    //+ Response 200
    //
    //        Ok.
    //");
    const std::string bluerpintSource = \
    "# GET /1\n"\
    "+ Parameters\n"\
    "    + 4\n"\
    "        + ilegal\n"\
    "\n"\
    "+ Response 200\n"\
    "\n"\
    "        Ok.\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(bluerpintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].transactions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].transactions[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].transactions[0].responses[0].name == "200");
}

TEST_CASE("Warn when re-setting the use attribute", "[parameter_definition]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Parameters
    //    + id
    //        + optional
    //        + required
    //
    //+ Response 204
    //");
    const std::string bluerpintSource = \
    "# GET /1\n"\
    "+ Parameters\n"\
    "    + id\n"\
    "        + optional\n"\
    "        + required\n"\
    "\n"\
    "+ Response 204\n\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(bluerpintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == RedefinitionWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].use == RequiredParameterUse);
}

TEST_CASE("Warn about superfluous content in the use attribute", "[parameter_definition]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Parameters
    //    + id
    //        + optional
    //          extra-1
    //
    //+ Response 204
    //");
    const std::string bluerpintSource = \
    "# GET /1\n"\
    "+ Parameters\n"\
    "    + id\n"\
    "        + optional\n"\
    "          extra-1\n"\
    "\n"\
    "+ Response 204\n\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(bluerpintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].use == OptionalParameterUse);
}

TEST_CASE("Warn about superfluous blocks in the use attribute", "[parameter_definition]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Parameters
    //    + id
    //        + optional
    //        
    //          extra-1
    //
    //+ Response 204
    //");
    const std::string bluerpintSource = \
    "# GET /1\n"\
    "+ Parameters\n"\
    "    + id\n"\
    "        + optional\n"\
    "        \n"\
    "          extra-1\n"\
    "\n"\
    "+ Response 204\n\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(bluerpintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].use == OptionalParameterUse);
}

TEST_CASE("Warn when re-setting a key-value attribute", "[parameter_definition]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Parameters
    //    + id
    //        + Example: `42`
    //        + Example: `43`
    //
    //+ Response 204
    //");
    const std::string bluerpintSource = \
    "# GET /1\n"\
    "+ Parameters\n"\
    "    + id\n"\
    "        + Example: `42`\n"\
    "        + Example: `43`\n"\
    "\n"\
    "+ Response 204\n\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(bluerpintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == RedefinitionWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].exampleValue == "43");
}

TEST_CASE("Warn superfluous content in a key-value attribute", "[parameter_definition]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Parameters
    //    + id
    //        + Example: `42`
    //          extra-1
    //
    //+ Response 204
    //");
    const std::string bluerpintSource = \
    "# GET /1\n"\
    "+ Parameters\n"\
    "    + id\n"\
    "        + Example: `42`\n"\
    "          extra-1\n"\
    "\n"\
    "+ Response 204\n\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(bluerpintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].exampleValue == "42");
}

TEST_CASE("Warn about superfluous blocks in a key-value attribute", "[parameter_definition]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Parameters
    //    + id
    //        + Example: `42`
    //
    //          extra-1
    //
    //+ Response 204
    //");
    const std::string bluerpintSource = \
    "# GET /1\n"\
    "+ Parameters\n"\
    "    + id\n"\
    "        + Example: `42`\n"\
    "        \n"\
    "          extra-1\n"\
    "\n"\
    "+ Response 204\n\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(bluerpintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].exampleValue == "42");
}
