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
    REQUIRE(propertyMember.node.name.variable.empty());
    REQUIRE(propertyMember.node.description == "A color");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.valueDefinition.values[0].literal == "red");
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.name == mson::StringTypeName);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol.empty());
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
    REQUIRE(propertyMember.node.name.variable.empty());
    REQUIRE(propertyMember.node.description == "A color");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::BlockDescriptionType);
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
    REQUIRE(propertyMember.node.name.variable.empty());
    REQUIRE(propertyMember.node.description == "A color");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.attributes == mson::RequiredTypeAttribute);
    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::BlockDescriptionType);
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
    REQUIRE(propertyMember.node.name.variable.empty());
    REQUIRE(propertyMember.node.description == "A color");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.valueDefinition.values[0].literal == "red");
    REQUIRE(propertyMember.node.sections.size() == 3);
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::BlockDescriptionType);
    REQUIRE(propertyMember.node.sections[0].content.description == "Which is also very nice\n\n- and awesome\n");
    REQUIRE(propertyMember.node.sections[1].type == mson::TypeSection::DefaultType);
    REQUIRE(propertyMember.node.sections[1].content.value == "yellow");
    REQUIRE(propertyMember.node.sections[2].type == mson::TypeSection::SampleType);
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
    REQUIRE(propertyMember.node.name.variable.empty());
    REQUIRE(propertyMember.node.description.empty());
    REQUIRE(propertyMember.node.valueDefinition.values.empty());
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.name == mson::ObjectTypeName);
    REQUIRE(propertyMember.node.sections.size() == 1);

    REQUIRE(propertyMember.node.sections[0].content.description.empty());
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::MemberType);
    REQUIRE(propertyMember.node.sections[0].content.members().size() == 2);
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).type == mson::MemberType::PropertyType);
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.name.literal == "first_name");
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.name.variable.empty());
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.valueDefinition.values[0].literal == "Pavan");
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.description == "A sample value");
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).type == mson::MemberType::PropertyType);
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).content.property.name.literal == "last_name");
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).content.property.name.variable.empty());
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).content.property.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).content.property.valueDefinition.values[0].literal == "Sunkara");
}

TEST_CASE("Parse mson array property member with nested properties type section", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- user (array)\n\n"\
    "    List of users\n"\
    "    - Properties\n"\
    "        - first_name\n"\
    "        - last_name";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.size() == 1);
    REQUIRE(propertyMember.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::BlockDescriptionType);
}

TEST_CASE("Parse mson property member when it has a member group in nested members", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- user (object)\n"\
    "    - username (string)\n"\
    "    - Properties\n"\
    "        - last_name\n"\
    "    - first_name (string)";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.sections.size() == 2);
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::MemberType);
    REQUIRE(propertyMember.node.sections[0].content.members().size() == 1);
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.name.literal == "username");
    REQUIRE(propertyMember.node.sections[1].type == mson::TypeSection::MemberType);
    REQUIRE(propertyMember.node.sections[1].content.members().size() == 2);
    REQUIRE(propertyMember.node.sections[1].content.members().at(0).content.property.name.literal == "last_name");
    REQUIRE(propertyMember.node.sections[1].content.members().at(1).content.property.name.literal == "first_name");
}

TEST_CASE("Parse mson property member when it has multiple member groups", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- user (object)\n\n"\
    "    This is good\n"\
    "    - really\n\n"\
    "    I am serious\n"\
    "    - Properties\n"\
    "        - last_name\n"\
    "    -  Properties\n"\
    "        - first_name";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.sections.size() == 3);
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::BlockDescriptionType);
    REQUIRE(propertyMember.node.sections[0].content.description == "This is good\n\n- really\n\nI am serious\n");
    REQUIRE(propertyMember.node.sections[1].type == mson::TypeSection::MemberType);
    REQUIRE(propertyMember.node.sections[1].content.members().size() == 1);
    REQUIRE(propertyMember.node.sections[1].content.members().at(0).content.property.name.literal == "last_name");
    REQUIRE(propertyMember.node.sections[2].type == mson::TypeSection::MemberType);
    REQUIRE(propertyMember.node.sections[2].content.members().size() == 1);
    REQUIRE(propertyMember.node.sections[2].content.members().at(0).content.property.name.literal == "first_name");
}

TEST_CASE("Parse mson property member when it has the wrong member group", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- user (array)\n\n"\
    "    This is good\n"\
    "    - Properties\n"\
    "        - last_name";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.size() == 1);
    REQUIRE(propertyMember.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::BlockDescriptionType);
    REQUIRE(propertyMember.node.sections[0].content.description == "This is good\n");
}

TEST_CASE("Parse mson property member when it is an object and has no sub-type specified and with member group", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- user\n\n"\
    "    This is good\n"\
    "    - Properties\n"\
    "        - last_name: sunkara (string)";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "user");
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.name == mson::UndefinedTypeName);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol.empty());

    REQUIRE(propertyMember.node.sections.size() == 2);
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::BlockDescriptionType);
    REQUIRE(propertyMember.node.sections[0].content.description == "This is good\n");
    REQUIRE(propertyMember.node.sections[1].type == mson::TypeSection::MemberType);
    REQUIRE(propertyMember.node.sections[1].content.members().size() == 1);
    REQUIRE(propertyMember.node.sections[1].content.members().at(0).type == mson::MemberType::PropertyType);
    REQUIRE(propertyMember.node.sections[1].content.members().at(0).content.property.name.literal == "last_name");
    REQUIRE(propertyMember.node.sections[1].content.members().at(0).content.property.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections[1].content.members().at(0).content.property.valueDefinition.values[0].literal == "sunkara");
    REQUIRE(propertyMember.node.sections[1].content.members().at(0).content.property.sections.empty());
}

TEST_CASE("Parse mson property member when it is an object and has no sub-type specified", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- user\n\n"\
    "    - last_name: sunkara (string)\n\n"\
    "    Some random para node\n"\
    "    - first_name: pavan (string)";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.size() == 1);
    REQUIRE(propertyMember.report.warnings[0].code == IgnoringWarning);

    REQUIRE(propertyMember.node.name.literal == "user");
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.name == mson::UndefinedTypeName);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol.empty());

    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::MemberType);
    REQUIRE(propertyMember.node.sections[0].baseType == mson::ImplicitObjectBaseType);
    REQUIRE(propertyMember.node.sections[0].content.members().size() == 2);

    REQUIRE(propertyMember.node.sections[0].content.members().at(0).type == mson::MemberType::PropertyType);
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.name.literal == "last_name");
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.valueDefinition.values[0].literal == "sunkara");
    REQUIRE(propertyMember.node.sections[0].content.members().at(0).content.property.sections.empty());

    REQUIRE(propertyMember.node.sections[0].content.members().at(1).type == mson::MemberType::PropertyType);
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).content.property.name.literal == "first_name");
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).content.property.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).content.property.valueDefinition.values[0].literal == "pavan");
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).content.property.sections.empty());
}

TEST_CASE("Parse mson property member when it is a string and has no sub-type specified", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- username\n\n"\
    "    Some block description\n\n"\
    "    - Sample: Pavan, Sunkara";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "username");
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.baseType == mson::ImplicitPrimitiveBaseType);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.name == mson::UndefinedTypeName);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol.empty());

    REQUIRE(propertyMember.node.sections.size() == 2);
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::BlockDescriptionType);
    REQUIRE(propertyMember.node.sections[0].content.description == "Some block description\n");
    REQUIRE(propertyMember.node.sections[1].type == mson::TypeSection::SampleType);
    REQUIRE(propertyMember.node.sections[1].baseType == mson::ImplicitPrimitiveBaseType);
    REQUIRE(propertyMember.node.sections[1].content.value == "Pavan, Sunkara");
}

TEST_CASE("Parse mson property member when no sub-type specified and no nested sections", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- username\n\n"\
    "    Some block description";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "username");
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.baseType == mson::ImplicitPrimitiveBaseType);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.name == mson::UndefinedTypeName);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol.empty());

    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::BlockDescriptionType);
    REQUIRE(propertyMember.node.sections[0].content.description == "Some block description");
}

TEST_CASE("Parse mson property member when containing a mixin", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- formal_person (object)\n"\
    "  - prefix: Mr\n"\
    "  - Include Person";

    NamedTypes namedTypes;
    NamedTypeHelper::build("Person", mson::ObjectBaseType, namedTypes);

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption, Models(), NULL, namedTypes);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "formal_person");
    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::MemberType);
    REQUIRE(propertyMember.node.sections[0].content.members().size() == 2);

    REQUIRE(propertyMember.node.sections[0].content.members().at(0).type == mson::MemberType::PropertyType);
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).type == mson::MemberType::MixinType);
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).content.mixin.typeDefinition.typeSpecification.name.symbol.literal == "Person");
}

TEST_CASE("Parse mson property member when containing an oneOf", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- formal_person (object)\n"\
    "    - first_name\n"\
    "    - One Of\n"\
    "        - last_name\n"\
    "        - given_name: Smith";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "formal_person");
    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].type == mson::TypeSection::MemberType);
    REQUIRE(propertyMember.node.sections[0].content.members().size() == 2);

    REQUIRE(propertyMember.node.sections[0].content.members().at(0).type == mson::MemberType::PropertyType);
    REQUIRE(propertyMember.node.sections[0].content.members().at(1).type == mson::MemberType::OneOfType);

    mson::OneOf oneOf = propertyMember.node.sections[0].content.members().at(1).content.oneOf;
    REQUIRE(oneOf.members().size() == 2);
    REQUIRE(oneOf.members().at(0).type == mson::MemberType::PropertyType);
    REQUIRE(oneOf.members().at(0).content.property.name.literal == "last_name");
    REQUIRE(oneOf.members().at(1).type == mson::MemberType::PropertyType);
    REQUIRE(oneOf.members().at(1).content.property.name.literal == "given_name");
}