//
//  test-csnowcrash.cc
//  snowcrash
//
//  Created by Ali Khoramshahi on 3/7/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "catch.hpp"
#include "csnowcrash.h"


TEST_CASE("Parse simple blueprint with C interface", "[cinterface]")
{
    const std::string blueprintSource = \
"\n"\
"FORMAT: 1A\n"\
"\n"\
"# My API\n"\
"Description of *My API*.\n"\
"\n"\
"## Message [/message]\n"\
"### GET\n"\
"+ Response 200 (text/plain)\n"\
"\n"\
"        Hello World!\n"\
"\n"\
"# Group test\n"\
"\n"\
"## Message [/message]\n"\
"\n";


    sc_result_s* result;
    sc_blueprint_t* blueprint;
    sc_c_parse(blueprintSource.c_str(), SC_REQUIRE_BLUEPRINT_NAME_OPTION, &result, &blueprint);

    const sc_warnings_t* warns = sc_warnings_handler(result);
    REQUIRE(sc_warnings_size(warns) == 1);

    const sc_warning_t* warn = sc_warning_handler(warns, 0);
    REQUIRE(std::string(sc_warning_message(warn)) == "the resource '/message' is already defined" );

    const sc_metadata_collection_t* meta_col = sc_metadata_collection_handle(blueprint);
    REQUIRE(sc_metadata_collection_size(meta_col) == 1);

    const sc_metadata_t * meta = sc_metadata_handle(meta_col, 0);
    REQUIRE(std::string(sc_metadata_key(meta)) == "FORMAT" );
    REQUIRE(std::string(sc_metadata_value(meta)) == "1A" );

    const sc_resource_groups_collection_t* res_gr_col = sc_resource_groups_collection_handle(blueprint);
    const sc_resource_groups_t* res_gr = sc_resource_groups_handle(res_gr_col, 0);

    REQUIRE(sc_resource_groups_collection_size(res_gr_col) == 2);
    REQUIRE(std::string(sc_resource_groups_name(res_gr)) == "");

    const sc_resource_collection_t* re_col = sc_resource_collection_handle(res_gr);
    const sc_resource_t* res = sc_resource_handle(re_col, 0);

    REQUIRE(std::string(sc_resource_uritemplate(res)) == "/message");

    const sc_action_collection_t* act_col = sc_action_collection_handle(res);
    const sc_action_t* act = sc_action_handle(act_col, 0);

    REQUIRE(std::string(sc_action_httpmethod(act)) == "GET");

    const sc_transaction_example_collection_t* trans_col = sc_transaction_example_collection_handle(act);
    const sc_transaction_example_t* trans = sc_transaction_example_handle(trans_col, 0);

    REQUIRE(std::string(sc_transaction_example_name(trans)) == "");

    const sc_payload_collection_t* respons_col =  sc_payload_collection_handle_responses(trans);
    const sc_payload_t* respons = sc_payload_handle(respons_col, 0);

    REQUIRE(std::string(sc_payload_name(respons)) == "200");

    REQUIRE(std::string(sc_payload_body(respons)) == "Hello World!\n");

    const sc_header_collection_t* header_col = sc_header_collection_handle_payload(respons);
    const sc_header_t* header = sc_header_handle(header_col, 0);

    REQUIRE(std::string(sc_header_key(header)) == "Content-Type");
    REQUIRE(std::string(sc_header_value(header)) == "text/plain");

    sc_blueprint_free(blueprint);
    sc_result_free(result);
}

TEST_CASE("Parse blueprint with multiple requests and responses via C interface", "[cinterface]")
{
    const std::string blueprintSource = \
"# /resource\n"\
"## GET\n"\
"\n"\
"+ request A\n"\
"\n"\
"        A\n"\
"\n"\
"+ response 200\n"\
"\n"\
"        200-A\n"\
"\n"\
"+ request B\n"\
"\n"\
"        B\n"\
"\n"\
"+ response 200\n"\
"\n"\
"        200-B\n"\
"\n";

    sc_result_s* result;
    sc_blueprint_t* blueprint;
    sc_c_parse(blueprintSource.c_str(), SC_RENDER_DESCRIPTIONS_OPTION, &result, &blueprint);

    const sc_resource_groups_collection_t* res_gr_col = sc_resource_groups_collection_handle(blueprint);
    const sc_resource_groups_t* res_gr = sc_resource_groups_handle(res_gr_col, 0);

    REQUIRE(sc_resource_groups_collection_size(res_gr_col) == 1);
    REQUIRE(std::string(sc_resource_groups_name(res_gr)) == "");

    const sc_resource_collection_t* re_col = sc_resource_collection_handle(res_gr);
    const sc_resource_t* res = sc_resource_handle(re_col, 0);

    REQUIRE(std::string(sc_resource_uritemplate(res)) == "/resource");

    const sc_action_collection_t* act_col = sc_action_collection_handle(res);
    const sc_action_t* act = sc_action_handle(act_col, 0);

    REQUIRE(std::string(sc_action_httpmethod(act)) == "GET");

    const sc_transaction_example_collection_t* trans_col = sc_transaction_example_collection_handle(act);

    REQUIRE(sc_transaction_example_collection_size(trans_col) == 2);

}

TEST_CASE("CBlueprint issue on sc_resource_groups_handle", "[cinterface]")
{
    const std::string blueprintSource = \
"# My API\n"\
"\n"\
"# Group A\n"\
"## /Resource A1\n"\
"### GET\n"\
"+ Response 200\n"\
"\n"\
"        A1\n"\
"\n"\
"# Group B\n"\
"## /Resource B1\n"\
"### GET\n"\
"+ Response 200\n"\
"\n"\
"        B1\n"\
"\n"\
"## /Resource B2\n"\
"### GET\n"\
"+ Response 200\n"\
"\n"\
"        B2\n"\
"\n";

    sc_result_s* result;
    sc_blueprint_t* blueprint;
    sc_c_parse(blueprintSource.c_str(), 0, &result, &blueprint);

    const sc_resource_groups_collection_t* res_gr_col = sc_resource_groups_collection_handle(blueprint);
    const sc_resource_groups_t* res_gr1 = sc_resource_groups_handle(res_gr_col, 0);
    const sc_resource_groups_t* res_gr2 = sc_resource_groups_handle(res_gr_col, 1);

    REQUIRE(sc_resource_groups_collection_size(res_gr_col) == 2);

    REQUIRE(std::string(sc_resource_groups_name(res_gr1)) == "A");
    REQUIRE(std::string(sc_resource_groups_name(res_gr2)) == "B");

    const sc_resource_collection_t* res_col1 = sc_resource_collection_handle(res_gr1);
    REQUIRE(sc_resource_collection_size(res_col1) == 1);

    const sc_resource_collection_t* res_col2 = sc_resource_collection_handle(res_gr2);
    REQUIRE(sc_resource_collection_size(res_col2) == 2);

}
