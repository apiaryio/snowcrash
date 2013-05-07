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

TEST_CASE("parser/parse-empty", "Parse empty blueprint.")
{
    Parser parser;
    Result result;
    Blueprint blueprint;
    
    const std::string bluerpintSource = "";
    
    parser.parse(bluerpintSource, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(blueprint.name.empty());
    REQUIRE(blueprint.description.empty());
}

TEST_CASE("parser/parse-simple", "Parse simple blueprint.")
{
    Parser parser;
    Result result;
    Blueprint blueprint;
    
    const std::string bluerpintSource = \
"# Snowcrash API \n\
\n\
# GET /resource\n\
Resource **description**\n\
\n\
+ Response 200\n\
    + Body\n\
\n\
        Text\n\
\n\
            { ... }\n\
";
        
    parser.parse(bluerpintSource, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(blueprint.name == "Snowcrash API");
    REQUIRE(blueprint.description.empty());
    REQUIRE(blueprint.resourceGroups.size() == 1);
    
    ResourceGroup& resourceGroup = blueprint.resourceGroups.front();
    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 1);
    
    // TODO:
    //Resource& resource = resourceGroup.resources.front();
    //REQUIRE(resource.uri == "/resource");
}
