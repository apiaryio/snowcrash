//
//  test-Parser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/8/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "catch.hpp"
#include "Parser.h"

using namespace snowcrash;

TEST_CASE("parser/init", "Parser construction")
{
    Parser* parser;
    REQUIRE_NOTHROW(parser = ::new Parser);
    REQUIRE_NOTHROW(delete parser);
}

TEST_CASE("parser/params", "parse() method parameters.")
{

    Parser parser;
    REQUIRE_NOTHROW(parser.parse("", nullptr));
    
    bool didEnterCallback = false;
    parser.parse("", [&](const Result& report, const Blueprint& blueprint){
        didEnterCallback = true;
        REQUIRE(report.error.code == Error::OK);
    });
    REQUIRE(didEnterCallback);
}

TEST_CASE("parser/parse-api-name", "Parse API name.")
{
    
    Parser parser;
    const std::string bluerpintSource =
R"(
# Snowcrash API
    
# GET /resource
Resource **description**
    
+ Response 200
    + Body

        Text
    
            { ... }
)";
        
    bool didEnterCallback = false;
    parser.parse(bluerpintSource, [&](const Result& report, const Blueprint& blueprint){
        didEnterCallback = true;
        
        REQUIRE(report.error.code == Error::OK);
        REQUIRE(blueprint.name == "Snowcrash API");
    });
    REQUIRE(didEnterCallback);    
}
