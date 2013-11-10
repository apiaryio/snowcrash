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
#include "PayloadParser.h"
#include "Fixture.h"

using namespace snowcrash;
using namespace snowcrashtest;

MarkdownBlock::Stack snowcrashtest::CanonicalBodyAssetFixture()
{
    // Blueprint in question:
    //R"(
    //+ Body
    //
    //          Lorem Ipsum
    //)";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Lorem Ipsum", 0, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    return markdown;
}

MarkdownBlock::Stack snowcrashtest::CanonicalSchemaAssetFixture()
{
    // Blueprint in question:
    //R"(
    //+ Sechema
    //
    //          Lorem Ipsum
    //)";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Schema", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Dolor Sit Amet", 0, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    return markdown;
}

TEST_CASE("aparser/signature-inline", "Verify asset signature, inline")
{
    SourceData source = "01";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Body", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(1, 1)));
    
    REQUIRE(GetAssetSignature(markdown.begin(), markdown.end()) == BodyAssetSignature);
    REQUIRE(GetAssetSignature(++BlockIterator(markdown.begin()), markdown.end()) == BodyAssetSignature);
}

TEST_CASE("aparser/signature", "Verify asset signature")
{
    
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    REQUIRE(GetAssetSignature(markdown.begin(), markdown.end()) == BodyAssetSignature);
    REQUIRE(GetAssetSignature(++BlockIterator(markdown.begin()), markdown.end()) == BodyAssetSignature);
    REQUIRE(HasAssetSignature(markdown.begin(), markdown.end()) == true);
    
    markdown = CanonicalSchemaAssetFixture();
    
    REQUIRE(GetAssetSignature(markdown.begin(), markdown.end()) == SchemaAssetSignature);
    REQUIRE(GetAssetSignature(++BlockIterator(markdown.begin()), markdown.end()) == SchemaAssetSignature);
    REQUIRE(HasAssetSignature(markdown.begin(), markdown.end()) == true);
}

TEST_CASE("aparser/no-signature-inline", "List without asset signature, inline")
{
    SourceData source = "01";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "buddy", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(1, 1)));
    
    REQUIRE(GetAssetSignature(markdown.begin(), markdown.end()) == NoAssetSignature);
    REQUIRE(GetAssetSignature(++BlockIterator(markdown.begin()), markdown.end()) == NoAssetSignature);
}

TEST_CASE("aparser/no-signature", "List without asset signature")
{
    SourceData source = "0123";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));

    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Lorem Ipsum", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    REQUIRE(GetAssetSignature(markdown.begin(), markdown.end()) == NoAssetSignature);
    REQUIRE(GetAssetSignature(++BlockIterator(markdown.begin()), markdown.end()) == NoAssetSignature);
}

TEST_CASE("aparser/classifier-body", "Body asset block classifier")
{
    
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    CHECK(markdown.size() == 6);
    
    BlockIterator cur = markdown.begin();
    // ListBlockBeginType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSectionType) == BodySectionType);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), BodySectionType) == BodySectionType);
    
    ++cur; // ListItemBlockBeginType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSectionType) == BodySectionType);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), BodySectionType) == UndefinedSectionType); // treat new list item as foreign
    
    ++cur; // ParagraphBlockType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), BodySectionType) == BodySectionType);

    ++cur; // CodeBlockType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), BodySectionType) == BodySectionType);
    
    ++cur; // ListItemBlockEndType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), BodySectionType) == UndefinedSectionType);
    
    ++cur; // ListBlockEndType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), BodySectionType) == UndefinedSectionType);
}

TEST_CASE("aparser/classifier-schema", "Schema asset block classifier")
{
    
    MarkdownBlock::Stack markdown = CanonicalSchemaAssetFixture();
    
    CHECK(markdown.size() == 6);
    
    BlockIterator cur = markdown.begin();
    // ListBlockBeginType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSectionType) == SchemaSectionType);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), SchemaSectionType) == SchemaSectionType);
    
    ++cur; // ListItemBlockBeginType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSectionType) == SchemaSectionType);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), SchemaSectionType) == UndefinedSectionType);
    
    ++cur; // ParagraphBlockType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), SchemaSectionType) == SchemaSectionType);
    
    ++cur; // CodeBlockType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), SchemaSectionType) == SchemaSectionType);
    
    ++cur; // ListItemBlockEndType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), SchemaSectionType) == UndefinedSectionType);
    
    ++cur; // ListBlockEndType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), SchemaSectionType) == UndefinedSectionType);
}

TEST_CASE("aparser/parse-body", "Parse body asset")
{
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    CHECK(markdown.size() == 6);
    
    Asset asset;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
    REQUIRE(asset == "Lorem Ipsum");
}

TEST_CASE("aparser/parse-schema", "Parse schema asset")
{
    MarkdownBlock::Stack markdown = CanonicalSchemaAssetFixture();
    
    CHECK(markdown.size() == 6);
    
    Asset asset;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
    REQUIRE(asset == "Dolor Sit Amet");
}

TEST_CASE("Parse body asset followed by other blocks", "[asset][foreign][dangling]")
{
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Hello World", 0, MakeSourceDataBlock(4, 1)));
    
    CHECK(markdown.size() == 7);
    
    Asset asset;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 7);
    REQUIRE(asset == "Lorem Ipsum4");
}

TEST_CASE("aparser/parse-foreign", "Parse body asset with foreign block inside")
{
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    MarkdownBlock foreign(ParagraphBlockType, "Hello World", 0, MakeSourceDataBlock(4, 1));
    MarkdownBlock::Stack::iterator pos = markdown.begin();
    std::advance(pos, 4);
    markdown.insert(pos, 1, foreign);
    
    CHECK(markdown.size() == 7);
    
    Asset asset;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 7);
    REQUIRE(asset == "Lorem Ipsum4");
}

TEST_CASE("Parse body asset with foreign list item inside", "[asset][foreign]")
{
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    MarkdownBlock::Stack foreign;
    foreign.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    foreign.push_back(MarkdownBlock(ListItemBlockEndType, "Foreign", 0, MakeSourceDataBlock(5, 1)));
    
    MarkdownBlock::Stack::iterator pos = markdown.begin();
    std::advance(pos, 5);
    markdown.insert(pos, foreign.begin(), foreign.end());
    
    CHECK(markdown.size() == 8);
    
    Asset asset;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 5);
    REQUIRE(asset == "Lorem Ipsum");
}

TEST_CASE("aparser/parse-multiline-signature", "Parse body asset with multiple lines in its signagure")
{
    // Blueprint in question:
    //R"(
    //+ Body
    //  A
    //
    //          Lorem Ipsum
    //)";
    
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    REQUIRE(markdown.size() == 6);
    
    markdown[2].content = "Body\n  A\n";
    
    Asset asset;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // expected code block
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
    REQUIRE(asset == "  A\nLorem Ipsum");
}

TEST_CASE("aparser/parse-multipart", "Parse body asset composed from multiple blocks")
{
    // Blueprint in question:
    //R"(
    //+ Body
    //  A
    //
    //  B
    //
    //          Lorem Ipsum
    //)";
    
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    REQUIRE(markdown.size() == 6);
    
    markdown[2].content = "Body\n  A\n";
    
    MarkdownBlock foreign(ParagraphBlockType, "B", 0, MakeSourceDataBlock(4, 1));
    MarkdownBlock::Stack::iterator pos = markdown.begin();
    std::advance(pos, 3);
    markdown.insert(pos, 1, foreign);
    
    Asset asset;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 2); // expected code block
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 7);
    REQUIRE(asset == "  A\n4Lorem Ipsum");
}

TEST_CASE("aparser/fix-body-not-parsed", "Issue: Object payload body isn't parsed")
{
    // http://github.com/apiaryio/snowcrash/issues/6
    // NOTE: caused by extra spaces before the body
    //
    // Blueprint in question:
    //R"(
    //+    Body
    //
    //          Lorem Ipsum
    //");
    
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    markdown[2].content = "   Body";
    
    Asset asset;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, asset);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
    REQUIRE(asset == "Lorem Ipsum");
}

