//
//  test-Parser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/8/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "catch.hpp"
#include "Parser.h"
#include "csnowcrash.h"
#include <cstring>

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
    
    const std::string blueprintSource = "";
    
    parser.parse(blueprintSource, 0, result, blueprint);
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
    
    const std::string blueprintSource = \
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
        
    parser.parse(blueprintSource, 0, result, blueprint);
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
    REQUIRE(resource.actions.size() == 1);
    
    Action& action = resource.actions[0];
    REQUIRE(action.method == "GET");
    REQUIRE(action.description == "Resource **description**\n\n");
    REQUIRE(!action.examples.empty());
    REQUIRE(action.examples.front().requests.empty());
    REQUIRE(action.examples.front().responses.size() == 1);
    
    Response& response = action.examples.front().responses[0];
    REQUIRE(response.name == "200");
    REQUIRE(response.body == "Text\n\n{ ... }\n");
}

TEST_CASE("Parse simple blueprint with C interface", "[parser]")
{
    //C_Result result;
    //cs_Blueprint blueprint;

    const std::string blueprintSource = \
"meta: data\nfoo:bar\n"\
"HOST http://test.test.com\n"\
"FORMAT: 1A\n"\
"# /\n"\
"+ Headers\n"\
"\n"\
"        header1: value1\n"\
"\n"\
"# GET\n"\
"+ Headers\n"\
"\n"\
"        header2: value2\n"\
"\n"\
"+ Response 200\n"\
"    + Headers\n"\
"\n"\
"            header3: value3\n"\
"\n"\
"\n";

    sc_return_value * parse_struct = cs_c_parse(blueprintSource.c_str());
    sc_warnings_s* warn_col = sc_warnings_handler(parse_struct->result);
    REQUIRE(sc_warnings_size(warn_col) == 2);

    sc_warning_t* wern = sc_warning_handler(warn_col, 0);
    REQUIRE(sc_warning_code(wern) == 9);

    sc_location_t* loc = sc_location_handler((sc_source_annotation_t*)wern);
    REQUIRE(sc_location_location(loc,0) == 62);

    const sc_metadata_collection_t* meta_col = sc_metadata_collection_handle(parse_struct->blueprint);
    REQUIRE(sc_metadata_collection_size(meta_col) == 4);

    const sc_metadata_t * meta = cs_metadata_handle(meta_col,0);
    REQUIRE(std::string(cs_metadata_key(meta)) == "meta" );
    REQUIRE(std::string(cs_metadata_value(meta)) == "data" );

    const sc_resource_groups_collection_t* res_gr_col = sc_resource_groups_collection_handle(parse_struct->blueprint);
    const sc_resource_groups_t* res_gr = sc_resource_groups_handle(res_gr_col , 0);

    REQUIRE(std::string(sc_resource_groups_name(res_gr)) == "");

    const sc_resource_collection_t* re_col = sc_resource_collection_handle(res_gr);
    const sc_resource_t* res = sc_resource_handle(re_col,0);

    REQUIRE(std::string(sc_resource_uritemplate(res)) == "/");

    const sc_action_collection_t* act_col = sc_action_collection_handle(res);
    const sc_action_t* act = sc_action_handle(act_col , 0);

    REQUIRE(std::string(sc_action_httpmethod(act)) == "GET");

    const sc_transaction_example_collection_t* trans_col = sc_transaction_example_collection_handle(act);
    const sc_transaction_example_t* trans = sc_transaction_example_handle(trans_col, 0);

    REQUIRE(std::string(sc_transaction_example_name(trans)) == "");

    const sc_payload_collection_t* respons_col =  sc_payload_collection_handle_responses(trans);
    const sc_payload_t* respons = sc_payload_handle(respons_col , 0);

    REQUIRE(std::string(sc_payload_name(respons)) == "200");

    const sc_header_collection_t* header_col = sc_header_collection_handle_payload(respons);
    const sc_header_t* header = sc_header_handle(header_col,2);

    REQUIRE(std::string(sc_header_key(header)) == "header3");
    REQUIRE(std::string(sc_header_value(header)) == "value3");

    sc_blueprint_free(parse_struct->blueprint);
    sc_result_free(parse_struct->result);
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
    const std::string blueprintSource = \
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
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
}

TEST_CASE("Support description ending with an list item", "[parser][issue][#8]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ a description item 
    //+ Response 200 
    //
    //        ...
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ a description item\n"\
    "+ Response 200\n"\
    "\n"\
    "        ...\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description == "+ a description item\n");
    REQUIRE(!blueprint.resourceGroups[0].resources[0].actions[0].examples.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "...\n");
}

TEST_CASE("Invalid ‘warning: empty body asset’ for certain status codes", "[parser][issue][#13]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Response 304
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Response 304\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(!blueprint.resourceGroups[0].resources[0].actions[0].examples.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "304");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body.empty());
}

TEST_CASE("Parse adjacent asset blocks", "[parser][issue][#9]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ response 200
    //
    //asset
    //
    //    pre
    //+ response 404
    //
    //    Not found
    //
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ response 200\n"\
    "\n"\
    "asset\n"\
    "\n"\
    "    pre\n"\
    "+ response 404\n"\
    "\n"\
    "        Not found\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(!blueprint.resourceGroups[0].resources[0].actions[0].examples.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 2);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "asset\n\npre\n");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[1].name == "404");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[1].body == "Not found\n");
}

TEST_CASE("Parse adjacent nested asset blocks", "[parser][issue][crash][#9]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ response 200
    //    + body
    //
    //        A
    //
    //    B
    //C
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ response 200\n"\
    "    + body\n"\
    "\n"\
    "        A\n"\
    "\n"\
    "    B\n"\
    "C\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    CHECK(result.warnings.size() == 2);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "A\nB\nC\n");
}

TEST_CASE("Parse adjacent asset list blocks", "[parser][issue][#9]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ response 200
    //+ list
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ response 200\n"\
    "+ list\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == IgnoringWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body.empty());
}

TEST_CASE("SIGTERM parsing blueprint", "[parser][issue][#45]")
{
    // Blueprint in question: See GH issue #45
    const std::string blueprintSource = "# A\n# B\nC\n\nD\n\nE\n\nF\n\nG\n\n# /1\n# GET\n+ Request\n+ Response 200\n    + Body\n\n            H\n\nI\n# J\n> K";

    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == 0);
    REQUIRE(result.warnings.size() == 4);
}

