//
//  test-DataStructuresParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 02/12/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "DataStructuresParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer DataStructuresFixture = "# Data structure";

TEST_CASE("Recognize explicit data structures signature", "[data_structures]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(DataStructuresFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<DataStructures>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == DataStructuresSectionType);
}

TEST_CASE("Parse canonical data structures", "[data_structures]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n\n"\
    "## User\n\n"\
    "- first_name\n"\
    "- last_name\n\n"\
    "## Email (array[string])";

    ParseResult<DataStructures> dataStructures;
    SectionParserHelper<DataStructures, DataStructuresParser>::parse(source, DataStructuresSectionType, dataStructures);

    REQUIRE(dataStructures.report.error.code == Error::OK);
    REQUIRE(dataStructures.report.warnings.empty());

    REQUIRE(dataStructures.node.size() == 2);
    REQUIRE(dataStructures.node[0].resolved.empty());
    REQUIRE(dataStructures.node[0].source.name.symbol.literal == "User");
    REQUIRE(dataStructures.node[0].source.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dataStructures.node[0].source.typeDefinition.attributes == 0);
    REQUIRE(dataStructures.node[0].source.typeDefinition.typeSpecification.name.empty());
    REQUIRE(dataStructures.node[0].source.typeDefinition.typeSpecification.nestedTypes.empty());
    REQUIRE(dataStructures.node[0].source.sections.size() == 1);
    REQUIRE(dataStructures.node[0].source.sections[0].type == mson::TypeSection::MemberType);
    REQUIRE(dataStructures.node[0].source.sections[0].content.members().size() == 2);
    REQUIRE(dataStructures.node[1].resolved.empty());
    REQUIRE(dataStructures.node[1].source.name.symbol.literal == "Email");
    REQUIRE(dataStructures.node[1].source.typeDefinition.typeSpecification.name.base == mson::ArrayTypeName);
    REQUIRE(dataStructures.node[1].source.typeDefinition.typeSpecification.nestedTypes.size() == 1);
}

TEST_CASE("Parse multiple data structures with type sections", "[data_structures]")
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

    ParseResult<DataStructures> dataStructures;
    SectionParserHelper<DataStructures, DataStructuresParser>::parse(source, DataStructuresSectionType, dataStructures);

    REQUIRE(dataStructures.report.error.code == Error::OK);
    REQUIRE(dataStructures.report.warnings.empty());

    REQUIRE(dataStructures.node.size() == 2);
    REQUIRE(dataStructures.node[0].resolved.empty());
    REQUIRE(dataStructures.node[0].source.name.symbol.literal == "User");
    REQUIRE(dataStructures.node[0].source.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dataStructures.node[0].source.typeDefinition.attributes == 0);
    REQUIRE(dataStructures.node[0].source.typeDefinition.typeSpecification.name.empty());
    REQUIRE(dataStructures.node[0].source.typeDefinition.typeSpecification.nestedTypes.empty());
    REQUIRE(dataStructures.node[0].source.sections.size() == 2);
    REQUIRE(dataStructures.node[0].source.sections[0].type == mson::TypeSection::BlockDescriptionType);
    REQUIRE(dataStructures.node[0].source.sections[0].content.description == "Some description\n\n");
    REQUIRE(dataStructures.node[0].source.sections[1].type == mson::TypeSection::MemberType);
    REQUIRE(dataStructures.node[0].source.sections[1].content.members().size() == 2);
    REQUIRE(dataStructures.node[1].resolved.empty());
    REQUIRE(dataStructures.node[1].source.name.symbol.literal == "Email");
    REQUIRE(dataStructures.node[1].source.typeDefinition.typeSpecification.name.base == mson::ArrayTypeName);
    REQUIRE(dataStructures.node[1].source.typeDefinition.typeSpecification.nestedTypes.size() == 1);
}
