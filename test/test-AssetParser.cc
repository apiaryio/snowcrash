//
//  test-AssetParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/17/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "AssetParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer BodyAssetFixture = \
"+ Body\n"\
"\n"\
"        Lorem Ipsum\n";

const mdp::ByteBuffer SchemaAssetFixture = \
"+ Schema\n"\
"\n"\
"        Dolor Sit Amet\n";

TEST_CASE("Recognize explicit body signature", "[asset]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(BodyAssetFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Asset>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == BodySectionType);
}

TEST_CASE("Recognize body with content on signature", "[asset]")
{
    mdp::ByteBuffer source = \
    "+ Body\n"\
    "        Lorem Ipsum\n";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Asset>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == BodySectionType);
}

TEST_CASE("Recognize schema signature", "[asset]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(SchemaAssetFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Asset>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == SchemaSectionType);
}

TEST_CASE("Parse body asset", "[asset]")
{
    ParseResult<Asset> asset;
    SectionParserHelper<Asset, AssetParser>::parse(BodyAssetFixture, BodySectionType, asset, ExportSourcemapOption);

    REQUIRE(asset.report.error.code == Error::OK);
    REQUIRE(asset.report.warnings.empty());
    REQUIRE(asset.node == "Lorem Ipsum\n");

    REQUIRE(asset.sourceMap.sourceMap.size() == 1);
    REQUIRE(asset.sourceMap.sourceMap[0].location == 12);
    REQUIRE(asset.sourceMap.sourceMap[0].length == 16);
}

TEST_CASE("Parse schema asset", "[asset]")
{
    ParseResult<Asset> asset;
    SectionParserHelper<Asset, AssetParser>::parse(SchemaAssetFixture, SchemaSectionType, asset, ExportSourcemapOption);

    REQUIRE(asset.report.error.code == Error::OK);
    REQUIRE(asset.report.warnings.empty());
    REQUIRE(asset.node == "Dolor Sit Amet\n");

    REQUIRE(asset.sourceMap.sourceMap.size() == 1);
    REQUIRE(asset.sourceMap.sourceMap[0].location == 14);
    REQUIRE(asset.sourceMap.sourceMap[0].length == 19);
}

TEST_CASE("Foreign block inside", "[asset]")
{
    mdp::ByteBuffer source = BodyAssetFixture;
    source += \
    "\n"\
    "    Hello World!\n";

    ParseResult<Asset> asset;
    SectionParserHelper<Asset, AssetParser>::parse(source, BodySectionType, asset, ExportSourcemapOption);

    REQUIRE(asset.report.error.code == Error::OK);
    REQUIRE(asset.report.warnings.size() == 1);
    REQUIRE(asset.report.warnings[0].code == IndentationWarning);
    REQUIRE(asset.node == "Lorem Ipsum\nHello World!\n");

    REQUIRE(asset.sourceMap.sourceMap.size() == 2);
    REQUIRE(asset.sourceMap.sourceMap[0].location == 12);
    REQUIRE(asset.sourceMap.sourceMap[0].length == 17);
    REQUIRE(asset.sourceMap.sourceMap[1].location == 33);
    REQUIRE(asset.sourceMap.sourceMap[1].length == 13);
}

TEST_CASE("Nested list block inside", "[asset]")
{
    mdp::ByteBuffer source = BodyAssetFixture;
    source += \
    "\n"\
    "    + Hello World!\n";

    ParseResult<Asset> asset;
    SectionParserHelper<Asset, AssetParser>::parse(source, BodySectionType, asset, ExportSourcemapOption);

    REQUIRE(asset.report.error.code == Error::OK);
    REQUIRE(asset.report.warnings.size() == 1);
    REQUIRE(asset.report.warnings[0].code == IndentationWarning);
    REQUIRE(asset.node == "Lorem Ipsum\n+ Hello World!\n");

    REQUIRE(asset.sourceMap.sourceMap.size() == 2);
    REQUIRE(asset.sourceMap.sourceMap[0].location == 12);
    REQUIRE(asset.sourceMap.sourceMap[0].length == 16);
    REQUIRE(asset.sourceMap.sourceMap[1].location == 33);
    REQUIRE(asset.sourceMap.sourceMap[1].length == 15);
}

TEST_CASE("Multiline signature", "[asset]")
{
    mdp::ByteBuffer source = \
    "+ Body\n"
    "  Multiline Signature Content\n"\
    "\n"\
    "        Hello World!\n";

    ParseResult<Asset> asset;
    SectionParserHelper<Asset, AssetParser>::parse(source, BodySectionType, asset, ExportSourcemapOption);

    REQUIRE(asset.report.error.code == Error::OK);
    REQUIRE(asset.report.warnings.size() == 1);
    REQUIRE(asset.report.warnings[0].code == IndentationWarning);
    REQUIRE(asset.node == "Multiline Signature Content\nHello World!\n");

    REQUIRE(asset.sourceMap.sourceMap.size() == 3);
    REQUIRE(asset.sourceMap.sourceMap[0].location == 2);
    REQUIRE(asset.sourceMap.sourceMap[0].length == 5);
    REQUIRE(asset.sourceMap.sourceMap[1].location == 9);
    REQUIRE(asset.sourceMap.sourceMap[1].length == 29);
    REQUIRE(asset.sourceMap.sourceMap[2].location == 42);
    REQUIRE(asset.sourceMap.sourceMap[2].length == 17);
}

TEST_CASE("Multiple blocks", "[asset]")
{
    mdp::ByteBuffer source = \
    "+ Body\n"\
    "\n"\
    "    Block 1\n"\
    "\n"\
    "        Block 2\n"\
    "\n"\
    "    Block 3\n";

    ParseResult<Asset> asset;
    SectionParserHelper<Asset, AssetParser>::parse(source, BodySectionType, asset, ExportSourcemapOption);

    REQUIRE(asset.report.error.code == Error::OK);
    REQUIRE(asset.report.warnings.size() == 2);
    REQUIRE(asset.report.warnings[0].code == IndentationWarning);
    REQUIRE(asset.report.warnings[1].code == IndentationWarning);
    REQUIRE(asset.node == "Block 1\n\nBlock 2\nBlock 3\n");

    REQUIRE(asset.sourceMap.sourceMap.size() == 3);
    REQUIRE(asset.sourceMap.sourceMap[0].location == 12);
    REQUIRE(asset.sourceMap.sourceMap[0].length == 9);
    REQUIRE(asset.sourceMap.sourceMap[1].location == 25);
    REQUIRE(asset.sourceMap.sourceMap[1].length == 13);
    REQUIRE(asset.sourceMap.sourceMap[2].location == 42);
    REQUIRE(asset.sourceMap.sourceMap[2].length == 8);
}

TEST_CASE("Extra spaces before signature", "[asset]")
{
    mdp::ByteBuffer source = \
    "+   Body\n"\
    "\n"\
    "        Lorem Ipsum\n";

    ParseResult<Asset> asset;
    SectionParserHelper<Asset, AssetParser>::parse(source, BodySectionType, asset, ExportSourcemapOption);

    REQUIRE(asset.report.error.code == Error::OK);
    REQUIRE(asset.report.warnings.empty());
    REQUIRE(asset.node == "Lorem Ipsum\n");

    REQUIRE(asset.sourceMap.sourceMap.size() == 1);
    REQUIRE(asset.sourceMap.sourceMap[0].location == 14);
    REQUIRE(asset.sourceMap.sourceMap[0].length == 16);
}

TEST_CASE("Asset parser greediness", "[asset]")
{
    mdp::ByteBuffer source = BodyAssetFixture;
    source +=\
    "\n"\
    "+ Another Block\n";

    ParseResult<Asset> asset;
    SectionParserHelper<Asset, AssetParser>::parse(source, BodySectionType, asset, ExportSourcemapOption);

    REQUIRE(asset.report.error.code == Error::OK);
    REQUIRE(asset.report.warnings.empty());
    REQUIRE(asset.node == "Lorem Ipsum\n");

    REQUIRE(asset.sourceMap.sourceMap.size() == 1);
    REQUIRE(asset.sourceMap.sourceMap[0].location == 12);
    REQUIRE(asset.sourceMap.sourceMap[0].length == 16);
}
