//
//  test-MSONOneOfParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONOneOfParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("OneOf block classifier", "[mson][one_of]")
{
    mdp::ByteBuffer source = \
    "- one Of";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(markdownAST.children().front().type == mdp::ListItemMarkdownNodeType);
    REQUIRE(!markdownAST.children().front().children().empty());

    sectionType = SectionProcessor<mson::OneOf>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONOneOfSectionType);

    markdownAST.children().front().children().front().text = "One of";
    sectionType = SectionProcessor<mson::OneOf>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONOneOfSectionType);
}

TEST_CASE("OneOf be tolerant on parsing input","[mson][one_of]")
{
    mdp::ByteBuffer source = \
    "- one \t Of";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    sectionType = SectionProcessor<mson::OneOf>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONOneOfSectionType);
}

TEST_CASE("Parse canonical mson one of", "[mson][one_of]")
{
    mdp::ByteBuffer source = \
    "- One of\n"\
    "  - state: Andhra Pradesh\n"\
    "  - province: Madras";

    ParseResult<mson::OneOf> oneOf;
    SectionParserHelper<mson::OneOf, MSONOneOfParser>::parseMSON(source, MSONOneOfSectionType, oneOf, ExportSourcemapOption);

    REQUIRE(oneOf.report.error.code == Error::OK);
    REQUIRE(oneOf.report.warnings.empty());

    REQUIRE(oneOf.node.size() == 2);

    REQUIRE(oneOf.node.at(0).klass == mson::Element::PropertyClass);
    REQUIRE(oneOf.node.at(0).content.property.name.literal == "state");
    REQUIRE(oneOf.node.at(0).content.property.valueDefinition.values.size() == 1);
    REQUIRE(oneOf.node.at(0).content.property.valueDefinition.values.at(0).literal == "Andhra Pradesh");
    REQUIRE(oneOf.node.at(0).content.property.sections.empty());
    REQUIRE(oneOf.node.at(0).content.property.description.empty());
    REQUIRE(oneOf.node.at(0).content.property.name.variable.empty());
    REQUIRE(oneOf.node.at(0).content.mixin.empty());
    REQUIRE(oneOf.node.at(0).content.value.empty());
    REQUIRE(oneOf.node.at(0).content.oneOf().empty());
    REQUIRE(oneOf.node.at(0).content.elements().empty());

    REQUIRE(oneOf.node.at(1).klass == mson::Element::PropertyClass);
    REQUIRE(oneOf.node.at(1).content.property.name.literal == "province");
    REQUIRE(oneOf.node.at(1).content.property.valueDefinition.values.size() == 1);
    REQUIRE(oneOf.node.at(1).content.property.valueDefinition.values.at(0).literal == "Madras");
    REQUIRE(oneOf.node.at(1).content.property.sections.empty());
    REQUIRE(oneOf.node.at(1).content.property.description.empty());
    REQUIRE(oneOf.node.at(1).content.property.name.variable.empty());

    REQUIRE(oneOf.sourceMap.collection.size() == 2);

    SourceMapHelper::check(oneOf.sourceMap.collection[0].property.name.sourceMap, 13, 22);
    SourceMapHelper::check(oneOf.sourceMap.collection[0].property.valueDefinition.sourceMap, 13, 22);
    SourceMapHelper::check(oneOf.sourceMap.collection[1].property.name.sourceMap, 37, 19);
    SourceMapHelper::check(oneOf.sourceMap.collection[1].property.valueDefinition.sourceMap, 37, 19);
}

TEST_CASE("Parse mson one of without any nested members", "[mson][one_of]")
{
    mdp::ByteBuffer source = "- One of\n";

    ParseResult<mson::OneOf> oneOf;
    SectionParserHelper<mson::OneOf, MSONOneOfParser>::parseMSON(source, MSONOneOfSectionType, oneOf, ExportSourcemapOption);

    REQUIRE(oneOf.report.error.code == Error::OK);
    REQUIRE(oneOf.report.warnings.size() == 1);
    REQUIRE(oneOf.report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(oneOf.node.empty());
    REQUIRE(oneOf.sourceMap.collection.empty());
}

TEST_CASE("Parse mson one of with one of", "[mson][one_of]")
{
    mdp::ByteBuffer source = \
    "- One of\n"\
    "    - last_name\n"\
    "    - one of\n"\
    "        - given_name\n"\
    "        - suffixed_name";

    ParseResult<mson::OneOf> oneOf;
    SectionParserHelper<mson::OneOf, MSONOneOfParser>::parseMSON(source, MSONOneOfSectionType, oneOf, ExportSourcemapOption);

    REQUIRE(oneOf.report.error.code == Error::OK);
    REQUIRE(oneOf.report.warnings.empty());

    REQUIRE(oneOf.node.size() == 2);

    REQUIRE(oneOf.node.at(0).klass == mson::Element::PropertyClass);
    REQUIRE(oneOf.node.at(0).content.property.name.literal == "last_name");
    REQUIRE(oneOf.node.at(0).content.property.sections.empty());
    REQUIRE(oneOf.node.at(0).content.property.description.empty());
    REQUIRE(oneOf.node.at(0).content.property.valueDefinition.empty());

    REQUIRE(oneOf.node.at(1).klass == mson::Element::OneOfClass);
    REQUIRE(oneOf.node.at(1).content.oneOf().size() == 2);
    REQUIRE(oneOf.node.at(1).content.oneOf().at(0).klass == mson::Element::PropertyClass);
    REQUIRE(oneOf.node.at(1).content.oneOf().at(0).content.property.name.literal == "given_name");
    REQUIRE(oneOf.node.at(1).content.oneOf().at(0).content.property.valueDefinition.empty());
    REQUIRE(oneOf.node.at(1).content.oneOf().at(1).klass == mson::Element::PropertyClass);
    REQUIRE(oneOf.node.at(1).content.oneOf().at(1).content.property.name.literal == "suffixed_name");
    REQUIRE(oneOf.node.at(1).content.oneOf().at(1).content.property.valueDefinition.empty());

    REQUIRE(oneOf.sourceMap.collection.size() == 2);

    SourceMapHelper::check(oneOf.sourceMap.collection[0].property.name.sourceMap, 15, 10);
    REQUIRE(oneOf.sourceMap.collection[0].property.valueDefinition.sourceMap.empty());

    REQUIRE(oneOf.sourceMap.collection[1].oneOf().collection.size() == 2);
    SourceMapHelper::check(oneOf.sourceMap.collection[1].oneOf().collection[0].property.name.sourceMap, 48, 11);
    SourceMapHelper::check(oneOf.sourceMap.collection[1].oneOf().collection[1].property.name.sourceMap, 67, 16);
}

TEST_CASE("Parse mson one of with member group", "[mson][one_of]")
{
    mdp::ByteBuffer source = \
    "- One Of\n"\
    "    - full_name\n"\
    "    - Properties\n"\
    "        - first_name\n"\
    "        - last_name";

    ParseResult<mson::OneOf> oneOf;
    SectionParserHelper<mson::OneOf, MSONOneOfParser>::parseMSON(source, MSONOneOfSectionType, oneOf, ExportSourcemapOption);

    REQUIRE(oneOf.report.error.code == Error::OK);
    REQUIRE(oneOf.report.warnings.empty());

    REQUIRE(oneOf.node.size() == 2);
    REQUIRE(oneOf.node.at(0).klass == mson::Element::PropertyClass);
    REQUIRE(oneOf.node.at(0).content.property.name.literal == "full_name");
    REQUIRE(oneOf.node.at(0).content.property.sections.empty());
    REQUIRE(oneOf.node.at(0).content.property.description.empty());
    REQUIRE(oneOf.node.at(0).content.property.valueDefinition.empty());
    REQUIRE(oneOf.node.at(1).klass == mson::Element::GroupClass);
    REQUIRE(oneOf.node.at(1).content.property.empty());
    REQUIRE(oneOf.node.at(1).content.mixin.empty());
    REQUIRE(oneOf.node.at(1).content.value.empty());
    REQUIRE(oneOf.node.at(1).content.elements().size() == 2);
    REQUIRE(oneOf.node.at(1).content.elements().at(0).klass == mson::Element::PropertyClass);
    REQUIRE(oneOf.node.at(1).content.elements().at(0).content.property.name.literal == "first_name");
    REQUIRE(oneOf.node.at(1).content.elements().at(1).klass == mson::Element::PropertyClass);
    REQUIRE(oneOf.node.at(1).content.elements().at(1).content.property.name.literal == "last_name");

    REQUIRE(oneOf.sourceMap.collection.size() == 2);

    SourceMapHelper::check(oneOf.sourceMap.collection[0].property.name.sourceMap, 15, 10);
    REQUIRE(oneOf.sourceMap.collection[0].property.valueDefinition.sourceMap.empty());

    REQUIRE(oneOf.sourceMap.collection[1].elements().collection.size() == 2);
    SourceMapHelper::check(oneOf.sourceMap.collection[1].elements().collection[0].property.name.sourceMap, 52, 11);
    SourceMapHelper::check(oneOf.sourceMap.collection[1].elements().collection[1].property.name.sourceMap, 71, 12);
}
