//
//  test-AssetParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/17/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "AssetParser.h"

using namespace snowcrash;

MarkdownBlock::Stack CanonicalBodyAssetFixture()
{
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Lorem Ipsum", 0, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    return markdown;
}

SourceData CanonicalBodyAssetSourceDataFixture = "0123456789ABCDEFGHIJKLMNOPQRST";

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
    SourceData source = CanonicalBodyAssetSourceDataFixture;
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    REQUIRE(GetAssetSignature(markdown.begin(), markdown.end()) == BodyAssetSignature);
    REQUIRE(GetAssetSignature(++BlockIterator(markdown.begin()), markdown.end()) == BodyAssetSignature);
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

TEST_CASE("aparser/classifier", "Asset block classifier")
{
    SourceData source = CanonicalBodyAssetSourceDataFixture;
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    CHECK(markdown.size() == 6);
    
    BlockIterator cur = markdown.begin();
    // ListBlockBeginType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSection) == BodySection);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), BodySection) == BodySection);
    
    ++cur; // ListItemBlockBeginType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSection) == BodySection);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), BodySection) == UndefinedSection); // treat new list item as foreign
    
    ++cur; // ParagraphBlockType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), BodySection) == BodySection);

    ++cur; // CodeBlockType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), BodySection) == BodySection);
    
    ++cur; // ListItemBlockEndType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), BodySection) == UndefinedSection);
    
    ++cur; // ListBlockEndType
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Asset>(cur, markdown.end(), BodySection) == UndefinedSection);
}

TEST_CASE("aparser/parse", "Parse body asset")
{
    SourceData source = CanonicalBodyAssetSourceDataFixture;
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    CHECK(markdown.size() == 6);
    
    Asset asset;
    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), asset);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
    REQUIRE(asset == "Lorem Ipsum");
}

TEST_CASE("aparser/parse-ajdacent", "Parse body asset followed by other blocks")
{
    SourceData source = CanonicalBodyAssetSourceDataFixture;
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Hello World", 0, MakeSourceDataBlock(4, 1)));
    
    CHECK(markdown.size() == 7);
    
    Asset asset;
    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), asset);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
    REQUIRE(asset == "Lorem Ipsum");
}

TEST_CASE("aparser/parse-foreign", "Parse body asset with foreign block inside")
{
    SourceData source = CanonicalBodyAssetSourceDataFixture;
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    MarkdownBlock foreign(ParagraphBlockType, "Hello World", 0, MakeSourceDataBlock(4, 1));
    MarkdownBlock::Stack::iterator pos = markdown.begin();
    std::advance(pos, 4);
    markdown.insert(pos, 1, foreign);
    
    CHECK(markdown.size() == 7);
    
    Asset asset;
    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), asset);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 7);
    REQUIRE(asset == "Lorem Ipsum4");
}

TEST_CASE("aparser/parse-foreign-listitem", "Parse body asset with foreign list item inside")
{
    SourceData source = CanonicalBodyAssetSourceDataFixture;
    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
    
    MarkdownBlock::Stack foreign;
    foreign.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    foreign.push_back(MarkdownBlock(ListItemBlockEndType, "Foreign", 0, MakeSourceDataBlock(5, 1)));
    
    MarkdownBlock::Stack::iterator pos = markdown.begin();
    std::advance(pos, 5);
    markdown.insert(pos, foreign.begin(), foreign.end());
    
    CHECK(markdown.size() == 8);
    
    Asset asset;
    ParseSectionResult result = AssetParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), asset);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 5);
    REQUIRE(asset == "Lorem Ipsum");
}
