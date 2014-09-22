//
//  test-ValuesParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 6/12/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "ValuesParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer ValuesFixture = \
"+ Values\n"\
"    + `1234`\n"\
"    + `0000`\n"\
"    + `beef`\n"\
"";

TEST_CASE("Recognize values signature", "[values]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(ValuesFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Values>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ValuesSectionType);
}

TEST_CASE("Parse canonical values", "[values]")
{
    ParseResult<Values> values;
    SectionParserHelper<Values, ValuesParser>::parse(ValuesFixture, ValuesSectionType, values);

    REQUIRE(values.report.error.code == Error::OK);
    CHECK(values.report.warnings.empty());

    REQUIRE(values.node.size() == 3);
    REQUIRE(values.node[0] == "1234");
    REQUIRE(values.node[1] == "0000");
    REQUIRE(values.node[2] == "beef");

    REQUIRE(values.sourceMap.list.size() == 3);
    REQUIRE(values.sourceMap.list[0].sourceMap.size() == 1);
    REQUIRE(values.sourceMap.list[0].sourceMap[0].location == 13);
    REQUIRE(values.sourceMap.list[0].sourceMap[0].length == 9);
    REQUIRE(values.sourceMap.list[1].sourceMap.size() == 1);
    REQUIRE(values.sourceMap.list[1].sourceMap[0].location == 26);
    REQUIRE(values.sourceMap.list[1].sourceMap[0].length == 9);
    REQUIRE(values.sourceMap.list[2].sourceMap.size() == 1);
    REQUIRE(values.sourceMap.list[2].sourceMap[0].location == 39);
    REQUIRE(values.sourceMap.list[2].sourceMap[0].length == 9);
}

TEST_CASE("Warn superfluous content in values attribute", "[values]")
{
    mdp::ByteBuffer source = \
    "+ Values\n\n"\
    " extra\n\n"\
    "    + `Hello`\n";

    ParseResult<Values> values;
    SectionParserHelper<Values, ValuesParser>::parse(source, ValuesSectionType, values);

    REQUIRE(values.report.error.code == Error::OK);
    REQUIRE(values.report.warnings.size() == 1);
    REQUIRE(values.report.warnings[0].code == IgnoringWarning);

    REQUIRE(values.node.size() == 1);
    REQUIRE(values.node[0] == "Hello");

    REQUIRE(values.sourceMap.list.size() == 1);
    REQUIRE(values.sourceMap.list[0].sourceMap.size() == 1);
    REQUIRE(values.sourceMap.list[0].sourceMap[0].location == 22);
    REQUIRE(values.sourceMap.list[0].sourceMap[0].length == 10);
}

TEST_CASE("Warn about illegal entities in values attribute", "[values]")
{
    const std::string source = \
    "+ Values\n"\
    "    + `Hello`\n"\
    "    + illegal\n"\
    "    + `Hi`\n";

    ParseResult<Values> values;
    SectionParserHelper<Values, ValuesParser>::parse(source, ValuesSectionType, values);

    REQUIRE(values.report.error.code == Error::OK);
    REQUIRE(values.report.warnings.size() == 1);

    REQUIRE(values.node.size() == 2);
    REQUIRE(values.node[0] == "Hello");
    REQUIRE(values.node[1] == "Hi");

    REQUIRE(values.sourceMap.list.size() == 2);
    REQUIRE(values.sourceMap.list[0].sourceMap.size() == 1);
    REQUIRE(values.sourceMap.list[0].sourceMap[0].location == 13);
    REQUIRE(values.sourceMap.list[0].sourceMap[0].length == 10);
    REQUIRE(values.sourceMap.list[1].sourceMap.size() == 1);
    REQUIRE(values.sourceMap.list[1].sourceMap[0].location == 41);
    REQUIRE(values.sourceMap.list[1].sourceMap[0].length == 7);
}
