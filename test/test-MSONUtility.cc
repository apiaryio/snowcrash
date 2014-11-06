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

    parseTypeDefinition(markdownAST.children().begin(), pd, attributes, typeDefinition);

    REQUIRE(typeDefinition.report.error.code == snowcrash::Error::OK);
    REQUIRE(typeDefinition.report.warnings.empty());

    REQUIRE(typeDefinition.node.typeSpecification.name.name == NumberTypeName);
    MSONHelper::empty(typeDefinition.node.typeSpecification.name.symbol);
    REQUIRE(typeDefinition.node.typeSpecification.nestedTypes.empty());
    REQUIRE((typeDefinition.node.attributes & RequiredTypeAttribute) == RequiredTypeAttribute);
    REQUIRE((typeDefinition.node.attributes & OptionalTypeAttribute) == 0);
    REQUIRE((typeDefinition.node.attributes & FixedTypeAttribute) == 0);
    REQUIRE((typeDefinition.node.attributes & SampleTypeAttribute) == 0);
    REQUIRE((typeDefinition.node.attributes & DefaultTypeAttribute) == 0);
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

    parseTypeDefinition(markdownAST.children().begin(), pd, attributes, typeDefinition);

    REQUIRE(typeDefinition.report.error.code == snowcrash::Error::OK);
    REQUIRE(typeDefinition.report.warnings.size() == 1);

    REQUIRE(typeDefinition.node.typeSpecification.name.name == UndefinedTypeName);
    REQUIRE(typeDefinition.node.typeSpecification.name.symbol.literal == "Person");
    REQUIRE(typeDefinition.node.typeSpecification.name.symbol.variable == false);
    REQUIRE(typeDefinition.node.typeSpecification.nestedTypes.empty());
    REQUIRE((typeDefinition.node.attributes & RequiredTypeAttribute) == 0);
    REQUIRE((typeDefinition.node.attributes & OptionalTypeAttribute) == 0);
    REQUIRE((typeDefinition.node.attributes & FixedTypeAttribute) == 0);
    REQUIRE((typeDefinition.node.attributes & SampleTypeAttribute) == 0);
    REQUIRE((typeDefinition.node.attributes & DefaultTypeAttribute) == 0);
}

TEST_CASE("Build member type from one of")
{
    OneOf oneOf;
    MemberType memberType;

    buildMemberType(oneOf, memberType);

    REQUIRE(memberType.type == OneOfMemberType);
}

TEST_CASE("Build member type from mixin")
{
    Mixin mixin;
    MemberType memberType;

    buildMemberType(mixin, memberType);

    REQUIRE(memberType.type == MixinMemberType);
}

TEST_CASE("Build member type from value member")
{
    ValueMember valueMember;
    MemberType memberType;

    buildMemberType(valueMember, memberType);

    REQUIRE(memberType.type == ValueMemberType);
}

TEST_CASE("Build member type from property memeber")
{
    PropertyMember propertyMember;
    MemberType memberType;

    buildMemberType(propertyMember, memberType);

    REQUIRE(memberType.type == PropertyMemberType);
}
