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

    REQUIRE(dataStructures.node.types.size() == 2);
    REQUIRE(dataStructures.node.types[0].resolved.empty());
    REQUIRE(dataStructures.node.types[0].source.name.symbol.literal == "User");
    REQUIRE(dataStructures.node.types[0].source.base.baseType == mson::ImplicitPropertyBaseType);
    REQUIRE(dataStructures.node.types[0].source.base.attributes == 0);
    REQUIRE(dataStructures.node.types[0].source.base.typeSpecification.name.empty());
    REQUIRE(dataStructures.node.types[0].source.base.typeSpecification.nestedTypes.empty());
    REQUIRE(dataStructures.node.types[0].source.sections.size() == 1);
    REQUIRE(dataStructures.node.types[0].source.sections[0].type == mson::MemberTypeSectionType);
    REQUIRE(dataStructures.node.types[0].source.sections[0].content.members().size() == 2);
    REQUIRE(dataStructures.node.types[1].resolved.empty());
    REQUIRE(dataStructures.node.types[1].source.name.symbol.literal == "Email");
    REQUIRE(dataStructures.node.types[1].source.base.typeSpecification.name.name == mson::ArrayTypeName);
    REQUIRE(dataStructures.node.types[1].source.base.typeSpecification.nestedTypes.size() == 1);
}
