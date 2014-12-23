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
    REQUIRE(attributes.node.source.typeDefinition.attributes == 0);
    REQUIRE(attributes.node.source.typeDefinition.typeSpecification.name.base == mson::ArrayTypeName);
    REQUIRE(attributes.node.source.typeDefinition.typeSpecification.nestedTypes.size() == 1);
    REQUIRE(attributes.node.source.typeDefinition.typeSpecification.nestedTypes[0].symbol.literal == "Coupon");
    REQUIRE(attributes.node.source.typeDefinition.baseType == mson::ValueBaseType);
}

TEST_CASE("Parse attributes with nested members", "[attributes]")
{
    mdp::ByteBuffer source = \
    "+ attribute\n"\
    "    + message (string) - The blog post article\n"\
    "    + author: john@appleseed.com (string) - Author of the blog post";

    mson::Element element;
    ParseResult<Attributes> attributes;
    SectionParserHelper<Attributes, AttributesParser>::parse(source, AttributesSectionType, attributes, ExportSourcemapOption);

    REQUIRE(attributes.report.error.code == Error::OK);
    REQUIRE(attributes.report.warnings.empty());

    REQUIRE(attributes.node.resolved.empty());
    REQUIRE(attributes.node.source.name.empty());
    REQUIRE(attributes.node.source.typeDefinition.attributes == 0);
    REQUIRE(attributes.node.source.typeDefinition.typeSpecification.empty());
    REQUIRE(attributes.node.source.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(attributes.node.source.sections.size() == 1);
    REQUIRE(attributes.node.source.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(attributes.node.source.sections[0].content.elements().size() == 2);

    element = attributes.node.source.sections[0].content.elements().at(0);
    REQUIRE(element.klass == mson::Element::PropertyClass);
    REQUIRE(element.content.property.name.literal == "message");
    REQUIRE(element.content.property.description == "The blog post article");
    REQUIRE(element.content.property.valueDefinition.values.empty());
    REQUIRE(element.content.property.valueDefinition.typeDefinition.typeSpecification.name.base == mson::StringTypeName);

    element = attributes.node.source.sections[0].content.elements().at(1);
    REQUIRE(element.klass == mson::Element::PropertyClass);
    REQUIRE(element.content.property.name.literal == "author");
    REQUIRE(element.content.property.description == "Author of the blog post");
    REQUIRE(element.content.property.valueDefinition.values.size() == 1);
    REQUIRE(element.content.property.valueDefinition.values[0].literal == "john@appleseed.com");
    REQUIRE(element.content.property.valueDefinition.typeDefinition.typeSpecification.name.base == mson::StringTypeName);
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
    REQUIRE(attributes.node.source.typeDefinition.attributes == 0);
    REQUIRE(attributes.node.source.typeDefinition.typeSpecification.empty());
    REQUIRE(attributes.node.source.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(attributes.node.source.sections.size() == 2);
    REQUIRE(attributes.node.source.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(attributes.node.source.sections[0].content.description == "Awesome description\n\n+ With list\n");
    REQUIRE(attributes.node.source.sections[1].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(attributes.node.source.sections[1].content.elements().size() == 1);
}
