//
//  test-MSONValueMemberParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/22/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONValueMemberParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Parse canonical mson value member", "[mson_value_member]")
{
    mdp::ByteBuffer source = \
    "- red (string, required) - A color";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description == "A color");
    REQUIRE(valueMember.node.valueDefinition.values.size() == 1);
    REQUIRE(valueMember.node.valueDefinition.values[0].literal == "red");
    REQUIRE(valueMember.node.valueDefinition.typedefinition.typeSpecification.name.name == mson::StringTypeName);
    REQUIRE(valueMember.node.valueDefinition.typedefinition.attributes == mson::RequiredTypeAttribute);
}

TEST_CASE("Parse mson value member with description not on new line", "[mson_value_member]")
{
    mdp::ByteBuffer source = \
    "- red (string, required) - A color\n"\
    "  Which is also very nice\n\n";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description == "A color");
    REQUIRE(valueMember.node.valueDefinition.values.size() == 1);
    REQUIRE(valueMember.node.sections.size() == 1);
    REQUIRE(valueMember.node.sections[0].type == mson::BlockDescriptionTypeSectionType);
    REQUIRE(valueMember.node.sections[0].content.description == "Which is also very nice\n");
    REQUIRE(valueMember.node.sections[0].content.members().empty());
}
