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

TEST_CASE("recognize explicit body signature", "[asset]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(BodyAssetFixture, markdownAST);
    
    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Asset, AssetSM>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == BodySectionType);
}

TEST_CASE("recognize body with content on signature", "[asset]")
{
    mdp::ByteBuffer source = \
    "+ Body\n"\
    "        Lorem Ipsum\n";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Asset, AssetSM>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == BodySectionType);
}

TEST_CASE("recognize schema signature", "[asset]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(SchemaAssetFixture, markdownAST);
    
    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Asset, AssetSM>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == SchemaSectionType);
}

TEST_CASE("parse body asset", "[asset]")
{
    Asset asset;
    Report report;
    SectionParserHelper<Asset, AssetSM, AssetParser>::parse(BodyAssetFixture, BodySectionType, report, asset);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    REQUIRE(asset == "Lorem Ipsum\n");
}

TEST_CASE("parse schema asset", "[asset]")
{
    Asset asset;
    Report report;
    SectionParserHelper<Asset, AssetSM, AssetParser>::parse(SchemaAssetFixture, SchemaSectionType, report, asset);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    REQUIRE(asset == "Dolor Sit Amet\n");
}

TEST_CASE("Foreign block inside", "[asset]")
{
    mdp::ByteBuffer source = BodyAssetFixture;
    source += \
    "\n"\
    "    Hello World!\n";
    
    Asset asset;
    Report report;
    SectionParserHelper<Asset, AssetSM, AssetParser>::parse(source, BodySectionType, report, asset);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IndentationWarning);
    REQUIRE(asset == "Lorem Ipsum\nHello World!\n");
}

TEST_CASE("Nested list block inside", "[asset]")
{
    mdp::ByteBuffer source = BodyAssetFixture;
    source += \
    "\n"\
    "    + Hello World!\n";
    
    Asset asset;
    Report report;
    SectionParserHelper<Asset, AssetSM, AssetParser>::parse(source, BodySectionType, report, asset);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IndentationWarning);
    REQUIRE(asset == "Lorem Ipsum\n+ Hello World!\n");
}

TEST_CASE("Multiline signature", "[asset]")
{
    mdp::ByteBuffer source = \
    "+ Body\n"
    "  Multiline Signature Content\n"\
    "\n"\
    "        Hello World!\n";
    
    Asset asset;
    Report report;
    SectionParserHelper<Asset, AssetSM, AssetParser>::parse(source, BodySectionType, report, asset);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IndentationWarning);
    REQUIRE(asset == "Multiline Signature Content\nHello World!\n");
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
    
    Asset asset;
    Report report;
    SectionParserHelper<Asset, AssetSM, AssetParser>::parse(source, BodySectionType, report, asset);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 2);
    REQUIRE(report.warnings[0].code == IndentationWarning);
    REQUIRE(report.warnings[1].code == IndentationWarning);
    REQUIRE(asset == "Block 1\n\nBlock 2\nBlock 3\n");
}

TEST_CASE("Extra spaces before signature", "[asset]")
{
    mdp::ByteBuffer source = \
    "+   Body\n"\
    "\n"\
    "        Lorem Ipsum\n";
    
    Asset asset;
    Report report;
    SectionParserHelper<Asset, AssetSM, AssetParser>::parse(source, BodySectionType, report, asset);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    REQUIRE(asset == "Lorem Ipsum\n");
}

TEST_CASE("Asset parser greediness", "[asset]")
{
    mdp::ByteBuffer source = BodyAssetFixture;
    source +=\
    "\n"\
    "+ Another Block\n";
    
    Asset asset;
    Report report;
    SectionParserHelper<Asset, AssetSM, AssetParser>::parse(source, BodySectionType, report, asset);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    REQUIRE(asset == "Lorem Ipsum\n");
}
