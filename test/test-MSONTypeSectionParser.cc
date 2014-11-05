//
//  test-MSONTypeSectionParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/4/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONTypeSectionParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Type Section header block classifier", "[type_section]")
{
    mdp::ByteBuffer source = \
    "## Items";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(markdownAST.children().front().type == mdp::HeaderMarkdownNodeType);

    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONTypeSectionSectionType);

    markdownAST.children().front().text = "Members";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONTypeSectionSectionType);

    markdownAST.children().front().text = "Properties";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONTypeSectionSectionType);

    markdownAST.children().front().text = "Default";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONTypeSectionSectionType);

    markdownAST.children().front().text = "Sample";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONTypeSectionSectionType);
}

TEST_CASE("Type Section list block classifier", "[type_section]")
{
    mdp::ByteBuffer source = \
    "- Items";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(markdownAST.children().front().type == mdp::ListItemMarkdownNodeType);
    REQUIRE(!markdownAST.children().front().children().empty());

    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONTypeSectionSectionType);

    markdownAST.children().front().children().front().text = "Members";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONTypeSectionSectionType);

    markdownAST.children().front().children().front().text = "Properties";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONTypeSectionSectionType);

    markdownAST.children().front().text = "Default";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONTypeSectionSectionType);

    markdownAST.children().front().children().front().text = "Default : 400";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONTypeSectionSectionType);

    markdownAST.children().front().children().front().text = "Sample: red, green";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONTypeSectionSectionType);
}

TEST_CASE("Parse default type section", "[type_section]")
{
    mdp::ByteBuffer source = \
    "- Default: yellow";

    ParseResult<mson::TypeSection> typeSection;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONTypeSectionSectionType, typeSection);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.type == mson::DefaultTypeSectionType);
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.members().size() == 1);
    MSONHelper::empty(typeSection.node.content.members()[0].content.property);
    MSONHelper::empty(typeSection.node.content.members()[0].content.mixin);
    MSONHelper::empty(typeSection.node.content.members()[0].content.oneOf);
    REQUIRE(typeSection.node.content.members()[0].type == mson::ValueMemberType);
    REQUIRE(typeSection.node.content.members()[0].content.value.valueDefinition.values.size() == 1);
    REQUIRE(typeSection.node.content.members()[0].content.value.valueDefinition.values[0].literal == "yellow");
    REQUIRE(typeSection.node.content.members()[0].content.value.valueDefinition.values[0].variable == false);
    MSONHelper::empty(typeSection.node.content.members()[0].content.value.valueDefinition.typedefinition);
    REQUIRE(typeSection.node.content.members()[0].content.value.description.empty());
    REQUIRE(typeSection.node.content.members()[0].content.value.sections.empty());
}
