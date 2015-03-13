//
//  test-MSONParameterParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 12/03/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONParameterParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer ParameterFixture = \
"+ id: `0000` (enum[number], optional) - Description\n\n"\
"    + Default: `1234`\n"\
"    + Members\n"\
"        + `1234` - The default value\n"\
"        + `0000`\n"\
"        + `beef` - I like beef\n";

TEST_CASE("Parse canonical parameter definition with new syntax", "[mson_parameter]")
{
    ParseResult<MSONParameter> parameter;
    SectionParserHelper<MSONParameter, MSONParameterParser>::parse(ParameterFixture, MSONParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.description == "Description");
    REQUIRE(parameter.node.type == "number");
    REQUIRE(parameter.node.use == OptionalParameterUse);
    REQUIRE(parameter.node.defaultValue == "1234");
    REQUIRE(parameter.node.exampleValue == "0000");
    REQUIRE(parameter.node.values.size() == 3);
    REQUIRE(parameter.node.values[0] == "1234");
    REQUIRE(parameter.node.values[1] == "0000");
    REQUIRE(parameter.node.values[2] == "beef");

    SourceMapHelper::check(parameter.sourceMap.name.sourceMap, 2, 50);
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 2, 50);
    SourceMapHelper::check(parameter.sourceMap.use.sourceMap, 2, 50);
    SourceMapHelper::check(parameter.sourceMap.type.sourceMap, 2, 50);
    SourceMapHelper::check(parameter.sourceMap.exampleValue.sourceMap, 2, 50);
    SourceMapHelper::check(parameter.sourceMap.defaultValue.sourceMap, 59, 16);
    REQUIRE(parameter.sourceMap.values.collection.size() == 3);
    SourceMapHelper::check(parameter.sourceMap.values.collection[0].sourceMap, 99, 27);
    SourceMapHelper::check(parameter.sourceMap.values.collection[1].sourceMap, 136, 7);
    SourceMapHelper::check(parameter.sourceMap.values.collection[2].sourceMap, 153, 21);
}

TEST_CASE("Parse parameter description when it occurs in different cases", "[mson_parameter]")
{
    mdp::ByteBuffer source = \
    "+ id: 100 - Same line\n"\
    "    Single newline\n\n"\
    "    Double newline\n";

    ParseResult<MSONParameter> parameter;
    SectionParserHelper<MSONParameter, MSONParameterParser>::parse(source, MSONParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.empty());

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.description == "Same line\nSingle newline\n\nDouble newline\n");
    REQUIRE(parameter.node.exampleValue == "100");

    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 2, 20, 1);
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 26, 16, 2);
    SourceMapHelper::check(parameter.sourceMap.description.sourceMap, 46, 15, 3);
}

TEST_CASE("Parse parameter when it has more than 2 traits", "[mson_parameter]")
{
    mdp::ByteBuffer source = "+ id: 100 (optional, number, tagging)";

    ParseResult<MSONParameter> parameter;
    SectionParserHelper<MSONParameter, MSONParameterParser>::parse(source, MSONParameterSectionType, parameter, ExportSourcemapOption);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == FormattingWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.use == UndefinedParameterUse);
    REQUIRE(parameter.node.type.empty());

    REQUIRE(parameter.sourceMap.type.sourceMap.empty());
    REQUIRE(parameter.sourceMap.use.sourceMap.empty());
}

TEST_CASE("Warn about implicit required vs default clash in new parameter", "[mson_parameter]")
{
    mdp::ByteBuffer source = \
    "+ id\n"\
    "    + Default: 42";

    ParseResult<MSONParameter> parameter;
    SectionParserHelper<MSONParameter, MSONParameterParser>::parse(source, MSONParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.use == UndefinedParameterUse);
    REQUIRE(parameter.node.defaultValue == "42");
}

TEST_CASE("Warn missing example item in values in new parameter syntax", "[mson_parameter]")
{
    mdp::ByteBuffer source = \
    "+ id: `Value1` (optional, enum[string])\n"\
    "    + Default: `Value2`\n"\
    "    + Members\n"\
    "        + `Value2`\n";

    ParseResult<MSONParameter> parameter;
    SectionParserHelper<MSONParameter, MSONParameterParser>::parse(source, MSONParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.exampleValue == "Value1");
    REQUIRE(parameter.node.defaultValue == "Value2");
}

TEST_CASE("Warn missing default value in values in new parameter syntax", "[mson_parameter]")
{
    mdp::ByteBuffer source = \
    "+ id: `Value2` (optional, enum[string])\n"\
    "    + Default: `Value1`\n"\
    "    + Members\n"\
    "        + `Value2`\n";

    ParseResult<MSONParameter> parameter;
    SectionParserHelper<MSONParameter, MSONParameterParser>::parse(source, MSONParameterSectionType, parameter);

    REQUIRE(parameter.report.error.code == Error::OK);
    REQUIRE(parameter.report.warnings.size() == 1);
    REQUIRE(parameter.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(parameter.node.name == "id");
    REQUIRE(parameter.node.exampleValue == "Value2");
    REQUIRE(parameter.node.defaultValue == "Value1");
}
