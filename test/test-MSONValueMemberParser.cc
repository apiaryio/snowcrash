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

TEST_CASE("Parse canonical mson value member", "[mson][value_member]")
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
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.typeSpecification.name.name == mson::StringTypeName);
    MSONHelper::empty(valueMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol);
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.typeSpecification.nestedTypes.empty());
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.attributes == mson::RequiredTypeAttribute);
}

TEST_CASE("Parse mson value member with description not on new line", "[mson][value_member]")
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

TEST_CASE("Parse mson value member with block description", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- red (string, required) - A color\n\n"\
    "  Which is also very nice\n\n"\
    "  - and awesome\n\n"\
    "and really really nice\n\n";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description == "A color");
    REQUIRE(valueMember.node.valueDefinition.values.size() == 1);
    REQUIRE(valueMember.node.sections.size() == 1);
    REQUIRE(valueMember.node.sections[0].type == mson::BlockDescriptionTypeSectionType);
    REQUIRE(valueMember.node.sections[0].content.description == "Which is also very nice\n\n- and awesome\n");
    REQUIRE(valueMember.node.sections[0].content.members().empty());
}

TEST_CASE("Parse mson value member with block description, default and sample", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- red (string, required) - A color\n\n"\
    "  Which is also very nice\n\n"\
    "  - and awesome\n\n"\
    "  - Default: yellow\n"\
    "  - Sample\n\n"\
    "    green\n";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description == "A color");
    REQUIRE(valueMember.node.valueDefinition.values.size() == 1);
    REQUIRE(valueMember.node.sections.size() == 3);
    REQUIRE(valueMember.node.sections[0].type == mson::BlockDescriptionTypeSectionType);
    REQUIRE(valueMember.node.sections[0].content.description == "Which is also very nice\n\n- and awesome\n");
    REQUIRE(valueMember.node.sections[0].content.members().empty());
    REQUIRE(valueMember.node.sections[1].type == mson::DefaultTypeSectionType);
    REQUIRE(valueMember.node.sections[1].content.description.empty());
    REQUIRE(valueMember.node.sections[1].content.members().size() == 1);
    REQUIRE(valueMember.node.sections[2].type == mson::SampleTypeSectionType);
    REQUIRE(valueMember.node.sections[2].content.description.empty());
    REQUIRE(valueMember.node.sections[2].content.members().size() == 1);
}

TEST_CASE("Parse mson value member with multiple values", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- 1, yellow, true";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description.empty());
    REQUIRE(valueMember.node.sections.empty());
    REQUIRE(valueMember.node.valueDefinition.values.size() == 3);
}
