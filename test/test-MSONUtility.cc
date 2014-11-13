//
//  test-MSONUtility.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/24/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONUtility.h"

using namespace mson;
using namespace snowcrashtest;

TEST_CASE("Parse canonical literal value", "[mson][utility]")
{
    std::string source = "red";

    Value value = parseValue(source);

    REQUIRE(value.literal == "red");
    REQUIRE(value.variable == false);
}

TEST_CASE("Parse canonical variable value", "[mson][utility]")
{
    std::string source = "*rel*";

    Value value = parseValue(source);

    REQUIRE(value.literal == "rel");
    REQUIRE(value.variable == true);
}

TEST_CASE("Parse canonical variable value with '_'", "[mson][utility]")
{
    std::string source = "_rel_";

    Value value = parseValue(source);

    REQUIRE(value.literal == "rel");
    REQUIRE(value.variable == true);
}

TEST_CASE("Parse variable value with more than 1 level '*'", "[mson][utility]")
{
    std::string source = "**r*e*l**";

    Value value = parseValue(source);

    REQUIRE(value.literal == "r*e*l");
    REQUIRE(value.variable == true);
}

TEST_CASE("Parse wildcard value", "[mson][utility]")
{
    std::string source = "*";

    Value value = parseValue(source);

    REQUIRE(value.literal.empty());
    REQUIRE(value.variable == true);
}

TEST_CASE("Parse value with non-matching '_'", "[mson][utility]")
{
    std::string source = "_rel";

    Value value = parseValue(source);

    REQUIRE(value.literal == "_rel");
    REQUIRE(value.variable == false);
}

TEST_CASE("Parse canonical type name", "[mson][utility]")
{
    std::string source = "Person";
    TypeName typeName;

    parseTypeName(source, typeName);

    REQUIRE(typeName.name == UndefinedTypeName);
    REQUIRE(typeName.symbol.literal == "Person");
    REQUIRE(typeName.symbol.variable == false);
}

TEST_CASE("Parse boolean type name", "[mson][utility]")
{
    std::string source = "boolean";
    TypeName typeName;

    parseTypeName(source, typeName);

    REQUIRE(typeName.name == BooleanTypeName);
    MSONHelper::empty(typeName.symbol);
}

TEST_CASE("Parse string type name", "[mson][utility]")
{
    std::string source = "string";
    TypeName typeName;

    parseTypeName(source, typeName);

    REQUIRE(typeName.name == StringTypeName);
    MSONHelper::empty(typeName.symbol);
}

TEST_CASE("Parse number type name", "[mson][utility]")
{
    std::string source = "number";
    TypeName typeName;

    parseTypeName(source, typeName);

    REQUIRE(typeName.name == NumberTypeName);
    MSONHelper::empty(typeName.symbol);
}

TEST_CASE("Parse array type name", "[mson][utility]")
{
    std::string source = "array";
    TypeName typeName;

    parseTypeName(source, typeName);

    REQUIRE(typeName.name == ArrayTypeName);
    MSONHelper::empty(typeName.symbol);
}

TEST_CASE("Parse enum type name", "[mson][utility]")
{
    std::string source = "enum";
    TypeName typeName;

    parseTypeName(source, typeName);

    REQUIRE(typeName.name == EnumTypeName);
    MSONHelper::empty(typeName.symbol);
}

TEST_CASE("Parse object type name", "[mson][utility]")
{
    std::string source = "object";
    TypeName typeName;

    parseTypeName(source, typeName);

    REQUIRE(typeName.name == ObjectTypeName);
    MSONHelper::empty(typeName.symbol);
}

TEST_CASE("Parse variable type name", "[mson][utility]")
{
    std::string source = "*T*";
    TypeName typeName;

    parseTypeName(source, typeName);

    REQUIRE(typeName.name == UndefinedTypeName);
    REQUIRE(typeName.symbol.literal == "T");
    REQUIRE(typeName.symbol.variable == true);
}

TEST_CASE("Parse wildcard type name", "[mson][utility]")
{
    std::string source = "*";
    TypeName typeName;

    parseTypeName(source, typeName);

    REQUIRE(typeName.name == UndefinedTypeName);
    REQUIRE(typeName.symbol.literal.empty());
    REQUIRE(typeName.symbol.variable == true);
}

TEST_CASE("Parse required type attribute", "[mson][utility]")
{
    std::string source = "required";
    TypeAttributes typeAttributes = 0;

    bool isAttributeParsed;
    isAttributeParsed = parseTypeAttribute(source, typeAttributes);

    REQUIRE((typeAttributes & RequiredTypeAttribute) == RequiredTypeAttribute);
    REQUIRE((typeAttributes & OptionalTypeAttribute) == 0);
    REQUIRE((typeAttributes & FixedTypeAttribute) == 0);
    REQUIRE((typeAttributes & SampleTypeAttribute) == 0);
    REQUIRE((typeAttributes & DefaultTypeAttribute) == 0);
    REQUIRE(isAttributeParsed);
}

TEST_CASE("Parse optional type attribute", "[mson][utility]")
{
    std::string source = "optional";
    TypeAttributes typeAttributes = 0;

    bool isAttributeParsed;
    isAttributeParsed = parseTypeAttribute(source, typeAttributes);

    REQUIRE((typeAttributes & RequiredTypeAttribute) == 0);
    REQUIRE((typeAttributes & OptionalTypeAttribute) == OptionalTypeAttribute);
    REQUIRE((typeAttributes & FixedTypeAttribute) == 0);
    REQUIRE((typeAttributes & SampleTypeAttribute) == 0);
    REQUIRE((typeAttributes & DefaultTypeAttribute) == 0);
    REQUIRE(isAttributeParsed);
}

TEST_CASE("Parse fixed type attribute", "[mson][utility]")
{
    std::string source = "fixed";
    TypeAttributes typeAttributes = 0;

    bool isAttributeParsed;
    isAttributeParsed = parseTypeAttribute(source, typeAttributes);

    REQUIRE((typeAttributes & RequiredTypeAttribute) == 0);
    REQUIRE((typeAttributes & OptionalTypeAttribute) == 0);
    REQUIRE((typeAttributes & FixedTypeAttribute) == FixedTypeAttribute);
    REQUIRE((typeAttributes & SampleTypeAttribute) == 0);
    REQUIRE((typeAttributes & DefaultTypeAttribute) == 0);
    REQUIRE(isAttributeParsed);
}

TEST_CASE("Parse sample type attribute", "[mson][utility]")
{
    std::string source = "sample";
    TypeAttributes typeAttributes = 0;

    bool isAttributeParsed;
    isAttributeParsed = parseTypeAttribute(source, typeAttributes);

    REQUIRE((typeAttributes & RequiredTypeAttribute) == 0);
    REQUIRE((typeAttributes & OptionalTypeAttribute) == 0);
    REQUIRE((typeAttributes & FixedTypeAttribute) == 0);
    REQUIRE((typeAttributes & SampleTypeAttribute) == SampleTypeAttribute);
    REQUIRE((typeAttributes & DefaultTypeAttribute) == 0);
    REQUIRE(isAttributeParsed);
}

TEST_CASE("Parse default type attribute", "[mson][utility]")
{
    std::string source = "default";
    TypeAttributes typeAttributes = 0;

    bool isAttributeParsed;
    isAttributeParsed = parseTypeAttribute(source, typeAttributes);

    REQUIRE((typeAttributes & RequiredTypeAttribute) == 0);
    REQUIRE((typeAttributes & OptionalTypeAttribute) == 0);
    REQUIRE((typeAttributes & FixedTypeAttribute) == 0);
    REQUIRE((typeAttributes & SampleTypeAttribute) == 0);
    REQUIRE((typeAttributes & DefaultTypeAttribute) == DefaultTypeAttribute);
    REQUIRE(isAttributeParsed);
}

TEST_CASE("Parse required type attribute enclosed in backticks", "[mson][utility]")
{
    std::string source = "`required`";
    TypeAttributes typeAttributes = 0;

    bool isAttributeParsed;
    isAttributeParsed = parseTypeAttribute(source, typeAttributes);

    REQUIRE((typeAttributes & RequiredTypeAttribute) == 0);
    REQUIRE((typeAttributes & OptionalTypeAttribute) == 0);
    REQUIRE((typeAttributes & FixedTypeAttribute) == 0);
    REQUIRE((typeAttributes & SampleTypeAttribute) == 0);
    REQUIRE((typeAttributes & DefaultTypeAttribute) == 0);
    REQUIRE(isAttributeParsed == false);
}

TEST_CASE("Parse canonical type specification", "[mson][utility]")
{
    std::string source = "object";
    TypeSpecification typeSpecification;

    parseTypeSpecification(source, typeSpecification);

    REQUIRE(typeSpecification.name.name == ObjectTypeName);
    MSONHelper::empty(typeSpecification.name.symbol);
    REQUIRE(typeSpecification.nestedTypes.empty());
}

TEST_CASE("Parse linked type specification", "[mson][utility]")
{
    std::string source = "[Person](http://google.com)";
    TypeSpecification typeSpecification;

    parseTypeSpecification(source, typeSpecification);

    REQUIRE(typeSpecification.name.name == UndefinedTypeName);
    REQUIRE(typeSpecification.name.symbol.literal == "Person");
    REQUIRE(typeSpecification.name.symbol.variable == false);
    REQUIRE(typeSpecification.nestedTypes.empty());
}

TEST_CASE("Parse implicit linked type specification", "[mson][utility]")
{
    std::string source = "[Person ] []";
    TypeSpecification typeSpecification;

    parseTypeSpecification(source, typeSpecification);

    REQUIRE(typeSpecification.name.name == UndefinedTypeName);
    REQUIRE(typeSpecification.name.symbol.literal == "Person");
    REQUIRE(typeSpecification.name.symbol.variable == false);
    REQUIRE(typeSpecification.nestedTypes.empty());
}

TEST_CASE("Parse nested types in type specification", "[mson][utility]")
{
    std::string source = "array[ object , Link ]";
    TypeSpecification typeSpecification;

    parseTypeSpecification(source, typeSpecification);

    REQUIRE(typeSpecification.name.name == ArrayTypeName);
    MSONHelper::empty(typeSpecification.name.symbol);
    REQUIRE(typeSpecification.nestedTypes.size() == 2);

    REQUIRE(typeSpecification.nestedTypes[0].name == ObjectTypeName);
    MSONHelper::empty(typeSpecification.nestedTypes[0].symbol);

    REQUIRE(typeSpecification.nestedTypes[1].name == UndefinedTypeName);
    REQUIRE(typeSpecification.nestedTypes[1].symbol.literal == "Link");
    REQUIRE(typeSpecification.nestedTypes[1].symbol.variable == false);
}

TEST_CASE("Parse linked nested types in type specification", "[mson][utility]")
{
    std::string source = "Links[[Repo][],[Search](http://google.com)]";
    TypeSpecification typeSpecification;

    parseTypeSpecification(source, typeSpecification);

    REQUIRE(typeSpecification.name.name == UndefinedTypeName);
    REQUIRE(typeSpecification.name.symbol.literal == "Links");
    REQUIRE(typeSpecification.name.symbol.variable == false);
    REQUIRE(typeSpecification.nestedTypes.size() == 2);

    REQUIRE(typeSpecification.nestedTypes[0].name == UndefinedTypeName);
    REQUIRE(typeSpecification.nestedTypes[0].symbol.literal == "Repo");
    REQUIRE(typeSpecification.nestedTypes[0].symbol.variable == false);

    REQUIRE(typeSpecification.nestedTypes[1].name == UndefinedTypeName);
    REQUIRE(typeSpecification.nestedTypes[1].symbol.literal == "Search");
    REQUIRE(typeSpecification.nestedTypes[1].symbol.variable == false);
}

TEST_CASE("Parse canonical type definition", "[mson][utility]")
{
    std::vector<std::string> attributes;
    snowcrash::ParseResult<TypeDefinition> typeDefinition;
    snowcrash::Blueprint blueprint;

    attributes.push_back("number");
    attributes.push_back("required");

    mdp::ByteBuffer source = "+ (number, required)";
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;

    markdownParser.parse(source, markdownAST);
    snowcrash::SectionParserData pd(0, source, blueprint);

    parseTypeDefinition(markdownAST.children().begin(), pd, attributes, typeDefinition.report, typeDefinition.node);

    REQUIRE(typeDefinition.report.error.code == snowcrash::Error::OK);
    REQUIRE(typeDefinition.report.warnings.empty());

    REQUIRE(typeDefinition.node.typeSpecification.name.name == NumberTypeName);
    MSONHelper::empty(typeDefinition.node.typeSpecification.name.symbol);
    REQUIRE(typeDefinition.node.typeSpecification.nestedTypes.empty());
    REQUIRE(typeDefinition.node.attributes == RequiredTypeAttribute);
}

TEST_CASE("Parse type definition with non recognized type attribute", "[mson][utility]")
{
    std::vector<std::string> attributes;
    snowcrash::ParseResult<TypeDefinition> typeDefinition;
    snowcrash::Blueprint blueprint;

    attributes.push_back("[Person][]");
    attributes.push_back("optinal");

    mdp::ByteBuffer source = "+ ([Person][], optinal)";
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;

    markdownParser.parse(source, markdownAST);
    snowcrash::SectionParserData pd(0, source, blueprint);

    parseTypeDefinition(markdownAST.children().begin(), pd, attributes, typeDefinition.report, typeDefinition.node);

    REQUIRE(typeDefinition.report.error.code == snowcrash::Error::OK);
    REQUIRE(typeDefinition.report.warnings.size() == 1);

    REQUIRE(typeDefinition.node.typeSpecification.name.name == UndefinedTypeName);
    REQUIRE(typeDefinition.node.typeSpecification.name.symbol.literal == "Person");
    REQUIRE(typeDefinition.node.typeSpecification.name.symbol.variable == false);
    REQUIRE(typeDefinition.node.typeSpecification.nestedTypes.empty());
    REQUIRE(typeDefinition.node.attributes == 0);
}

TEST_CASE("Parse type definition when non-structure type has nested types", "[mson][utility]")
{
    std::vector<std::string> attributes;
    snowcrash::ParseResult<TypeDefinition> typeDefinition;
    snowcrash::Blueprint blueprint;

    attributes.push_back("Person[number, string]");

    mdp::ByteBuffer source = "+ (Person[number, string])";
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;

    markdownParser.parse(source, markdownAST);
    snowcrash::SectionParserData pd(0, source, blueprint);

    pd.namedTypeBaseTable["Person"] = mson::PropertyBaseType;

    parseTypeDefinition(markdownAST.children().begin(), pd, attributes, typeDefinition.report, typeDefinition.node);

    REQUIRE(typeDefinition.report.error.code == snowcrash::Error::OK);
    REQUIRE(typeDefinition.report.warnings.size() == 1);
    REQUIRE(typeDefinition.report.warnings[0].code == snowcrash::LogicalErrorWarning);

    REQUIRE(typeDefinition.node.typeSpecification.name.name == UndefinedTypeName);
    REQUIRE(typeDefinition.node.typeSpecification.name.symbol.literal == "Person");
    REQUIRE(typeDefinition.node.typeSpecification.name.symbol.variable == false);
    REQUIRE(typeDefinition.node.typeSpecification.nestedTypes.size() == 2);
    REQUIRE(typeDefinition.node.attributes == 0);
}

TEST_CASE("Build member type from one of", "[mson][utility]")
{
    OneOf oneOf;
    MemberType memberType;

    buildMemberType(oneOf, memberType);

    REQUIRE(memberType.type == OneOfMemberType);
}

TEST_CASE("Build member type from mixin", "[mson][utility]")
{
    Mixin mixin;
    MemberType memberType;

    buildMemberType(mixin, memberType);

    REQUIRE(memberType.type == MixinMemberType);
}

TEST_CASE("Build member type from value member", "[mson][utility]")
{
    ValueMember valueMember;
    MemberType memberType;

    buildMemberType(valueMember, memberType);

    REQUIRE(memberType.type == ValueMemberType);
}

TEST_CASE("Build member type from property memeber", "[mson][utility]")
{
    PropertyMember propertyMember;
    MemberType memberType;

    buildMemberType(propertyMember, memberType);

    REQUIRE(memberType.type == PropertyMemberType);
}

TEST_CASE("Parsing base type from base type name", "[mson][utility]")
{
    BaseTypeName baseTypeName = UndefinedTypeName;
    REQUIRE(parseBaseType(baseTypeName) == UndefinedBaseType);

    baseTypeName = StringTypeName;
    REQUIRE(parseBaseType(baseTypeName) == PrimitiveBaseType);

    baseTypeName = BooleanTypeName;
    REQUIRE(parseBaseType(baseTypeName) == PrimitiveBaseType);

    baseTypeName = NumberTypeName;
    REQUIRE(parseBaseType(baseTypeName) == PrimitiveBaseType);

    baseTypeName = ObjectTypeName;
    REQUIRE(parseBaseType(baseTypeName) == PropertyBaseType);

    baseTypeName = ArrayTypeName;
    REQUIRE(parseBaseType(baseTypeName) == ValueBaseType);

    baseTypeName = EnumTypeName;
    REQUIRE(parseBaseType(baseTypeName) == ValueBaseType);
}

TEST_CASE("Parse canonical property name", "[mson][utility]")
{
    std::string id = "customer";
    snowcrash::ParseResult<PropertyName> propertyName;
    snowcrash::Blueprint blueprint;

    mdp::ByteBuffer source = "+ " + id;
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;

    markdownParser.parse(source, markdownAST);
    snowcrash::SectionParserData pd(0, source, blueprint);

    parsePropertyName(markdownAST.children().begin(), pd, id, propertyName.report, propertyName.node);

    REQUIRE(propertyName.report.error.code == snowcrash::Error::OK);
    REQUIRE(propertyName.report.warnings.empty());

    REQUIRE(propertyName.node.literal == "customer");
    MSONHelper::empty(propertyName.node.variable);
}

TEST_CASE("Parse variable property name", "[mson][utility]")
{
    std::string id = "*rel (Custom String)*";
    snowcrash::ParseResult<PropertyName> propertyName;
    snowcrash::Blueprint blueprint;

    mdp::ByteBuffer source = "+ " + id;
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;

    markdownParser.parse(source, markdownAST);
    snowcrash::SectionParserData pd(0, source, blueprint);

    parsePropertyName(markdownAST.children().begin(), pd, id, propertyName.report, propertyName.node);

    REQUIRE(propertyName.report.error.code == snowcrash::Error::OK);
    REQUIRE(propertyName.report.warnings.empty());

    REQUIRE(propertyName.node.literal.empty());
    REQUIRE(propertyName.node.variable.values.size() == 1);
    REQUIRE(propertyName.node.variable.values[0].literal == "rel");
    REQUIRE(propertyName.node.variable.values[0].variable == false);
    REQUIRE(propertyName.node.variable.typeDefinition.attributes == 0);
    REQUIRE(propertyName.node.variable.typeDefinition.typeSpecification.name.name == UndefinedTypeName);
    REQUIRE(propertyName.node.variable.typeDefinition.typeSpecification.name.symbol.literal == "Custom String");
    REQUIRE(propertyName.node.variable.typeDefinition.typeSpecification.name.symbol.variable == false);
}

TEST_CASE("Parse multi-value variable property name", "[mson][utility]")
{
    std::string id = "*1, 2 (Custom)*";
    snowcrash::ParseResult<PropertyName> propertyName;
    snowcrash::Blueprint blueprint;

    mdp::ByteBuffer source = "+ " + id;
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;

    markdownParser.parse(source, markdownAST);
    snowcrash::SectionParserData pd(0, source, blueprint);

    parsePropertyName(markdownAST.children().begin(), pd, id, propertyName.report, propertyName.node);

    REQUIRE(propertyName.report.error.code == snowcrash::Error::OK);
    REQUIRE(propertyName.report.warnings.empty());

    REQUIRE(propertyName.node.literal.empty());
    REQUIRE(propertyName.node.variable.values.size() == 1);
    REQUIRE(propertyName.node.variable.values[0].literal == "1, 2");
    REQUIRE(propertyName.node.variable.values[0].variable == false);
    REQUIRE(propertyName.node.variable.typeDefinition.attributes == 0);
    REQUIRE(propertyName.node.variable.typeDefinition.typeSpecification.name.name == UndefinedTypeName);
    REQUIRE(propertyName.node.variable.typeDefinition.typeSpecification.name.symbol.literal == "Custom");
    REQUIRE(propertyName.node.variable.typeDefinition.typeSpecification.name.symbol.variable == false);
}