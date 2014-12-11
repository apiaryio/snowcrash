//
//  test-MSONMixinParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONMixinParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Mixin block classifier", "[mson][mixin]")
{
    mdp::ByteBuffer source = "- Include Person";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(markdownAST.children().front().type == mdp::ListItemMarkdownNodeType);
    REQUIRE(!markdownAST.children().front().children().empty());

    sectionType = SectionProcessor<mson::Mixin>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONMixinSectionType);

    markdownAST.children().front().children().front().text = "Include (Address, sample)";
    sectionType = SectionProcessor<mson::Mixin>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONMixinSectionType);
}

TEST_CASE("Parse canonical mson mixin", "[mson][mixin]")
{
    mdp::ByteBuffer source = "- Include Person";

    NamedTypes namedTypes;
    NamedTypeHelper::build("Person", mson::ObjectBaseType, namedTypes);

    ParseResult<mson::Mixin> mixin;
    SectionParserHelper<mson::Mixin, MSONMixinParser>::parseMSON(source, MSONMixinSectionType, mixin, 0, namedTypes);

    REQUIRE(mixin.report.error.code == Error::OK);
    REQUIRE(mixin.report.warnings.empty());

    REQUIRE(mixin.node.typeDefinition.attributes == 0);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.name.name == mson::UndefinedTypeName);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.name.symbol.literal == "Person");
    REQUIRE(mixin.node.typeDefinition.typeSpecification.name.symbol.variable == false);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.nestedTypes.empty());
}

TEST_CASE("Parse mson mixin with canonical type definition", "[mson][mixin]")
{
    mdp::ByteBuffer source = "- Include (Person, sample)";

    NamedTypes namedTypes;
    NamedTypeHelper::build("Person", mson::ObjectBaseType, namedTypes);

    ParseResult<mson::Mixin> mixin;
    SectionParserHelper<mson::Mixin, MSONMixinParser>::parseMSON(source, MSONMixinSectionType, mixin, 0, namedTypes);

    REQUIRE(mixin.report.error.code == Error::OK);
    REQUIRE(mixin.report.warnings.empty());

    REQUIRE(mixin.node.typeDefinition.attributes == mson::SampleTypeAttribute);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.name.name == mson::UndefinedTypeName);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.name.symbol.literal == "Person");
    REQUIRE(mixin.node.typeDefinition.typeSpecification.name.symbol.variable == false);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.nestedTypes.empty());
}

TEST_CASE("Parse mson mixin with base type definition", "[mson][mixin]")
{
    mdp::ByteBuffer source = "- Include (string)";

    ParseResult<mson::Mixin> mixin;
    SectionParserHelper<mson::Mixin, MSONMixinParser>::parse(source, MSONMixinSectionType, mixin);

    REQUIRE(mixin.report.error.code == Error::OK);
    REQUIRE(mixin.report.warnings.size() == 1);
    REQUIRE(mixin.report.warnings[0].code == FormattingWarning);

    REQUIRE(mixin.node.typeDefinition.attributes == 0);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.name.name == mson::StringTypeName);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.name.symbol.empty());
    REQUIRE(mixin.node.typeDefinition.typeSpecification.nestedTypes.empty());
}

TEST_CASE("Parse mson mixin with nested type definition", "[mson][mixin]")
{
    mdp::ByteBuffer source = "- Include (Person[number, string], required)";

    NamedTypes namedTypes;
    NamedTypeHelper::build("Person", mson::ValueBaseType, namedTypes);

    ParseResult<mson::Mixin> mixin;
    SectionParserHelper<mson::Mixin, MSONMixinParser>::parseMSON(source, MSONMixinSectionType, mixin, 0, namedTypes);

    REQUIRE(mixin.report.error.code == Error::OK);
    REQUIRE(mixin.report.warnings.empty());

    REQUIRE(mixin.node.typeDefinition.attributes == mson::RequiredTypeAttribute);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.name.name == mson::UndefinedTypeName);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.name.symbol.literal == "Person");
    REQUIRE(mixin.node.typeDefinition.typeSpecification.name.symbol.variable == false);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.nestedTypes.size() == 2);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.nestedTypes[0].name == mson::NumberTypeName);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.nestedTypes[0].symbol.empty());
    REQUIRE(mixin.node.typeDefinition.typeSpecification.nestedTypes[1].name == mson::StringTypeName);
    REQUIRE(mixin.node.typeDefinition.typeSpecification.nestedTypes[1].symbol.empty());
}
