//
//  test-MSONPropertyMemberParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONPropertyMemberParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Parse canonical mson property member", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- color: red (string, required) - A color";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "color");
    MSONHelper::empty(propertyMember.node.name.variable);
    REQUIRE(propertyMember.node.description == "A color");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.valueDefinition.values[0].literal == "red");
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.name == mson::StringTypeName);
    MSONHelper::empty(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.nestedTypes.empty());
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.attributes == mson::RequiredTypeAttribute);
}

TEST_CASE("Parse mson property member with description not on new line", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- color: red (string, required) - A color\n"\
    "  Which is also very nice\n\n";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "color");
    MSONHelper::empty(propertyMember.node.name.variable);
    REQUIRE(propertyMember.node.description == "A color");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].type == mson::BlockDescriptionTypeSectionType);
    REQUIRE(propertyMember.node.sections[0].content.description == "Which is also very nice\n");
    REQUIRE(propertyMember.node.sections[0].content.members().empty());
}

TEST_CASE("Parse mson property member with block description", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- color: red (string, required) - A color\n\n"\
    "    Which is also very nice\n\n"\
    "    - and awesome\n\n"\
    "and really really nice\n\n";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "color");
    MSONHelper::empty(propertyMember.node.name.variable);
    REQUIRE(propertyMember.node.description == "A color");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.attributes == mson::RequiredTypeAttribute);
    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].type == mson::BlockDescriptionTypeSectionType);
    REQUIRE(propertyMember.node.sections[0].content.description == "Which is also very nice\n\n- and awesome\n");
}

TEST_CASE("Parse mson property member with block description, default and sample", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- color: red (string) - A color\n\n"\
    "    Which is also very nice\n\n"\
    "    - and awesome\n\n"\
    "    - Default: yellow\n"\
    "    - Sample\n\n"\
    "        green\n";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "color");
    MSONHelper::empty(propertyMember.node.name.variable);
    REQUIRE(propertyMember.node.description == "A color");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.valueDefinition.values[0].literal == "red");
    REQUIRE(propertyMember.node.sections.size() == 3);
    REQUIRE(propertyMember.node.sections[0].type == mson::BlockDescriptionTypeSectionType);
    REQUIRE(propertyMember.node.sections[0].content.description == "Which is also very nice\n\n- and awesome\n");
    REQUIRE(propertyMember.node.sections[1].type == mson::DefaultTypeSectionType);
    REQUIRE(propertyMember.node.sections[1].content.value == "yellow");
    REQUIRE(propertyMember.node.sections[2].type == mson::SampleTypeSectionType);
    REQUIRE(propertyMember.node.sections[2].content.value == "green\n");
}

TEST_CASE("Parse mson property member object with nested members", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- user (object)\n"\
    "    - first_name: Pavan (string) - A sample value\n"\
    "    - last_name: Sunkara (string)";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "user");
    MSONHelper::empty(propertyMember.node.name.variable);
    REQUIRE(propertyMember.node.description.empty());
    REQUIRE(propertyMember.node.valueDefinition.values.empty());
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.name == mson::ObjectTypeName);
    REQUIRE(propertyMember.node.sections.size() == 1);

    REQUIRE(propertyMember.node.sections[0].content.description.empty());
    REQUIRE(propertyMember.node.sections[0].type == mson::MemberTypeSectionType);
    REQUIRE(propertyMember.node.sections[0].content.members().size() == 2);
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).type == mson::PropertyMemberType);
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.name.literal == "first_name");
    MSONHelper::empty(propertyMember.node.sections[0].content.members().at(0).content.property.name.variable);
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.valueDefinition.values[0].literal == "Pavan");
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.description == "A sample value");
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).type == mson::PropertyMemberType);
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).content.property.name.literal == "last_name");
    MSONHelper::empty(propertyMember.node.sections[0].content.members().at(1).content.property.name.variable);
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).content.property.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).content.property.valueDefinition.values[0].literal == "Sunkara");
}
