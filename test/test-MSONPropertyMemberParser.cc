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
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "color");
    REQUIRE(propertyMember.node.name.variable.empty());
    REQUIRE(propertyMember.node.description == "A color");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.valueDefinition.values[0].literal == "red");
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.base == mson::StringTypeName);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol.empty());
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.nestedTypes.empty());
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.attributes == mson::RequiredTypeAttribute);

    SourceMapHelper::check(propertyMember.sourceMap.name.sourceMap, 0, 42);
    SourceMapHelper::check(propertyMember.sourceMap.description.sourceMap, 0, 42);
    SourceMapHelper::check(propertyMember.sourceMap.valueDefinition.sourceMap, 0, 42);
}

TEST_CASE("Parse mson property member with description not on new line", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- color: red (string, required) - A color\n"\
    "  Which is also very nice\n\n";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "color");
    REQUIRE(propertyMember.node.name.variable.empty());
    REQUIRE(propertyMember.node.description == "A color");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(propertyMember.node.sections[0].content.description == "Which is also very nice\n");
    REQUIRE(propertyMember.node.sections[0].content.elements().empty());

    SourceMapHelper::check(propertyMember.sourceMap.name.sourceMap, 0, 69);
    SourceMapHelper::check(propertyMember.sourceMap.description.sourceMap, 0, 69);
    SourceMapHelper::check(propertyMember.sourceMap.valueDefinition.sourceMap, 0, 69);

    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 1);
    REQUIRE(propertyMember.sourceMap.sections.collection[0].elements().collection.empty());
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].description.sourceMap, 0, 69);
}

TEST_CASE("Parse mson property member with block description", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- color: red (string, required) - A color\n\n"\
    "    Which is also very nice\n\n"\
    "    - and awesome\n\n"\
    "and really really nice\n\n";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "color");
    REQUIRE(propertyMember.node.name.variable.empty());
    REQUIRE(propertyMember.node.description == "A color");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.attributes == mson::RequiredTypeAttribute);
    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(propertyMember.node.sections[0].content.description == "Which is also very nice\n\n- and awesome\n");

    SourceMapHelper::check(propertyMember.sourceMap.name.sourceMap, 2, 41);
    SourceMapHelper::check(propertyMember.sourceMap.description.sourceMap, 2, 41);
    SourceMapHelper::check(propertyMember.sourceMap.valueDefinition.sourceMap, 2, 41);

    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 1);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].description.sourceMap, 47, 24, 76, 14);
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
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "color");
    REQUIRE(propertyMember.node.name.variable.empty());
    REQUIRE(propertyMember.node.description == "A color");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.valueDefinition.values[0].literal == "red");
    REQUIRE(propertyMember.node.sections.size() == 3);
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(propertyMember.node.sections[0].content.description == "Which is also very nice\n\n- and awesome\n");
    REQUIRE(propertyMember.node.sections[1].klass == mson::TypeSection::DefaultClass);
    REQUIRE(propertyMember.node.sections[1].content.value == "yellow");
    REQUIRE(propertyMember.node.sections[2].klass == mson::TypeSection::SampleClass);
    REQUIRE(propertyMember.node.sections[2].content.value == "green\n");

    SourceMapHelper::check(propertyMember.sourceMap.name.sourceMap, 2, 31);
    SourceMapHelper::check(propertyMember.sourceMap.description.sourceMap, 2, 31);
    SourceMapHelper::check(propertyMember.sourceMap.valueDefinition.sourceMap, 2, 31);

    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 3);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].description.sourceMap, 37, 24, 66, 14);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[1].value.sourceMap, 87, 16);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[2].value.sourceMap, 125, 6);
}

TEST_CASE("Parse mson property member object with nested members", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- user (object)\n"\
    "    - first_name: Pavan (string) - A sample value\n"\
    "    - last_name: Sunkara (string)";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "user");
    REQUIRE(propertyMember.node.name.variable.empty());
    REQUIRE(propertyMember.node.description.empty());
    REQUIRE(propertyMember.node.valueDefinition.values.empty());
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.base == mson::ObjectTypeName);
    REQUIRE(propertyMember.node.sections.size() == 1);

    REQUIRE(propertyMember.node.sections[0].content.description.empty());
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(propertyMember.node.sections[0].content.elements().size() == 2);
    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).klass == mson::Element::PropertyClass);
    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).content.property.name.literal == "first_name");
    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).content.property.name.variable.empty());
    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).content.property.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).content.property.valueDefinition.values[0].literal == "Pavan");
    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).content.property.description == "A sample value");
    REQUIRE(propertyMember.node.sections[0].content.elements().at(1).klass == mson::Element::PropertyClass);
    REQUIRE(propertyMember.node.sections[0].content.elements().at(1).content.property.name.literal == "last_name");
    REQUIRE(propertyMember.node.sections[0].content.elements().at(1).content.property.name.variable.empty());
    REQUIRE(propertyMember.node.sections[0].content.elements().at(1).content.property.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections[0].content.elements().at(1).content.property.valueDefinition.values[0].literal == "Sunkara");

    SourceMapHelper::check(propertyMember.sourceMap.name.sourceMap, 2, 14);
    SourceMapHelper::check(propertyMember.sourceMap.valueDefinition.sourceMap, 2, 14);
    REQUIRE(propertyMember.sourceMap.description.sourceMap.empty());

    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 1);
    REQUIRE(propertyMember.sourceMap.sections.collection[0].elements().collection.size() == 2);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].elements().collection[0].property.name.sourceMap, 22, 44);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].elements().collection[1].property.name.sourceMap, 70, 30);
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
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.size() == 1);
    REQUIRE(propertyMember.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);

    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 1);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].description.sourceMap, 20, 14);
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
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.sections.size() == 2);
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(propertyMember.node.sections[0].content.elements().size() == 1);
    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).content.property.name.literal == "username");
    REQUIRE(propertyMember.node.sections[1].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(propertyMember.node.sections[1].content.elements().size() == 2);
    REQUIRE(propertyMember.node.sections[1].content.elements().at(0).content.property.name.literal == "last_name");
    REQUIRE(propertyMember.node.sections[1].content.elements().at(1).content.property.name.literal == "first_name");

    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 2);
    REQUIRE(propertyMember.sourceMap.sections.collection[0].elements().collection.size() == 1);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].elements().collection[0].property.name.sourceMap, 22, 18);
    REQUIRE(propertyMember.sourceMap.sections.collection[1].elements().collection.size() == 2);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[1].elements().collection[0].property.name.sourceMap, 67, 10);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[1].elements().collection[1].property.name.sourceMap, 81, 22);
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
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.sections.size() == 3);
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(propertyMember.node.sections[0].content.description == "This is good\n\n- really\n\nI am serious\n");
    REQUIRE(propertyMember.node.sections[1].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(propertyMember.node.sections[1].content.elements().size() == 1);
    REQUIRE(propertyMember.node.sections[1].content.elements().at(0).content.property.name.literal == "last_name");
    REQUIRE(propertyMember.node.sections[2].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(propertyMember.node.sections[2].content.elements().size() == 1);
    REQUIRE(propertyMember.node.sections[2].content.elements().at(0).content.property.name.literal == "first_name");

    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 3);
    REQUIRE(propertyMember.sourceMap.sections.collection[0].description.sourceMap.size() == 3);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].description.sourceMap, 21, 13, 1);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].description.sourceMap, 38, 10, 2);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].description.sourceMap, 52, 13, 3);

    REQUIRE(propertyMember.sourceMap.sections.collection[1].elements().collection.size() == 1);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[1].elements().collection[0].property.name.sourceMap, 92, 10);

    REQUIRE(propertyMember.sourceMap.sections.collection[2].elements().collection.size() == 1);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[2].elements().collection[0].property.name.sourceMap, 128, 13);
}

TEST_CASE("Parse mson property member when it has the wrong member group", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- user (array)\n\n"\
    "    This is good\n"\
    "    - Properties\n"\
    "        - last_name";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.size() == 1);
    REQUIRE(propertyMember.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(propertyMember.node.sections[0].content.description == "This is good\n");

    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 1);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].description.sourceMap, 20, 13);
}

TEST_CASE("Parse mson property member when it is an object and has no sub-type specified and with member group", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- user\n\n"\
    "    This is good\n"\
    "    - Properties\n"\
    "        - last_name: sunkara (string)";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "user");
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.base == mson::UndefinedTypeName);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol.empty());

    REQUIRE(propertyMember.node.sections.size() == 2);
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(propertyMember.node.sections[0].content.description == "This is good\n");
    REQUIRE(propertyMember.node.sections[1].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(propertyMember.node.sections[1].content.elements().size() == 1);
    REQUIRE(propertyMember.node.sections[1].content.elements().at(0).klass == mson::Element::PropertyClass);
    REQUIRE(propertyMember.node.sections[1].content.elements().at(0).content.property.name.literal == "last_name");
    REQUIRE(propertyMember.node.sections[1].content.elements().at(0).content.property.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections[1].content.elements().at(0).content.property.valueDefinition.values[0].literal == "sunkara");
    REQUIRE(propertyMember.node.sections[1].content.elements().at(0).content.property.sections.empty());

    SourceMapHelper::check(propertyMember.sourceMap.name.sourceMap, 2, 6);
    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 2);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].description.sourceMap, 12, 13);

    REQUIRE(propertyMember.sourceMap.sections.collection[1].elements().collection.size() == 1);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[1].elements().collection[0].property.name.sourceMap, 50, 30);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[1].elements().collection[0].property.valueDefinition.sourceMap, 50, 30);
}

TEST_CASE("Parse mson property member when it is an object and has no sub-type specified", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- user\n\n"\
    "    - last_name: sunkara (string)\n\n"\
    "    Some random para node\n"\
    "    - first_name: pavan (string)";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.size() == 1);
    REQUIRE(propertyMember.report.warnings[0].code == IgnoringWarning);

    REQUIRE(propertyMember.node.name.literal == "user");
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.base == mson::UndefinedTypeName);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol.empty());

    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(propertyMember.node.sections[0].baseType == mson::ImplicitObjectBaseType);
    REQUIRE(propertyMember.node.sections[0].content.elements().size() == 2);

    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).klass == mson::Element::PropertyClass);
    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).content.property.name.literal == "last_name");
    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).content.property.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).content.property.valueDefinition.values[0].literal == "sunkara");
    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).content.property.sections.empty());

    REQUIRE(propertyMember.node.sections[0].content.elements().at(1).klass == mson::Element::PropertyClass);
    REQUIRE(propertyMember.node.sections[0].content.elements().at(1).content.property.name.literal == "first_name");
    REQUIRE(propertyMember.node.sections[0].content.elements().at(1).content.property.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.sections[0].content.elements().at(1).content.property.valueDefinition.values[0].literal == "pavan");
    REQUIRE(propertyMember.node.sections[0].content.elements().at(1).content.property.sections.empty());

    SourceMapHelper::check(propertyMember.sourceMap.name.sourceMap, 2, 5);
    REQUIRE(propertyMember.sourceMap.valueDefinition.sourceMap.empty());

    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 1);
    REQUIRE(propertyMember.sourceMap.sections.collection[0].elements().collection.size() == 2);

    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].elements().collection[0].property.name.sourceMap, 14, 28);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].elements().collection[0].property.valueDefinition.sourceMap, 14, 28);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].elements().collection[1].property.name.sourceMap, 73, 29);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].elements().collection[1].property.valueDefinition.sourceMap, 73, 29);
}

TEST_CASE("Parse mson property member when it is a string and has no sub-type specified", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- username\n\n"\
    "    Some block description\n\n"\
    "    - Sample: Pavan, Sunkara";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "username");
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.baseType == mson::ImplicitPrimitiveBaseType);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.base == mson::UndefinedTypeName);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol.empty());

    REQUIRE(propertyMember.node.sections.size() == 2);
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(propertyMember.node.sections[0].content.description == "Some block description\n");
    REQUIRE(propertyMember.node.sections[1].klass == mson::TypeSection::SampleClass);
    REQUIRE(propertyMember.node.sections[1].baseType == mson::ImplicitPrimitiveBaseType);
    REQUIRE(propertyMember.node.sections[1].content.value == "Pavan, Sunkara");

    SourceMapHelper::check(propertyMember.sourceMap.name.sourceMap, 2, 10);
    REQUIRE(propertyMember.sourceMap.valueDefinition.sourceMap.empty());

    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 2);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].description.sourceMap, 16, 23);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[1].value.sourceMap, 44, 25);
}

TEST_CASE("Parse mson property member when no sub-type specified and no nested sections", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- username\n\n"\
    "    Some block description";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "username");
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.baseType == mson::ImplicitPrimitiveBaseType);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.base == mson::UndefinedTypeName);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol.empty());

    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(propertyMember.node.sections[0].content.description == "Some block description");

    SourceMapHelper::check(propertyMember.sourceMap.name.sourceMap, 2, 10);
    REQUIRE(propertyMember.sourceMap.valueDefinition.sourceMap.empty());

    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 1);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].description.sourceMap, 16, 22);
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
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(propertyMember.node.sections[0].content.elements().size() == 2);

    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).klass == mson::Element::PropertyClass);
    REQUIRE(propertyMember.node.sections[0].content.elements().at(1).klass == mson::Element::MixinClass);
    REQUIRE(propertyMember.node.sections[0].content.elements().at(1).content.mixin.typeSpecification.name.symbol.literal == "Person");

    SourceMapHelper::check(propertyMember.sourceMap.name.sourceMap, 2, 23);
    SourceMapHelper::check(propertyMember.sourceMap.valueDefinition.sourceMap, 2, 23);

    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 1);
    REQUIRE(propertyMember.sourceMap.sections.collection[0].elements().collection.size() == 2);

    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].elements().collection[0].property.name.sourceMap, 29, 11);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].elements().collection[1].mixin.sourceMap, 42, 17);
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
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "formal_person");
    REQUIRE(propertyMember.node.sections.size() == 1);
    REQUIRE(propertyMember.node.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(propertyMember.node.sections[0].content.elements().size() == 2);

    REQUIRE(propertyMember.node.sections[0].content.elements().at(0).klass == mson::Element::PropertyClass);
    REQUIRE(propertyMember.node.sections[0].content.elements().at(1).klass == mson::Element::OneOfClass);

    mson::OneOf oneOf = propertyMember.node.sections[0].content.elements().at(1).content.oneOf();
    REQUIRE(oneOf.size() == 2);
    REQUIRE(oneOf.at(0).klass == mson::Element::PropertyClass);
    REQUIRE(oneOf.at(0).content.property.name.literal == "last_name");
    REQUIRE(oneOf.at(1).klass == mson::Element::PropertyClass);
    REQUIRE(oneOf.at(1).content.property.name.literal == "given_name");

    SourceMapHelper::check(propertyMember.sourceMap.name.sourceMap, 2, 23);
    SourceMapHelper::check(propertyMember.sourceMap.valueDefinition.sourceMap, 2, 23);

    REQUIRE(propertyMember.sourceMap.sections.collection.size() == 1);
    REQUIRE(propertyMember.sourceMap.sections.collection[0].elements().collection.size() == 2);
    SourceMapHelper::check(propertyMember.sourceMap.sections.collection[0].elements().collection[0].property.name.sourceMap, 31, 11);

    SourceMap<mson::OneOf> oneOfSM = propertyMember.sourceMap.sections.collection[0].elements().collection[1].oneOf();
    REQUIRE(oneOfSM.collection.size() == 2);
    SourceMapHelper::check(oneOfSM.collection[0].property.name.sourceMap, 65, 10);
    SourceMapHelper::check(oneOfSM.collection[1].property.name.sourceMap, 83, 20);
}

TEST_CASE("Parse mson property member containing a list of values and no type specification", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- list: 1, 2, 3";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "list");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 3);
    REQUIRE(propertyMember.node.valueDefinition.values[0].literal == "1");
    REQUIRE(propertyMember.node.valueDefinition.values[1].literal == "2");
    REQUIRE(propertyMember.node.valueDefinition.values[2].literal == "3");
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.baseType == mson::ImplicitValueBaseType);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.empty());
    REQUIRE(propertyMember.node.sections.empty());

    SourceMapHelper::check(propertyMember.sourceMap.name.sourceMap, 0, 16);
    SourceMapHelper::check(propertyMember.sourceMap.valueDefinition.sourceMap, 0, 16);
}

TEST_CASE("Parse mson property containing list of value with string type specification", "[mson][property_member]")
{
    mdp::ByteBuffer source = \
    "- list: 1, 2, 3 (string)";

    ParseResult<mson::PropertyMember> propertyMember;
    SectionParserHelper<mson::PropertyMember, MSONPropertyMemberParser>::parse(source, MSONPropertyMemberSectionType, propertyMember, ExportSourcemapOption);

    REQUIRE(propertyMember.report.error.code == Error::OK);
    REQUIRE(propertyMember.report.warnings.empty());

    REQUIRE(propertyMember.node.name.literal == "list");
    REQUIRE(propertyMember.node.valueDefinition.values.size() == 1);
    REQUIRE(propertyMember.node.valueDefinition.values[0].literal == "1, 2, 3");
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.baseType == mson::PrimitiveBaseType);
    REQUIRE(propertyMember.node.valueDefinition.typeDefinition.typeSpecification.name.base == mson::StringTypeName);
    REQUIRE(propertyMember.node.sections.empty());

    SourceMapHelper::check(propertyMember.sourceMap.name.sourceMap, 0, 25);
    SourceMapHelper::check(propertyMember.sourceMap.valueDefinition.sourceMap, 0, 25);
}
