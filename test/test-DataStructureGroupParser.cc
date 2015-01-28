//
//  test-DataStructureGroupParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 02/12/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "DataStructureGroupParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer DataStructuresFixture = "# Data structure";

TEST_CASE("Recognize explicit data structures signature", "[data_structure_group]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(DataStructuresFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<DataStructureGroup>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == DataStructureGroupSectionType);
}

TEST_CASE("Parse canonical data structures", "[data_structure_group]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n\n"\
    "## User\n\n"\
    "- first_name\n"\
    "- last_name\n\n"\
    "## Email (array[string])";

    DataStructure dataStructure;
    ParseResult<DataStructureGroup> dataStructureGroup;
    SectionParserHelper<DataStructureGroup, DataStructureGroupParser>::parse(source, DataStructureGroupSectionType, dataStructureGroup);

    REQUIRE(dataStructureGroup.report.error.code == Error::OK);
    REQUIRE(dataStructureGroup.report.warnings.empty());

    REQUIRE(dataStructureGroup.node.element == Element::CategoryElement);
    REQUIRE(dataStructureGroup.node.content.elements().size() == 2);
    REQUIRE(dataStructureGroup.node.content.elements().at(0).element == Element::DataStructureElement);
    REQUIRE(dataStructureGroup.node.content.elements().at(1).element == Element::DataStructureElement);

    dataStructure = dataStructureGroup.node.content.elements().at(0).content.dataStructure;
    REQUIRE(dataStructure.resolved.empty());
    REQUIRE(dataStructure.source.name.symbol.literal == "User");
    REQUIRE(dataStructure.source.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dataStructure.source.typeDefinition.attributes == 0);
    REQUIRE(dataStructure.source.typeDefinition.typeSpecification.name.empty());
    REQUIRE(dataStructure.source.typeDefinition.typeSpecification.nestedTypes.empty());
    REQUIRE(dataStructure.source.sections.size() == 1);
    REQUIRE(dataStructure.source.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(dataStructure.source.sections[0].content.elements().size() == 2);

    dataStructure = dataStructureGroup.node.content.elements().at(1).content.dataStructure;
    REQUIRE(dataStructure.resolved.empty());
    REQUIRE(dataStructure.source.name.symbol.literal == "Email");
    REQUIRE(dataStructure.source.typeDefinition.typeSpecification.name.base == mson::ArrayTypeName);
    REQUIRE(dataStructure.source.typeDefinition.typeSpecification.nestedTypes.size() == 1);
}

TEST_CASE("Parse multiple data structures with type sections", "[data_structure_group]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## User\n"\
    "\n"\
    "Some description\n"\
    "\n"\
    "### Properties\n"\
    "\n"\
    "- first_name\n"\
    "- last_name\n"\
    "\n"\
    "## Email (array[string])";

    DataStructure dataStructure;
    ParseResult<DataStructureGroup> dataStructureGroup;
    SectionParserHelper<DataStructureGroup, DataStructureGroupParser>::parse(source, DataStructureGroupSectionType, dataStructureGroup);

    REQUIRE(dataStructureGroup.report.error.code == Error::OK);
    REQUIRE(dataStructureGroup.report.warnings.empty());

    REQUIRE(dataStructureGroup.node.element == Element::CategoryElement);
    REQUIRE(dataStructureGroup.node.content.elements().size() == 2);
    REQUIRE(dataStructureGroup.node.content.elements().at(0).element == Element::DataStructureElement);
    REQUIRE(dataStructureGroup.node.content.elements().at(1).element == Element::DataStructureElement);

    dataStructure = dataStructureGroup.node.content.elements().at(0).content.dataStructure;
    REQUIRE(dataStructure.resolved.empty());
    REQUIRE(dataStructure.source.name.symbol.literal == "User");
    REQUIRE(dataStructure.source.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dataStructure.source.typeDefinition.attributes == 0);
    REQUIRE(dataStructure.source.typeDefinition.typeSpecification.name.empty());
    REQUIRE(dataStructure.source.typeDefinition.typeSpecification.nestedTypes.empty());
    REQUIRE(dataStructure.source.sections.size() == 2);
    REQUIRE(dataStructure.source.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(dataStructure.source.sections[0].content.description == "Some description\n\n");
    REQUIRE(dataStructure.source.sections[1].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(dataStructure.source.sections[1].content.elements().size() == 2);

    dataStructure = dataStructureGroup.node.content.elements().at(1).content.dataStructure;
    REQUIRE(dataStructure.resolved.empty());
    REQUIRE(dataStructure.source.name.symbol.literal == "Email");
    REQUIRE(dataStructure.source.typeDefinition.typeSpecification.name.base == mson::ArrayTypeName);
    REQUIRE(dataStructure.source.typeDefinition.typeSpecification.nestedTypes.size() == 1);
}
