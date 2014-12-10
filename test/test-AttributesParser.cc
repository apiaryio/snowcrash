//
//  test-AttributesParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/25/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "AttributesParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer AttributesFixture = \
"+ Attributes (array[[Coupon](#coupon)])";

TEST_CASE("Recognize explicit attributes signature", "[attributes]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(AttributesFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<Attributes>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == AttributesSectionType);
}

TEST_CASE("Parse canonical attributes", "[attributes]")
{
    ParseResult<Attributes> attributes;
    SectionParserHelper<Attributes, AttributesParser>::parse(AttributesFixture, AttributesSectionType, attributes, ExportSourcemapOption);

    REQUIRE(attributes.report.error.code == Error::OK);
    REQUIRE(attributes.report.warnings.empty());

    REQUIRE(attributes.node.resolved.empty());
    REQUIRE(attributes.node.source.name.empty());
    REQUIRE(attributes.node.source.sections.empty());
    REQUIRE(attributes.node.source.base.attributes == 0);
    REQUIRE(attributes.node.source.base.typeSpecification.name.name == mson::ArrayTypeName);
    REQUIRE(attributes.node.source.base.typeSpecification.nestedTypes.size() == 1);
    REQUIRE(attributes.node.source.base.typeSpecification.nestedTypes[0].symbol.literal == "Coupon");
    REQUIRE(attributes.node.source.base.baseType == mson::ValueBaseType);
}

TEST_CASE("Parse attributes with nested members", "[attributes]")
{
    mdp::ByteBuffer source = \
    "+ attribute\n"\
    "    + message (string) - The blog post article\n"\
    "    + author: john@appleseed.com (string) - Author of the blog post";

    mson::MemberType member;
    ParseResult<Attributes> attributes;
    SectionParserHelper<Attributes, AttributesParser>::parse(source, AttributesSectionType, attributes, ExportSourcemapOption);

    REQUIRE(attributes.report.error.code == Error::OK);
    REQUIRE(attributes.report.warnings.empty());

    REQUIRE(attributes.node.resolved.empty());
    REQUIRE(attributes.node.source.name.empty());
    REQUIRE(attributes.node.source.base.attributes == 0);
    REQUIRE(attributes.node.source.base.typeSpecification.empty());
    REQUIRE(attributes.node.source.base.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(attributes.node.source.sections.size() == 1);
    REQUIRE(attributes.node.source.sections[0].type == mson::TypeSection::MemberType);
    REQUIRE(attributes.node.source.sections[0].content.members().size() == 2);

    member = attributes.node.source.sections[0].content.members().at(0);
    REQUIRE(member.type == mson::MemberType::PropertyType);
    REQUIRE(member.content.property.name.literal == "message");
    REQUIRE(member.content.property.description == "The blog post article");
    REQUIRE(member.content.property.valueDefinition.values.empty());
    REQUIRE(member.content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::StringTypeName);

    member = attributes.node.source.sections[0].content.members().at(1);
    REQUIRE(member.type == mson::MemberType::PropertyType);
    REQUIRE(member.content.property.name.literal == "author");
    REQUIRE(member.content.property.description == "Author of the blog post");
    REQUIRE(member.content.property.valueDefinition.values.size() == 1);
    REQUIRE(member.content.property.valueDefinition.values[0].literal == "john@appleseed.com");
    REQUIRE(member.content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::StringTypeName);
}

TEST_CASE("Parse attributes with block description", "[attributes]")
{
    mdp::ByteBuffer source = \
    "+ Attributes\n"\
    "    Awesome description\n\n"\
    "    + With list\n"\
    "    + Properties\n"\
    "        + message (string)";

    ParseResult<Attributes> attributes;
    SectionParserHelper<Attributes, AttributesParser>::parse(source, AttributesSectionType, attributes, ExportSourcemapOption);

    REQUIRE(attributes.report.error.code == Error::OK);
    REQUIRE(attributes.report.warnings.empty());

    REQUIRE(attributes.node.resolved.empty());
    REQUIRE(attributes.node.source.name.empty());
    REQUIRE(attributes.node.source.base.attributes == 0);
    REQUIRE(attributes.node.source.base.typeSpecification.empty());
    REQUIRE(attributes.node.source.base.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(attributes.node.source.sections.size() == 2);
    REQUIRE(attributes.node.source.sections[0].type == mson::TypeSection::BlockDescriptionType);
    REQUIRE(attributes.node.source.sections[0].content.description == "Awesome description\n\n+ With list\n");
    REQUIRE(attributes.node.source.sections[1].type == mson::TypeSection::MemberType);
    REQUIRE(attributes.node.source.sections[1].content.members().size() == 1);
}
