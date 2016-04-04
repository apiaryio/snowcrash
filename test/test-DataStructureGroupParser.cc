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

    ParseResult<DataStructureGroup> dataStructureGroup;
    SectionParserHelper<DataStructureGroup, DataStructureGroupParser>::parse(source, DataStructureGroupSectionType, dataStructureGroup, ExportSourcemapOption);

    REQUIRE(dataStructureGroup.report.error.code == Error::OK);
    REQUIRE(dataStructureGroup.report.warnings.empty());

    REQUIRE(dataStructureGroup.node.element == Element::CategoryElement);
    REQUIRE(dataStructureGroup.node.content.elements().size() == 2);
    REQUIRE(dataStructureGroup.node.content.elements().at(0).element == Element::DataStructureElement);
    REQUIRE(dataStructureGroup.node.content.elements().at(1).element == Element::DataStructureElement);

    DataStructure dataStructure = dataStructureGroup.node.content.elements().at(0).content.dataStructure;
    REQUIRE(dataStructure.name.symbol.literal == "User");
    REQUIRE(dataStructure.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dataStructure.typeDefinition.empty());
    REQUIRE(dataStructure.sections.size() == 1);
    REQUIRE(dataStructure.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(dataStructure.sections[0].content.elements().size() == 2);

    dataStructure = dataStructureGroup.node.content.elements().at(1).content.dataStructure;
    REQUIRE(dataStructure.name.symbol.literal == "Email");
    REQUIRE(dataStructure.typeDefinition.typeSpecification.name.base == mson::ArrayTypeName);
    REQUIRE(dataStructure.typeDefinition.typeSpecification.nestedTypes.size() == 1);

    SourceMap<DataStructure> dataStructureSM = dataStructureGroup.sourceMap.content.elements().collection[0].content.dataStructure;
    SourceMapHelper::check(dataStructureSM.name.sourceMap, 19, 9);
    REQUIRE(dataStructureSM.typeDefinition.sourceMap.empty());
    REQUIRE(dataStructureSM.sections.collection[0].elements().collection.size() == 2);

    dataStructureSM = dataStructureGroup.sourceMap.content.elements().collection[1].content.dataStructure;
    SourceMapHelper::check(dataStructureSM.name.sourceMap, 54, 24);
    SourceMapHelper::check(dataStructureSM.typeDefinition.sourceMap, 54, 24);
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

    ParseResult<DataStructureGroup> dataStructureGroup;
    SectionParserHelper<DataStructureGroup, DataStructureGroupParser>::parse(source, DataStructureGroupSectionType, dataStructureGroup, ExportSourcemapOption);

    REQUIRE(dataStructureGroup.report.error.code == Error::OK);
    REQUIRE(dataStructureGroup.report.warnings.empty());

    REQUIRE(dataStructureGroup.node.element == Element::CategoryElement);
    REQUIRE(dataStructureGroup.node.content.elements().size() == 2);
    REQUIRE(dataStructureGroup.node.content.elements().at(0).element == Element::DataStructureElement);
    REQUIRE(dataStructureGroup.node.content.elements().at(1).element == Element::DataStructureElement);

    DataStructure dataStructure = dataStructureGroup.node.content.elements().at(0).content.dataStructure;
    REQUIRE(dataStructure.name.symbol.literal == "User");
    REQUIRE(dataStructure.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dataStructure.typeDefinition.empty());
    REQUIRE(dataStructure.sections.size() == 2);
    REQUIRE(dataStructure.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(dataStructure.sections[0].content.description == "Some description\n\n");
    REQUIRE(dataStructure.sections[1].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(dataStructure.sections[1].content.elements().size() == 2);

    dataStructure = dataStructureGroup.node.content.elements().at(1).content.dataStructure;
    REQUIRE(dataStructure.name.symbol.literal == "Email");
    REQUIRE(dataStructure.typeDefinition.typeSpecification.name.base == mson::ArrayTypeName);
    REQUIRE(dataStructure.typeDefinition.typeSpecification.nestedTypes.size() == 1);

    SourceMap<DataStructure> dataStructureSM = dataStructureGroup.sourceMap.content.elements().collection[0].content.dataStructure;
    SourceMapHelper::check(dataStructureSM.name.sourceMap, 19, 9);
    REQUIRE(dataStructureSM.typeDefinition.sourceMap.empty());
    SourceMapHelper::check(dataStructureSM.sections.collection[0].description.sourceMap, 28, 18);
    REQUIRE(dataStructureSM.sections.collection[1].elements().collection.size() == 2);

    dataStructureSM = dataStructureGroup.sourceMap.content.elements().collection[1].content.dataStructure;
    SourceMapHelper::check(dataStructureSM.name.sourceMap, 88, 24);
    SourceMapHelper::check(dataStructureSM.typeDefinition.sourceMap, 88, 24);
}
