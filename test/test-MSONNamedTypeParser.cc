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
    SectionParserHelper<mson::NamedType, MSONNamedTypeParser>::parse(source, MSONNamedTypeSectionType, namedType, ExportSourcemapOption);

    mson::Element element, subelement;
    SourceMap<mson::Element> elementSM, subelementSM;

    REQUIRE(namedType.report.error.code == Error::OK);
    REQUIRE(namedType.report.warnings.empty());

    REQUIRE(namedType.node.name.symbol.literal == "User");
    REQUIRE(namedType.node.name.base == mson::UndefinedTypeName);
    REQUIRE(namedType.node.typeDefinition.attributes == 0);
    REQUIRE(namedType.node.typeDefinition.typeSpecification.name.base == mson::ObjectTypeName);
    REQUIRE(namedType.node.typeDefinition.typeSpecification.name.symbol.empty());
    REQUIRE(namedType.node.typeDefinition.baseType == mson::ObjectBaseType);
    REQUIRE(namedType.node.sections.size() == 1);
    REQUIRE(namedType.node.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(namedType.node.sections[0].content.elements().size() == 5);

    element = namedType.node.sections[0].content.elements().at(0);
    REQUIRE(element.klass == mson::Element::PropertyClass);
    REQUIRE(element.content.property.name.literal == "login");
    REQUIRE(element.content.property.valueDefinition.values.size() == 1);
    REQUIRE(element.content.property.valueDefinition.values[0].literal == "pksunkara");
    REQUIRE(element.content.property.valueDefinition.typeDefinition.baseType == mson::PrimitiveBaseType);
    REQUIRE(element.content.property.valueDefinition.typeDefinition.typeSpecification.name.base == mson::StringTypeName);
    REQUIRE(element.content.property.sections.empty());

    element = namedType.node.sections[0].content.elements().at(1);
    REQUIRE(element.klass == mson::Element::PropertyClass);
    REQUIRE(element.content.property.name.literal == "name");
    REQUIRE(element.content.property.valueDefinition.values.size() == 1);
    REQUIRE(element.content.property.valueDefinition.values[0].literal == "Pavan Kumar Sunkara");
    REQUIRE(element.content.property.valueDefinition.typeDefinition.baseType == mson::PrimitiveBaseType);
    REQUIRE(element.content.property.valueDefinition.typeDefinition.typeSpecification.name.base == mson::StringTypeName);
    REQUIRE(element.content.property.sections.empty());

    element = namedType.node.sections[0].content.elements().at(2);
    REQUIRE(element.klass == mson::Element::PropertyClass);
    REQUIRE(element.content.property.name.literal == "admin");
    REQUIRE(element.content.property.valueDefinition.values.size() == 1);
    REQUIRE(element.content.property.valueDefinition.values[0].literal == "false");
    REQUIRE(element.content.property.valueDefinition.typeDefinition.baseType == mson::PrimitiveBaseType);
    REQUIRE(element.content.property.valueDefinition.typeDefinition.typeSpecification.name.base == mson::BooleanTypeName);
    REQUIRE(element.content.property.sections.empty());

    element = namedType.node.sections[0].content.elements().at(3);
    REQUIRE(element.klass == mson::Element::PropertyClass);
    REQUIRE(element.content.property.name.literal == "orgs");
    REQUIRE(element.content.property.valueDefinition.values.size() == 2);
    REQUIRE(element.content.property.valueDefinition.values[0].literal == "confy");
    REQUIRE(element.content.property.valueDefinition.values[1].literal == "apiary");
    REQUIRE(element.content.property.valueDefinition.typeDefinition.baseType == mson::ValueBaseType);
    REQUIRE(element.content.property.valueDefinition.typeDefinition.typeSpecification.name.base == mson::ArrayTypeName);
    REQUIRE(element.content.property.sections.empty());

    element = namedType.node.sections[0].content.elements().at(4);
    REQUIRE(element.klass == mson::Element::PropertyClass);
    REQUIRE(element.content.property.name.literal == "plan");
    REQUIRE(element.content.property.valueDefinition.values.empty());
    REQUIRE(element.content.property.valueDefinition.typeDefinition.baseType == mson::ObjectBaseType);
    REQUIRE(element.content.property.valueDefinition.typeDefinition.typeSpecification.name.base == mson::ObjectTypeName);
    REQUIRE(element.content.property.sections.size() == 2);
    REQUIRE(element.content.property.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(element.content.property.sections[0].content.description == "The subscription plan of the user\n");
    REQUIRE(element.content.property.sections[1].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(element.content.property.sections[1].content.elements().size() == 2);

    subelement = element.content.property.sections[1].content.elements().at(0);
    REQUIRE(subelement.klass == mson::Element::PropertyClass);
    REQUIRE(subelement.content.property.name.literal == "stripe");
    REQUIRE(subelement.content.property.valueDefinition.typeDefinition.typeSpecification.name.base == mson::NumberTypeName);
    REQUIRE(subelement.content.property.valueDefinition.values.size() == 1);
    REQUIRE(subelement.content.property.valueDefinition.values[0].literal == "1284");

    subelement = element.content.property.sections[1].content.elements().at(1);
    REQUIRE(subelement.klass == mson::Element::PropertyClass);
    REQUIRE(subelement.content.property.name.literal == "name");
    REQUIRE(subelement.content.property.valueDefinition.typeDefinition.typeSpecification.name.base == mson::StringTypeName);
    REQUIRE(subelement.content.property.valueDefinition.values.size() == 1);
    REQUIRE(subelement.content.property.valueDefinition.values[0].literal == "Medium");

    SourceMapHelper::check(namedType.sourceMap.name.sourceMap, 0, 16);
    SourceMapHelper::check(namedType.sourceMap.typeDefinition.sourceMap, 0, 16);
    REQUIRE(namedType.sourceMap.sections.collection.size() == 1);
    REQUIRE(namedType.sourceMap.sections.collection[0].elements().collection.size() == 5);

    elementSM = namedType.sourceMap.sections.collection[0].elements().collection[0];
    SourceMapHelper::check(elementSM.property.name.sourceMap, 18, 26);
    SourceMapHelper::check(elementSM.property.valueDefinition.sourceMap, 18, 26);
    REQUIRE(elementSM.property.sections.collection.empty());

    elementSM = namedType.sourceMap.sections.collection[0].elements().collection[1];
    SourceMapHelper::check(elementSM.property.name.sourceMap, 46, 45);
    SourceMapHelper::check(elementSM.property.valueDefinition.sourceMap, 46, 45);
    REQUIRE(elementSM.property.sections.collection.empty());

    elementSM = namedType.sourceMap.sections.collection[0].elements().collection[2];
    SourceMapHelper::check(elementSM.property.name.sourceMap, 93, 32);
    SourceMapHelper::check(elementSM.property.valueDefinition.sourceMap, 93, 32);
    REQUIRE(elementSM.property.sections.collection.empty());

    elementSM = namedType.sourceMap.sections.collection[0].elements().collection[3];
    SourceMapHelper::check(elementSM.property.name.sourceMap, 127, 28);
    SourceMapHelper::check(elementSM.property.valueDefinition.sourceMap, 127, 28);
    REQUIRE(elementSM.property.sections.collection.empty());

    elementSM = namedType.sourceMap.sections.collection[0].elements().collection[4];
    SourceMapHelper::check(elementSM.property.name.sourceMap, 155, 135);
    SourceMapHelper::check(elementSM.property.valueDefinition.sourceMap, 155, 135);
    REQUIRE(elementSM.property.sections.collection.size() == 2);
    SourceMapHelper::check(elementSM.property.sections.collection[0].description.sourceMap, 155, 135);
    REQUIRE(elementSM.property.sections.collection[1].elements().collection.size() == 2);

    subelementSM = elementSM.property.sections.collection[1].elements().collection[0];
    SourceMapHelper::check(subelementSM.property.name.sourceMap, 236, 22);
    SourceMapHelper::check(subelementSM.property.valueDefinition.sourceMap, 236, 22);
    REQUIRE(subelementSM.property.sections.collection.empty());

    subelementSM = elementSM.property.sections.collection[1].elements().collection[1];
    SourceMapHelper::check(subelementSM.property.name.sourceMap, 268, 22);
    SourceMapHelper::check(subelementSM.property.valueDefinition.sourceMap, 268, 22);
    REQUIRE(subelementSM.property.sections.collection.empty());
}

TEST_CASE("Parse named type with a type section", "[mson][named_type]")
{
    mdp::ByteBuffer source = \
    "# User (string)\n"\
    "## Sample: pksunkara";

    ParseResult<mson::NamedType> namedType;
    SectionParserHelper<mson::NamedType, MSONNamedTypeParser>::parse(source, MSONNamedTypeSectionType, namedType, ExportSourcemapOption);

    REQUIRE(namedType.report.error.code == Error::OK);
    REQUIRE(namedType.report.warnings.empty());

    REQUIRE(namedType.node.name.symbol.literal == "User");
    REQUIRE(namedType.node.name.base == mson::UndefinedTypeName);
    REQUIRE(namedType.node.typeDefinition.attributes == 0);
    REQUIRE(namedType.node.typeDefinition.typeSpecification.name.base == mson::StringTypeName);
    REQUIRE(namedType.node.typeDefinition.typeSpecification.name.symbol.empty());
    REQUIRE(namedType.node.typeDefinition.baseType == mson::PrimitiveBaseType);
    REQUIRE(namedType.node.sections.size() == 1);
    REQUIRE(namedType.node.sections[0].klass == mson::TypeSection::SampleClass);
    REQUIRE(namedType.node.sections[0].content.value == "pksunkara");

    SourceMapHelper::check(namedType.sourceMap.name.sourceMap, 0, 16);
    SourceMapHelper::check(namedType.sourceMap.typeDefinition.sourceMap, 0, 16);
    REQUIRE(namedType.sourceMap.sections.collection.size() == 1);
    SourceMapHelper::check(namedType.sourceMap.sections.collection[0].value.sourceMap, 16, 20);
}

TEST_CASE("Parse named type without type specification", "[mson][named_type]")
{
    mdp::ByteBuffer source = \
    "# User\n"\
    "## Sample: pksunkara";

    ParseResult<mson::NamedType> namedType;
    SectionParserHelper<mson::NamedType, MSONNamedTypeParser>::parse(source, MSONNamedTypeSectionType, namedType, ExportSourcemapOption);

    REQUIRE(namedType.report.error.code == Error::OK);
    REQUIRE(namedType.report.warnings.size() == 1);
    REQUIRE(namedType.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(namedType.node.name.symbol.literal == "User");
    REQUIRE(namedType.node.typeDefinition.attributes == 0);
    REQUIRE(namedType.node.typeDefinition.typeSpecification.name.empty());
    REQUIRE(namedType.node.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(namedType.node.sections.size() == 1);
    REQUIRE(namedType.node.sections[0].klass == mson::TypeSection::SampleClass);
    REQUIRE(namedType.node.sections[0].baseType == mson::ImplicitObjectBaseType);

    SourceMapHelper::check(namedType.sourceMap.name.sourceMap, 0, 7);
    REQUIRE(namedType.sourceMap.typeDefinition.sourceMap.empty());
    REQUIRE(namedType.sourceMap.sections.collection.size() == 1);
    REQUIRE(namedType.sourceMap.sections.collection[0].value.sourceMap.empty());
}
