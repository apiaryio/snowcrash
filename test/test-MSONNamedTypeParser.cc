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

    REQUIRE(namedType.report.error.code == Error::OK);
    REQUIRE(namedType.report.warnings.empty());

    REQUIRE(namedType.node.name.symbol.literal == "User");
    REQUIRE(namedType.node.name.name == mson::UndefinedTypeName);
    REQUIRE(namedType.node.base.attributes == 0);
    REQUIRE(namedType.node.base.typeSpecification.name.name == mson::ObjectTypeName);
    MSONHelper::empty(namedType.node.base.typeSpecification.name.symbol);
    REQUIRE(namedType.node.base.baseType == mson::PropertyBaseType);
    REQUIRE(namedType.node.sections.size() == 1);
    REQUIRE(namedType.node.sections[0].type == mson::MemberTypeSectionType);
    REQUIRE(namedType.node.sections[0].content.members().size() == 5);
    REQUIRE(namedType.node.sections[0].content.members().at(0).type == mson::PropertyMemberType);
    REQUIRE(namedType.node.sections[0].content.members().at(0).content.property.name.literal == "login");
    REQUIRE(namedType.node.sections[0].content.members().at(0).content.property.valueDefinition.values.size() == 1);
    REQUIRE(namedType.node.sections[0].content.members().at(0).content.property.valueDefinition.values[0].literal == "pksunkara");
    REQUIRE(namedType.node.sections[0].content.members().at(0).content.property.valueDefinition.typeDefinition.baseType == mson::PrimitiveBaseType);
    REQUIRE(namedType.node.sections[0].content.members().at(0).content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::StringTypeName);
    REQUIRE(namedType.node.sections[0].content.members().at(0).content.property.sections.empty());
    REQUIRE(namedType.node.sections[0].content.members().at(1).type == mson::PropertyMemberType);
    REQUIRE(namedType.node.sections[0].content.members().at(1).content.property.name.literal == "name");
    REQUIRE(namedType.node.sections[0].content.members().at(1).content.property.valueDefinition.values.size() == 1);
    REQUIRE(namedType.node.sections[0].content.members().at(1).content.property.valueDefinition.values[0].literal == "Pavan Kumar Sunkara");
    REQUIRE(namedType.node.sections[0].content.members().at(1).content.property.valueDefinition.typeDefinition.baseType == mson::PrimitiveBaseType);
    REQUIRE(namedType.node.sections[0].content.members().at(1).content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::StringTypeName);
    REQUIRE(namedType.node.sections[0].content.members().at(1).content.property.sections.empty());
    REQUIRE(namedType.node.sections[0].content.members().at(2).type == mson::PropertyMemberType);
    REQUIRE(namedType.node.sections[0].content.members().at(2).content.property.name.literal == "admin");
    REQUIRE(namedType.node.sections[0].content.members().at(2).content.property.valueDefinition.values.size() == 1);
    REQUIRE(namedType.node.sections[0].content.members().at(2).content.property.valueDefinition.values[0].literal == "false");
    REQUIRE(namedType.node.sections[0].content.members().at(2).content.property.valueDefinition.typeDefinition.baseType == mson::PrimitiveBaseType);
    REQUIRE(namedType.node.sections[0].content.members().at(2).content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::BooleanTypeName);
    REQUIRE(namedType.node.sections[0].content.members().at(2).content.property.sections.empty());
    REQUIRE(namedType.node.sections[0].content.members().at(3).type == mson::PropertyMemberType);
    REQUIRE(namedType.node.sections[0].content.members().at(3).content.property.name.literal == "orgs");
    REQUIRE(namedType.node.sections[0].content.members().at(3).content.property.valueDefinition.values.size() == 2);
    REQUIRE(namedType.node.sections[0].content.members().at(3).content.property.valueDefinition.values[0].literal == "confy");
    REQUIRE(namedType.node.sections[0].content.members().at(3).content.property.valueDefinition.values[1].literal == "apiary");
    REQUIRE(namedType.node.sections[0].content.members().at(3).content.property.valueDefinition.typeDefinition.baseType == mson::ValueBaseType);
    REQUIRE(namedType.node.sections[0].content.members().at(3).content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::ArrayTypeName);
    REQUIRE(namedType.node.sections[0].content.members().at(3).content.property.sections.empty());
    REQUIRE(namedType.node.sections[0].content.members().at(4).type == mson::PropertyMemberType);
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.name.literal == "plan");
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.valueDefinition.values.empty());
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.valueDefinition.typeDefinition.baseType == mson::PropertyBaseType);
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::ObjectTypeName);
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections.size() == 2);
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections[0].type == mson::BlockDescriptionTypeSectionType);
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections[0].content.description == "The subscription plan of the user\n");
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections[1].type == mson::MemberTypeSectionType);
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections[1].content.members().at(0).type == mson::PropertyMemberType);
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections[1].content.members().at(0).content.property.name.literal == "stripe");
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections[1].content.members().at(0).content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::NumberTypeName);
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections[1].content.members().at(0).content.property.valueDefinition.values.size() == 1);
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections[1].content.members().at(0).content.property.valueDefinition.values[0].literal == "1284");
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections[1].content.members().at(1).type == mson::PropertyMemberType);
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections[1].content.members().at(1).content.property.name.literal == "name");
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections[1].content.members().at(1).content.property.valueDefinition.typeDefinition.typeSpecification.name.name == mson::StringTypeName);
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections[1].content.members().at(1).content.property.valueDefinition.values.size() == 1);
    REQUIRE(namedType.node.sections[0].content.members().at(4).content.property.sections[1].content.members().at(1).content.property.valueDefinition.values[0].literal == "Medium");
}