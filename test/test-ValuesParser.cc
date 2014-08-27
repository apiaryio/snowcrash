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
    SectionType sectionType = SectionProcessor<Values, ValuesSM>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ValuesSectionType);
}

TEST_CASE("Parse canonical values", "[values]")
{
    Values values;
    Report report;
    SectionParserHelper<Values, ValuesSM, ValuesParser>::parse(ValuesFixture, ValuesSectionType, report, values);

    REQUIRE(report.error.code == Error::OK);
    CHECK(report.warnings.empty());

    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == "1234");
    REQUIRE(values[1] == "0000");
    REQUIRE(values[2] == "beef");
}

TEST_CASE("Warn superfluous content in values attribute", "[values]")
{
    mdp::ByteBuffer source = \
    "+ Values\n\n"\
    " extra\n\n"\
    "    + `Hello`\n";

    Values values;
    Report report;
    SectionParserHelper<Values, ValuesSM, ValuesParser>::parse(source, ValuesSectionType, report, values);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IgnoringWarning);

    REQUIRE(values.size() == 1);
    REQUIRE(values[0] == "Hello");
}

TEST_CASE("Warn about illegal entities in values attribute", "[values]")
{
    const std::string source = \
    "+ Values\n"\
    "    + `Hello`\n"\
    "    + illegal\n"\
    "    + `Hi`\n";

    Values values;
    Report report;
    SectionParserHelper<Values, ValuesSM, ValuesParser>::parse(source, ValuesSectionType, report, values);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);

    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == "Hello");
    REQUIRE(values[1] == "Hi");
}
