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

TEST_CASE("Parse canonical literal value", "[mson_utility]")
{
    std::string source = "red";

    Value value = parseValue(source);

    REQUIRE(value.literal == "red");
    REQUIRE(value.variable == false);
}

TEST_CASE("Parse canonical variable value", "[mson_utility]")
{
    std::string source = "*rel*";

    Value value = parseValue(source);

    REQUIRE(value.literal == "rel");
    REQUIRE(value.variable == true);
}

TEST_CASE("Parse canonical variable value with '_'", "[mson_utility]")
{
    std::string source = "_rel_";

    Value value = parseValue(source);

    REQUIRE(value.literal == "rel");
    REQUIRE(value.variable == true);
}

TEST_CASE("Parse variable value with more than 1 level '*'", "[mson_utility]")
{
    std::string source = "**r*e*l**";

    Value value = parseValue(source);

    REQUIRE(value.literal == "r*e*l");
    REQUIRE(value.variable == true);
}

TEST_CASE("Parse wildcard value", "[mson_utility]")
{
    std::string source = "*";

    Value value = parseValue(source);

    REQUIRE(value.literal.empty());
    REQUIRE(value.variable == true);
}

TEST_CASE("Parse value with non-matching '_'", "[mson_utility]")
{
    std::string source = "_rel";

    Value value = parseValue(source);

    REQUIRE(value.literal == "_rel");
    REQUIRE(value.variable == false);
}

TEST_CASE("Parse canonical type name",  "[mson_utility]")
{
    std::string source = "Person";

    TypeName typeName = parseTypeName(source);

    REQUIRE(typeName.name == UndefinedTypeName);
    REQUIRE(typeName.symbol.literal == "Person");
    REQUIRE(typeName.symbol.variable == false);
}

TEST_CASE("Parse boolean type name",  "[mson_utility]")
{
    std::string source = "boolean";

    TypeName typeName = parseTypeName(source);

    REQUIRE(typeName.name == BooleanTypeName);
    REQUIRE(typeName.symbol.literal.empty());
    REQUIRE(typeName.symbol.variable == false);
}

TEST_CASE("Parse string type name",  "[mson_utility]")
{
    std::string source = "string";

    TypeName typeName = parseTypeName(source);

    REQUIRE(typeName.name == StringTypeName);
    REQUIRE(typeName.symbol.literal.empty());
    REQUIRE(typeName.symbol.variable == false);
}

TEST_CASE("Parse number type name",  "[mson_utility]")
{
    std::string source = "number";

    TypeName typeName = parseTypeName(source);

    REQUIRE(typeName.name == NumberTypeName);
    REQUIRE(typeName.symbol.literal.empty());
    REQUIRE(typeName.symbol.variable == false);
}

TEST_CASE("Parse array type name",  "[mson_utility]")
{
    std::string source = "array";

    TypeName typeName = parseTypeName(source);

    REQUIRE(typeName.name == ArrayTypeName);
    REQUIRE(typeName.symbol.literal.empty());
    REQUIRE(typeName.symbol.variable == false);
}

TEST_CASE("Parse enum type name",  "[mson_utility]")
{
    std::string source = "enum";

    TypeName typeName = parseTypeName(source);

    REQUIRE(typeName.name == EnumTypeName);
    REQUIRE(typeName.symbol.literal.empty());
    REQUIRE(typeName.symbol.variable == false);
}

TEST_CASE("Parse object type name",  "[mson_utility]")
{
    std::string source = "object";

    TypeName typeName = parseTypeName(source);

    REQUIRE(typeName.name == ObjectTypeName);
    REQUIRE(typeName.symbol.literal.empty());
    REQUIRE(typeName.symbol.variable == false);
}

TEST_CASE("Parse variable type name", "[mson_utility]")
{
    std::string source = "*T*";

    TypeName typeName = parseTypeName(source);

    REQUIRE(typeName.name == UndefinedTypeName);
    REQUIRE(typeName.symbol.literal == "T");
    REQUIRE(typeName.symbol.variable == true);
}

TEST_CASE("Parse wildcard type name", "[mson_utility]")
{
    std::string source = "*";

    TypeName typeName = parseTypeName(source);

    REQUIRE(typeName.name == UndefinedTypeName);
    REQUIRE(typeName.symbol.literal.empty());
    REQUIRE(typeName.symbol.variable == true);
}
