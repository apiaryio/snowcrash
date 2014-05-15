//
//  test-AssetParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/17/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "AssetParser.h"
#include "MarkdownParser.h"

using namespace snowcrash;

const mdp::ByteBuffer BodyAssetFixture = \
"+ Body\n"\
"\n"\
"        Lorem Ipsum\n";

const mdp::ByteBuffer SchemaAssetFixture = \
"+ Schema\n"\
"\n"\
"        Lorem Ipsum\n";

TEST_CASE("recognize explicit body signature", "[asset]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(BodyAssetFixture, markdownAST);
    
    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<Asset>::sectionType(markdownAST.children().begin()) == BodySectionType);
}

TEST_CASE("recognize schema signature", "[asset]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(SchemaAssetFixture, markdownAST);
    
    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<Asset>::sectionType(markdownAST.children().begin()) == SchemaSectionType);
}

TEST_CASE("parse body asset", "[asset]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(BodyAssetFixture, markdownAST);
    
    REQUIRE(!markdownAST.children().empty());

    Blueprint bp;
    ParserData pd(0, mdp::ByteBuffer(), bp);
    Asset asset;
    Report report;
    MarkdownNodeIterator cur = AssetParser::parse(markdownAST.children().begin(),
                                                  markdownAST.children(),
                                                  pd,
                                                  report,
                                                  asset);

    REQUIRE(std::distance(markdownAST.children().begin(), cur) == 1);
    REQUIRE(asset == "Lorem Ipsum\n");
}

TEST_CASE("parse schema asset", "[asset]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(SchemaAssetFixture, markdownAST);
    
    REQUIRE(!markdownAST.children().empty());
    
    Blueprint bp;
    ParserData pd(0, mdp::ByteBuffer(), bp);
    Asset asset;
    Report report;
    MarkdownNodeIterator cur = AssetParser::parse(markdownAST.children().begin(),
                                                  markdownAST.children(),
                                                  pd,
                                                  report,
                                                  asset);
    
    REQUIRE(std::distance(markdownAST.children().begin(), cur) == 1);
    REQUIRE(asset == "Lorem Ipsum\n");
}

//TEST_CASE("Parse body asset followed by other blocks", "[asset][foreign][dangling]")
//{
//    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
//    
//    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Hello World", 0, MakeSourceDataBlock(4, 1)));
//    
//    CHECK(markdown.size() == 7);
//    
//    Asset asset;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.size() == 1);
//    
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 7);
//    REQUIRE(asset == "Lorem Ipsum4");
//}
//
//TEST_CASE("aparser/parse-foreign", "Parse body asset with foreign block inside")
//{
//    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
//    
//    MarkdownBlock foreign(ParagraphBlockType, "Hello World", 0, MakeSourceDataBlock(4, 1));
//    MarkdownBlock::Stack::iterator pos = markdown.begin();
//    std::advance(pos, 4);
//    markdown.insert(pos, 1, foreign);
//    
//    CHECK(markdown.size() == 7);
//    
//    Asset asset;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//    CHECK(result.first.warnings.size() == 1);
//    
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 7);
//    REQUIRE(asset == "Lorem Ipsum4");
//}
//
//TEST_CASE("Parse body asset with foreign list item inside", "[asset][foreign]")
//{
//    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
//    
//    MarkdownBlock::Stack foreign;
//    foreign.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
//    foreign.push_back(MarkdownBlock(ListItemBlockEndType, "Foreign", 0, MakeSourceDataBlock(5, 1)));
//    
//    MarkdownBlock::Stack::iterator pos = markdown.begin();
//    std::advance(pos, 5);
//    markdown.insert(pos, foreign.begin(), foreign.end());
//    
//    CHECK(markdown.size() == 8);
//    
//    Asset asset;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.empty());
//    
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 5);
//    REQUIRE(asset == "Lorem Ipsum");
//}
//
//TEST_CASE("aparser/parse-multiline-signature", "Parse body asset with multiple lines in its signagure")
//{
//    // Blueprint in question:
//    //R"(
//    //+ Body
//    //  A
//    //
//    //          Lorem Ipsum
//    //)";
//    
//    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
//    
//    REQUIRE(markdown.size() == 6);
//    
//    markdown[2].content = "Body\n  A\n";
//    
//    Asset asset;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//    CHECK(result.first.warnings.size() == 1); // expected code block
//    
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
//    REQUIRE(asset == "  A\nLorem Ipsum");
//}
//
//TEST_CASE("aparser/parse-multipart", "Parse body asset composed from multiple blocks")
//{
//    // Blueprint in question:
//    //R"(
//    //+ Body
//    //  A
//    //
//    //  B
//    //
//    //          Lorem Ipsum
//    //)";
//    
//    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
//    
//    REQUIRE(markdown.size() == 6);
//    
//    markdown[2].content = "Body\n  A\n";
//    
//    MarkdownBlock foreign(ParagraphBlockType, "B", 0, MakeSourceDataBlock(4, 1));
//    MarkdownBlock::Stack::iterator pos = markdown.begin();
//    std::advance(pos, 3);
//    markdown.insert(pos, 1, foreign);
//    
//    Asset asset;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//    CHECK(result.first.warnings.size() == 2); // expected code block
//    
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 7);
//    REQUIRE(asset == "  A\n4Lorem Ipsum");
//}
//
//TEST_CASE("aparser/fix-body-not-parsed", "Issue: Object payload body isn't parsed")
//{
//    // http://github.com/apiaryio/snowcrash/issues/6
//    // NOTE: caused by extra spaces before the body
//    //
//    // Blueprint in question:
//    //R"(
//    //+    Body
//    //
//    //          Lorem Ipsum
//    //");
//    
//    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
//    markdown[2].content = "   Body";
//    
//    Asset asset;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//    CHECK(result.first.warnings.empty());
//    
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
//    REQUIRE(asset == "Lorem Ipsum");
//}
//
