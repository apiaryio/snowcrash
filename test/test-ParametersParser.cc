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
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), ParametersSection) == UndefinedSection);
    
    ++cur; // ListItemBlockBeginType
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), UndefinedSection) == ParametersSection);
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), ParametersSection) == UndefinedSection);
    
    ++cur; // ParagraphBlockType
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), ParametersSection) == UndefinedSection);
    
    ++cur; // ListBlockBeginType
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<ParameterCollection>(cur, markdown.end(), ParametersSection) == ParameterDefinitionSection);
}

TEST_CASE("Parse canonical parameters", "[parameters][now]")
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

