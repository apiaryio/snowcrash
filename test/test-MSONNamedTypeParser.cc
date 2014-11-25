//
//  test-MSONNamedTypeParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONNamedTypeParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Parse canonical mson named type", "[mson][named_type]")
{
    mdp::ByteBuffer source = \
    "# User (object)\n"\
    "- login: pksunkara (string)\n"\
    "- name: Pavan Kumar Sunkara (string, optional)\n"\
    "- admin: false (boolean, default)\n"\
    "- orgs: confy, apiary (array)\n"\
    "- plan (object)\n"\
    "    The subscription plan of the user\n"\
    "    - Properties\n"\
    "        - stripe: 1284 (number)\n"\
    "        - name: Medium (string)\n";

    ParseResult<mson::NamedType> namedType;
    SectionParserHelper<mson::NamedType, MSONNamedTypeParser>::parse(source, MSONNamedTypeSectionType, namedType);

    mson::MemberType member, submember;

    REQUIRE(namedType.report.error.code == Error::OK);
    REQUIRE(namedType.report.warnings.empty());

    REQUIRE(namedType.node.name.symbol.literal == "User");
    REQUIRE(namedType.node.name.name == mson::UndefinedTypeName);
    REQUIRE(namedType.node.base.attributes == 0);
    REQUIRE(namedType.node.base.typeSpecification.name.name == mson::ObjectTypeName);
    REQUIRE(namedType.node.base.typeSpecification.name.symbol.empty());
    REQUIRE(namedType.node.base.baseType == mson::ObjectBaseType);
    REQUIRE(namedType.node.sections.size() == 1);
    REQUIRE(namedType.node.sections[0].type == mson::TypeSection::MemberType);
    REQUIRE(namedType.node.sections[0].content.members().size() == 5);

    member = namedType.node.sections[0].content.members().at(0);
    REQUIRE(member.type == mson::MemberType::PropertyType);
    REQUIRE(member.content.property.name.literal == "login");
    REQUIRE(member.content.property.valueDefinition.values.size() == 1);
    REQUIRE(member.content.property.valueDefinition.values[0].literal == "pksunkara");
    REQUIRE(member.content.property.valueDefinition.typeDefinition.baseType == mson::PrimitiveBaseType);
    REQUIRE(member.content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::StringTypeName);
    REQUIRE(member.content.property.sections.empty());

    member = namedType.node.sections[0].content.members().at(1);
    REQUIRE(member.type == mson::MemberType::PropertyType);
    REQUIRE(member.content.property.name.literal == "name");
    REQUIRE(member.content.property.valueDefinition.values.size() == 1);
    REQUIRE(member.content.property.valueDefinition.values[0].literal == "Pavan Kumar Sunkara");
    REQUIRE(member.content.property.valueDefinition.typeDefinition.baseType == mson::PrimitiveBaseType);
    REQUIRE(member.content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::StringTypeName);
    REQUIRE(member.content.property.sections.empty());

    member = namedType.node.sections[0].content.members().at(2);
    REQUIRE(member.type == mson::MemberType::PropertyType);
    REQUIRE(member.content.property.name.literal == "admin");
    REQUIRE(member.content.property.valueDefinition.values.size() == 1);
    REQUIRE(member.content.property.valueDefinition.values[0].literal == "false");
    REQUIRE(member.content.property.valueDefinition.typeDefinition.baseType == mson::PrimitiveBaseType);
    REQUIRE(member.content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::BooleanTypeName);
    REQUIRE(member.content.property.sections.empty());

    member = namedType.node.sections[0].content.members().at(3);
    REQUIRE(member.type == mson::MemberType::PropertyType);
    REQUIRE(member.content.property.name.literal == "orgs");
    REQUIRE(member.content.property.valueDefinition.values.size() == 2);
    REQUIRE(member.content.property.valueDefinition.values[0].literal == "confy");
    REQUIRE(member.content.property.valueDefinition.values[1].literal == "apiary");
    REQUIRE(member.content.property.valueDefinition.typeDefinition.baseType == mson::ValueBaseType);
    REQUIRE(member.content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::ArrayTypeName);
    REQUIRE(member.content.property.sections.empty());

    member = namedType.node.sections[0].content.members().at(4);
    REQUIRE(member.type == mson::MemberType::PropertyType);
    REQUIRE(member.content.property.name.literal == "plan");
    REQUIRE(member.content.property.valueDefinition.values.empty());
    REQUIRE(member.content.property.valueDefinition.typeDefinition.baseType == mson::ObjectBaseType);
    REQUIRE(member.content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::ObjectTypeName);
    REQUIRE(member.content.property.sections.size() == 2);
    REQUIRE(member.content.property.sections[0].type == mson::TypeSection::BlockDescriptionType);
    REQUIRE(member.content.property.sections[0].content.description == "The subscription plan of the user\n");
    REQUIRE(member.content.property.sections[1].type == mson::TypeSection::MemberType);
    REQUIRE(member.content.property.sections[1].content.members().size() == 2);

    submember = member.content.property.sections[1].content.members().at(0);
    REQUIRE(submember.type == mson::MemberType::PropertyType);
    REQUIRE(submember.content.property.name.literal == "stripe");
    REQUIRE(submember.content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::NumberTypeName);
    REQUIRE(submember.content.property.valueDefinition.values.size() == 1);
    REQUIRE(submember.content.property.valueDefinition.values[0].literal == "1284");

    submember = member.content.property.sections[1].content.members().at(1);
    REQUIRE(submember.type == mson::MemberType::PropertyType);
    REQUIRE(submember.content.property.name.literal == "name");
    REQUIRE(submember.content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::StringTypeName);
    REQUIRE(submember.content.property.valueDefinition.values.size() == 1);
    REQUIRE(submember.content.property.valueDefinition.values[0].literal == "Medium");
}

TEST_CASE("Parse named type with a type section", "[mson][named_type]")
{
    mdp::ByteBuffer source = \
    "# User (string)\n"\
    "## Sample: pksunkara";

    ParseResult<mson::NamedType> namedType;
    SectionParserHelper<mson::NamedType, MSONNamedTypeParser>::parse(source, MSONNamedTypeSectionType, namedType);

    REQUIRE(namedType.report.error.code == Error::OK);
    REQUIRE(namedType.report.warnings.empty());

    REQUIRE(namedType.node.name.symbol.literal == "User");
    REQUIRE(namedType.node.name.name == mson::UndefinedTypeName);
    REQUIRE(namedType.node.base.attributes == 0);
    REQUIRE(namedType.node.base.typeSpecification.name.name == mson::StringTypeName);
    REQUIRE(namedType.node.base.typeSpecification.name.symbol.empty());
    REQUIRE(namedType.node.base.baseType == mson::PrimitiveBaseType);
    REQUIRE(namedType.node.sections.size() == 1);
    REQUIRE(namedType.node.sections[0].type == mson::TypeSection::SampleType);
    REQUIRE(namedType.node.sections[0].content.value == "pksunkara");
}

TEST_CASE("Parse named type without type specification", "[mson][named_type]")
{
    mdp::ByteBuffer source = \
    "# User\n"\
    "## Sample: pksunkara";

    ParseResult<mson::NamedType> namedType;
    SectionParserHelper<mson::NamedType, MSONNamedTypeParser>::parse(source, MSONNamedTypeSectionType, namedType);

    REQUIRE(namedType.report.error.code == Error::OK);
    REQUIRE(namedType.report.warnings.size() == 1);
    REQUIRE(namedType.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(namedType.node.name.symbol.literal == "User");
    REQUIRE(namedType.node.base.attributes == 0);
    REQUIRE(namedType.node.base.typeSpecification.name.empty());
    REQUIRE(namedType.node.base.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(namedType.node.sections.size() == 1);
    REQUIRE(namedType.node.sections[0].type == mson::TypeSection::SampleType);
    REQUIRE(namedType.node.sections[0].baseType == mson::ImplicitObjectBaseType);
}
