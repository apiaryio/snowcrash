//
//  test-RelationParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 04/03/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "RelationParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

mdp::ByteBuffer RelationFixture = "+ Relation: create";

TEST_CASE("Recognize relation signature", "[relation]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(RelationFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<Relation>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == RelationSectionType);
}

TEST_CASE("Relation signature without colon", "[relation]")
{
    mdp::ByteBuffer source = "+ Relation delete";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<Relation>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType != RelationSectionType);
}

TEST_CASE("Parse canonical link relation", "[relation]")
{
    ParseResult<Relation> relation;
    SectionParserHelper<Relation, RelationParser>::parse(RelationFixture, RelationSectionType, relation, ExportSourcemapOption);

    REQUIRE(relation.report.error.code == Error::OK);
    REQUIRE(relation.report.warnings.empty());

    REQUIRE(relation.node.str == "create");
    SourceMapHelper::check(relation.sourceMap.sourceMap, 0, 19);
}

TEST_CASE("Relation identifier starting with non lower alphabet", "[relation]")
{
    mdp::ByteBuffer source = "+ Relation: 9delete";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<Relation>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == RelationSectionType);

    ParseResult<Relation> relation;
    SectionParserHelper<Relation, RelationParser>::parse(source, RelationSectionType, relation, ExportSourcemapOption);

    REQUIRE(relation.report.error.code == Error::OK);
    REQUIRE(relation.report.warnings.size() == 1);
    REQUIRE(relation.report.warnings[0].code == FormattingWarning);
    REQUIRE(relation.node.str.empty());
}

TEST_CASE("Relation identifier containing capital letters", "[relation]")
{
    mdp::ByteBuffer source = "+ Relation: deLete";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<Relation>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == RelationSectionType);

    ParseResult<Relation> relation;
    SectionParserHelper<Relation, RelationParser>::parse(source, RelationSectionType, relation, ExportSourcemapOption);

    REQUIRE(relation.report.error.code == Error::OK);
    REQUIRE(relation.report.warnings.size() == 1);
    REQUIRE(relation.report.warnings[0].code == FormattingWarning);
    REQUIRE(relation.node.str.empty());
}

TEST_CASE("Relation identifier containing special characters", "[relation]")
{
    mdp::ByteBuffer source = "+ Relation: del*et_e";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<Relation>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == RelationSectionType);

    ParseResult<Relation> relation;
    SectionParserHelper<Relation, RelationParser>::parse(source, RelationSectionType, relation, ExportSourcemapOption);

    REQUIRE(relation.report.error.code == Error::OK);
    REQUIRE(relation.report.warnings.size() == 1);
    REQUIRE(relation.report.warnings[0].code == FormattingWarning);
    REQUIRE(relation.node.str.empty());
}

TEST_CASE("Relation identifier consisting of dots and dashes", "[relation]")
{
    mdp::ByteBuffer source = "+ Relation: delete-task.2";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<Relation>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == RelationSectionType);

    ParseResult<Relation> relation;
    SectionParserHelper<Relation, RelationParser>::parse(source, RelationSectionType, relation, ExportSourcemapOption);

    REQUIRE(relation.report.error.code == Error::OK);
    REQUIRE(relation.report.warnings.empty());

    REQUIRE(relation.node.str == "delete-task.2");
}
