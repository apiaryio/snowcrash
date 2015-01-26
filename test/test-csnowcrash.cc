//
//  test-csnowcrash.cc
//  snowcrash
//
//  Created by Ali Khoramshahi on 3/7/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "csnowcrash.h"

TEST_CASE("Parse simple blueprint with C interface", "[cinterface]")
{
    mdp::ByteBuffer source = \
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
    "## Message [/message]\n";

    sc_report_t* report;
    sc_blueprint_t* blueprint;
    sc_sm_blueprint_t* sm_blueprint;

    sc_c_parse(source.c_str(), SC_REQUIRE_BLUEPRINT_NAME_OPTION | SC_EXPORT_SORUCEMAP_OPTION, &report, &blueprint, &sm_blueprint);

    const sc_warnings_t* warns = sc_warnings_handler(report);
    REQUIRE(sc_warnings_size(warns) == 1);

    const sc_warning_t* warn = sc_warning_handler(warns, 0);
    REQUIRE(std::string(sc_warning_message(warn)) == "the resource '/message' is already defined" );

    const sc_metadata_collection_t* meta_col = sc_metadata_collection_handle(blueprint);
    const sc_sm_metadata_collection_t* sm_meta_col = sc_sm_metadata_collection_handle(sm_blueprint);

    REQUIRE(sc_metadata_collection_size(meta_col) == 1);
    REQUIRE(sc_sm_metadata_collection_size(sm_meta_col) == 1);

    const sc_metadata_t* meta = sc_metadata_handle(meta_col, 0);
    REQUIRE(std::string(sc_metadata_key(meta)) == "FORMAT" );
    REQUIRE(std::string(sc_metadata_value(meta)) == "1A" );

    const sc_source_map_t* sm_meta = sc_sm_metadata(sc_sm_metadata_handle(sm_meta_col, 0));

    REQUIRE(sc_source_map_size(sm_meta) == 1);
    REQUIRE(sc_source_map_location(sm_meta, 0) == 0);
    REQUIRE(sc_source_map_length(sm_meta, 0) == 12);

    const sc_resource_group_collection_t* res_gr_col = sc_resource_group_collection_handle(blueprint);
    const sc_sm_resource_group_collection_t* sm_res_gr_col = sc_sm_resource_group_collection_handle(sm_blueprint);

    REQUIRE(sc_resource_group_collection_size(res_gr_col) == 2);
    REQUIRE(sc_sm_resource_group_collection_size(sm_res_gr_col) == 2);

    const sc_resource_group_t* res_gr = sc_resource_group_handle(res_gr_col, 0);
    const sc_sm_resource_group_t* sm_res_gr = sc_sm_resource_group_handle(sm_res_gr_col, 0);

    REQUIRE(std::string(sc_resource_group_name(res_gr)) == "");

    const sc_source_map_t* sm_res_gr_name = sc_sm_resource_group_name(sm_res_gr);

    REQUIRE(sc_source_map_size(sm_res_gr_name) == 0);

    const sc_resource_collection_t* re_col = sc_resource_collection_handle(res_gr);
    const sc_sm_resource_collection_t* sm_re_col = sc_sm_resource_collection_handle(sm_res_gr);

    REQUIRE(sc_resource_collection_size(re_col) == 1);
    REQUIRE(sc_sm_resource_collection_size(sm_re_col) == 1);

    const sc_resource_t* res = sc_resource_handle(re_col, 0);
    const sc_sm_resource_t* sm_res = sc_sm_resource_handle(sm_re_col, 0);

    REQUIRE(std::string(sc_resource_uritemplate(res)) == "/message");

    const sc_source_map_t* sm_res_uritemplate = sc_sm_resource_uritemplate(sm_res);

    REQUIRE(sc_source_map_size(sm_res_uritemplate) == 1);
    REQUIRE(sc_source_map_location(sm_res_uritemplate, 0) == 47);
    REQUIRE(sc_source_map_length(sm_res_uritemplate, 0) == 22);

    const sc_action_collection_t* act_col = sc_action_collection_handle(res);
    const sc_sm_action_collection_t* sm_act_col = sc_sm_action_collection_handle(sm_res);

    REQUIRE(sc_action_collection_size(act_col) == 1);
    REQUIRE(sc_sm_action_collection_size(sm_act_col) == 1);

    const sc_action_t* act = sc_action_handle(act_col, 0);
    const sc_sm_action_t* sm_act = sc_sm_action_handle(sm_act_col, 0);

    REQUIRE(std::string(sc_action_httpmethod(act)) == "GET");

    const sc_source_map_t* sm_act_httpmethod = sc_sm_action_httpmethod(sm_act);

    REQUIRE(sc_source_map_size(sm_act_httpmethod) == 1);
    REQUIRE(sc_source_map_location(sm_act_httpmethod, 0) == 69);
    REQUIRE(sc_source_map_length(sm_act_httpmethod, 0) == 8);

    const sc_transaction_example_collection_t* trans_col = sc_transaction_example_collection_handle(act);
    const sc_sm_transaction_example_collection_t* sm_trans_col = sc_sm_transaction_example_collection_handle(sm_act);

    REQUIRE(sc_transaction_example_collection_size(trans_col) == 1);
    REQUIRE(sc_sm_transaction_example_collection_size(sm_trans_col) == 1);

    const sc_transaction_example_t* trans = sc_transaction_example_handle(trans_col, 0);
    const sc_sm_transaction_example_t* sm_trans = sc_sm_transaction_example_handle(sm_trans_col, 0);

    REQUIRE(std::string(sc_transaction_example_name(trans)) == "");

    const sc_source_map_t* sm_trans_name = sc_sm_transaction_example_name(sm_trans);

    REQUIRE(sc_source_map_size(sm_trans_name) == 0);

    const sc_payload_collection_t* resp_col =  sc_payload_collection_handle_responses(trans);
    const sc_sm_payload_collection_t* sm_resp_col = sc_sm_payload_collection_handle_responses(sm_trans);

    REQUIRE(sc_payload_collection_size(resp_col) == 1);
    REQUIRE(sc_sm_payload_collection_size(sm_resp_col) == 1);

    const sc_payload_t* resp = sc_payload_handle(resp_col, 0);
    const sc_sm_payload_t* sm_resp = sc_sm_payload_handle(sm_resp_col, 0);

    REQUIRE(std::string(sc_payload_name(resp)) == "200");
    REQUIRE(std::string(sc_payload_body(resp)) == "Hello World!\n");

    const sc_asset_t* asset_body = sc_asset_body_handle(resp);
    const sc_asset_t* asset_schema = sc_asset_schema_handle(resp);

    REQUIRE(std::string(sc_asset_source(asset_body)) == "Hello World!\n");
    REQUIRE(std::string(sc_asset_resolved(asset_body)) == "");

    REQUIRE(std::string(sc_asset_source(asset_schema)) == "");
    REQUIRE(std::string(sc_asset_resolved(asset_schema)) == "");

    const sc_source_map_t* sm_resp_name = sc_sm_payload_name(sm_resp);
    const sc_source_map_t* sm_resp_body = sc_sm_payload_body(sm_resp);

    REQUIRE(sc_source_map_size(sm_resp_name) == 1);
    REQUIRE(sc_source_map_location(sm_resp_name, 0) == 79);
    REQUIRE(sc_source_map_length(sm_resp_name, 0) == 27);

    REQUIRE(sc_source_map_size(sm_resp_body) == 1);
    REQUIRE(sc_source_map_location(sm_resp_body, 0) == 110);
    REQUIRE(sc_source_map_length(sm_resp_body, 0) == 17);

    const sc_header_collection_t* header_col = sc_header_collection_handle_payload(resp);
    const sc_sm_header_collection_t* sm_header_col = sc_sm_header_collection_handle_payload(sm_resp);

    REQUIRE(sc_header_collection_size(header_col) == 1);
    REQUIRE(sc_sm_header_collection_size(sm_header_col) == 1);

    const sc_header_t* header = sc_header_handle(header_col, 0);

    REQUIRE(std::string(sc_header_key(header)) == "Content-Type");
    REQUIRE(std::string(sc_header_value(header)) == "text/plain");

    const sc_source_map_t* sm_header = sc_sm_header(sc_sm_header_handle(sm_header_col, 0));

    REQUIRE(sc_source_map_size(sm_header) == 1);
    REQUIRE(sc_source_map_location(sm_header, 0) == 79);
    REQUIRE(sc_source_map_length(sm_header, 0) == 27);

    sc_sm_blueprint_free(sm_blueprint);
    sc_blueprint_free(blueprint);
    sc_report_free(report);
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

    sc_report_t* report;
    sc_blueprint_t* blueprint;
    sc_sm_blueprint_t* sm_blueprint;

    sc_c_parse(blueprintSource.c_str(), SC_RENDER_DESCRIPTIONS_OPTION | SC_EXPORT_SORUCEMAP_OPTION, &report, &blueprint, &sm_blueprint);

    const sc_resource_group_collection_t* res_gr_col = sc_resource_group_collection_handle(blueprint);
    const sc_sm_resource_group_collection_t* sm_res_gr_col = sc_sm_resource_group_collection_handle(sm_blueprint);

    REQUIRE(sc_resource_group_collection_size(res_gr_col) == 1);
    REQUIRE(sc_sm_resource_group_collection_size(sm_res_gr_col) == 1);

    const sc_resource_group_t* res_gr = sc_resource_group_handle(res_gr_col, 0);
    const sc_sm_resource_group_t* sm_res_gr = sc_sm_resource_group_handle(sm_res_gr_col, 0);

    REQUIRE(std::string(sc_resource_group_name(res_gr)) == "");

    const sc_source_map_t* sm_res_gr_name = sc_sm_resource_group_name(sm_res_gr);

    REQUIRE(sc_source_map_size(sm_res_gr_name) == 0);

    const sc_resource_collection_t* re_col = sc_resource_collection_handle(res_gr);
    const sc_sm_resource_collection_t* sm_re_col = sc_sm_resource_collection_handle(sm_res_gr);

    REQUIRE(sc_resource_collection_size(re_col) == 1);
    REQUIRE(sc_sm_resource_collection_size(sm_re_col) == 1);

    const sc_resource_t* res = sc_resource_handle(re_col, 0);
    const sc_sm_resource_t* sm_res = sc_sm_resource_handle(sm_re_col, 0);

    REQUIRE(std::string(sc_resource_uritemplate(res)) == "/resource");

    const sc_source_map_t* sm_res_uritemplate = sc_sm_resource_uritemplate(sm_res);

    REQUIRE(sc_source_map_size(sm_res_uritemplate) == 1);
    REQUIRE(sc_source_map_location(sm_res_uritemplate, 0) == 0);
    REQUIRE(sc_source_map_length(sm_res_uritemplate, 0) == 12);

    const sc_action_collection_t* act_col = sc_action_collection_handle(res);
    const sc_sm_action_collection_t* sm_act_col = sc_sm_action_collection_handle(sm_res);

    REQUIRE(sc_action_collection_size(act_col) == 1);
    REQUIRE(sc_sm_action_collection_size(sm_act_col) == 1);

    const sc_action_t* act = sc_action_handle(act_col, 0);
    const sc_sm_action_t* sm_act = sc_sm_action_handle(sm_act_col, 0);

    REQUIRE(std::string(sc_action_httpmethod(act)) == "GET");

    const sc_source_map_t* sm_act_httpmethod = sc_sm_action_httpmethod(sm_act);

    REQUIRE(sc_source_map_size(sm_act_httpmethod) == 1);
    REQUIRE(sc_source_map_location(sm_act_httpmethod, 0) == 12);
    REQUIRE(sc_source_map_length(sm_act_httpmethod, 0) == 8);

    const sc_transaction_example_collection_t* trans_col = sc_transaction_example_collection_handle(act);
    const sc_sm_transaction_example_collection_t* sm_trans_col = sc_sm_transaction_example_collection_handle(sm_act);

    REQUIRE(sc_transaction_example_collection_size(trans_col) == 2);
    REQUIRE(sc_sm_transaction_example_collection_size(sm_trans_col) == 2);

    sc_sm_blueprint_free(sm_blueprint);
    sc_blueprint_free(blueprint);
    sc_report_free(report);
}

TEST_CASE("CBlueprint issue on sc_resource_group_handle", "[cinterface]")
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

    sc_report_t* report;
    sc_blueprint_t* blueprint;
    sc_sm_blueprint_t* sm_blueprint;

    sc_c_parse(blueprintSource.c_str(), 0, &report, &blueprint, &sm_blueprint);

    const sc_resource_group_collection_t* res_gr_col = sc_resource_group_collection_handle(blueprint);
    const sc_resource_group_t* res_gr1 = sc_resource_group_handle(res_gr_col, 0);
    const sc_resource_group_t* res_gr2 = sc_resource_group_handle(res_gr_col, 1);

    const sc_sm_resource_group_collection_t* sm_res_gr_col = sc_sm_resource_group_collection_handle(sm_blueprint);

    REQUIRE(sc_sm_resource_group_collection_size(sm_res_gr_col) == 0);
    REQUIRE(sc_resource_group_collection_size(res_gr_col) == 2);

    REQUIRE(std::string(sc_resource_group_name(res_gr1)) == "A");
    REQUIRE(std::string(sc_resource_group_name(res_gr2)) == "B");

    const sc_resource_collection_t* res_col1 = sc_resource_collection_handle(res_gr1);
    REQUIRE(sc_resource_collection_size(res_col1) == 1);

    const sc_resource_collection_t* res_col2 = sc_resource_collection_handle(res_gr2);
    REQUIRE(sc_resource_collection_size(res_col2) == 2);

    sc_sm_blueprint_free(sm_blueprint);
    sc_blueprint_free(blueprint);
    sc_report_free(report);
}

TEST_CASE("Parse blueprint with multiple reference via C interface", "[cinterface]")
{
    const std::string blueprintSource = \
    "#api name\n\n"\
    "# Resource 1 [/1]\n"\
    "+ Model (text/plain)\n\n"\
    "        `resource model` 1\n\n"\
    "# Resource 2 [/2]\n"\
    "## Retrieve [GET]\n\n"\
    "+ Response 200\n\n"\
    "    [Resource 1][]\n\n"\
    "# Resource 3 [/3]\n"\
    "## Retrieve [GET]\n\n"\
    "+ Response 200\n\n"\
    "    [Resource 4][]\n\n"\
    "# Resource 4 [/4]\n"\
    "+ Model (text/plain)\n\n"\
    "        `resource model` 4\n";

    sc_report_t* report;
    sc_blueprint_t* blueprint;
    sc_sm_blueprint_t* sm_blueprint;

    sc_c_parse(blueprintSource.c_str(), 0, &report, &blueprint, &sm_blueprint);

    const sc_resource_group_collection_t* res_gr_col = sc_resource_group_collection_handle(blueprint);
    REQUIRE(sc_resource_group_collection_size(res_gr_col) == 1);

    const sc_resource_group_t* res_gr = sc_resource_group_handle(res_gr_col, 0);
    REQUIRE(std::string(sc_resource_group_name(res_gr)) == "");

    const sc_resource_collection_t* sm_res_gr_col = sc_resource_collection_handle(res_gr);
    REQUIRE(sc_resource_collection_size(sm_res_gr_col) == 4);

    const sc_resource_t *res = sc_resource_handle(sm_res_gr_col, 1);
    REQUIRE(std::string(sc_resource_uritemplate(res)) == "/2");
    REQUIRE(std::string(sc_resource_name(res)) == "Resource 2");

    const sc_action_collection_t* action_col = sc_action_collection_handle(res);
    REQUIRE(sc_action_collection_size(action_col) == 1);

    const sc_action_t* action = sc_action_handle(action_col, 0);
    REQUIRE(std::string(sc_action_httpmethod(action)) == "GET");
    REQUIRE(std::string(sc_action_name(action)) == "Retrieve");

    const sc_transaction_example_collection_t* example_col = sc_transaction_example_collection_handle(action);
    REQUIRE(sc_transaction_example_collection_size(example_col) == 1);

    const sc_transaction_example_t* example = sc_transaction_example_handle(example_col, 0);
    REQUIRE(std::string(sc_transaction_example_name(example)) == "");

    const sc_payload_collection_t* response_col = sc_payload_collection_handle_responses(example);
    REQUIRE(sc_payload_collection_size(response_col) == 1);

    const sc_payload_t* response = sc_payload_handle(response_col, 0);
    REQUIRE(std::string(sc_payload_name(response)) == "200");
    REQUIRE(std::string(sc_payload_body(response)) == "`resource model` 1\n");

    const sc_reference_t* reference = sc_reference_handle_payload(response);
    REQUIRE(std::string(sc_reference_id(reference)) == "Resource 1");
    REQUIRE(sc_reference_type(reference) == 0);

    res = sc_resource_handle(sm_res_gr_col, 2);
    REQUIRE(std::string(sc_resource_uritemplate(res)) == "/3");
    REQUIRE(std::string(sc_resource_name(res)) == "Resource 3");

    action_col = sc_action_collection_handle(res);
    REQUIRE(sc_action_collection_size(action_col) == 1);

    action = sc_action_handle(action_col, 0);
    REQUIRE(std::string(sc_action_httpmethod(action)) == "GET");
    REQUIRE(std::string(sc_action_name(action)) == "Retrieve");

    example_col = sc_transaction_example_collection_handle(action);
    REQUIRE(sc_transaction_example_collection_size(example_col) == 1);

    example = sc_transaction_example_handle(example_col, 0);
    REQUIRE(std::string(sc_transaction_example_name(example)) == "");

    response_col = sc_payload_collection_handle_responses(example);
    REQUIRE(sc_payload_collection_size(response_col) == 1);

    response = sc_payload_handle(response_col, 0);
    REQUIRE(std::string(sc_payload_name(response)) == "200");
    REQUIRE(std::string(sc_payload_body(response)) == "`resource model` 4\n");

    reference = sc_reference_handle_payload(response);
    REQUIRE(std::string(sc_reference_id(reference)) == "Resource 4");
    REQUIRE(sc_reference_type(reference) == 0);

    sc_sm_blueprint_free(sm_blueprint);
    sc_blueprint_free(blueprint);
    sc_report_free(report);
}
