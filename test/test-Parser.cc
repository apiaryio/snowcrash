//
//  test-Parser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/8/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "catch.hpp"
#include "Parser.h"

using namespace snowcrash;

TEST_CASE("Parser construction", "[parser]")
{
    Parser* parser;
    REQUIRE_NOTHROW(parser = ::new Parser);
    REQUIRE_NOTHROW(delete parser);
}

TEST_CASE("Parse empty blueprint", "[parser]")
{
    Parser parser;
    Result result;
    Blueprint blueprint;
    
    const std::string bluerpintSource = "";
    
    parser.parse(bluerpintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(blueprint.metadata.empty());
    REQUIRE(blueprint.name.empty());
    REQUIRE(blueprint.description.empty());
    REQUIRE(blueprint.resourceGroups.empty());
}

TEST_CASE("Parse simple blueprint", "[parser]")
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
        
    parser.parse(bluerpintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(blueprint.name == "Snowcrash API");
    REQUIRE(blueprint.description.empty());
    REQUIRE(blueprint.resourceGroups.size() == 1);
    
    ResourceGroup& resourceGroup = blueprint.resourceGroups.front();
    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 1);
    
    Resource& resource = resourceGroup.resources.front();
    REQUIRE(resource.uriTemplate == "/resource");
    REQUIRE(resource.methods.size() == 1);
    
    Method& method = resource.methods[0];
    REQUIRE(method.method == "GET");
    REQUIRE(method.description == "Resource **description**\n\n");
    REQUIRE(method.requests.empty());
    REQUIRE(method.responses.size() == 1);
    
    Response& response = method.responses[0];
    REQUIRE(response.name == "200");
    REQUIRE(response.body == "Text\n\n{ ... }\n");
}

TEST_CASE("Parse bluprint with unsupported characters", "[parser]")
{
    Parser parser;
    Result result;
    Blueprint blueprint;
    
    parser.parse("hello\t", 0, result, blueprint);
    REQUIRE(result.error.code != Error::OK);
    REQUIRE(result.error.location.size() == 1);
    REQUIRE(result.error.location[0].location == 5);
    REQUIRE(result.error.location[0].length == 1);

    parser.parse("sun\n\rsalt\n\r", 0, result, blueprint);
    REQUIRE(result.error.code != Error::OK);
    REQUIRE(result.error.location.size() == 1);
    REQUIRE(result.error.location[0].location == 4);
    REQUIRE(result.error.location[0].length == 1);
    
}

TEST_CASE("Do not report duplicate response when media type differs", "[method][issue][#14]")
{
    // Blueprint in question:
    //R"(
    //# GET /message
    //+ Response 200 (application/json)
    //
    //        { "msg": "Hello." }
    //
    //+ Response 200 (text/plain)
    //
    //        Hello.
    //");
    const std::string bluerpintSource = \
"\n\
# GET /message\n\
+ Response 200 (application/json)\n\
\n\
        { \"msg\": \"Hello.\" }\n\
\n\
+ Response 200 (text/plain)\n\
\n\
        Hello.\n\
";

    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(bluerpintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
}
