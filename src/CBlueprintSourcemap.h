//
//  CBlueprintSourcemap.h
//  snowcrash
//  C Implementation of BlueprintSourcemap.h for binding purposes
//
//  Created by Pavan Kumar Sunkara on 12/9/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SC_C_BLUEPRINT_SOURCEMAP_H
#define SC_C_BLUEPRINT_SOURCEMAP_H

#include "Platform.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

    /** Class source map wrapper */
    struct sc_source_map_s;
    typedef struct sc_source_map_s sc_source_map_t;

    /** Class Blueprint source map wrapper */
    struct sc_sm_blueprint_s;
    typedef struct sc_sm_blueprint_s sc_sm_blueprint_t;

    /** Array Metadata source map wrapper */
    struct sc_sm_metadata_collection_s;
    typedef struct sc_sm_metadata_collection_s sc_sm_metadata_collection_t;

    /** Class Metadata source map wrapper */
    struct sc_sm_metadata_s;
    typedef struct sc_sm_metadata_s sc_sm_metadata_t;

    /** Array Resource Group Collection source map wrapper */
    struct sc_sm_resource_group_collection_s;
    typedef struct sc_sm_resource_group_collection_s sc_sm_resource_group_collection_t;

    /** Class Resource Group source map wrapper */
    struct sc_sm_resource_group_s;
    typedef struct sc_sm_resource_group_s sc_sm_resource_group_t;

    /** Array Resource Collection source map wrapper */
    struct sc_sm_resource_collection_s;
    typedef struct sc_sm_resource_collection_s sc_sm_resource_collection_t;

    /** Class Resource source map wrapper */
    struct sc_sm_resource_s;
    typedef struct sc_sm_resource_s sc_sm_resource_t;

    /** Class Resource Model source map wrapper */
    struct sc_sm_resource_model_s;
    typedef struct sc_sm_resource_model_s sc_sm_resource_model_t;

    /** Array Payload Collection source map wrapper */
    struct sc_sm_payload_collection_s;
    typedef struct sc_sm_payload_collection_s sc_sm_payload_collection_t;

    /** Class Payload source map wrapper */
    struct sc_sm_payload_s;
    typedef struct sc_sm_payload_s sc_sm_payload_t;

    /** Array Parameter source map wrapper */
    struct sc_sm_parameter_collection_s;
    typedef struct sc_sm_parameter_collection_s sc_sm_parameter_collection_t;

    /** class Parameter source map wrapper */
    struct sc_sm_parameter_s;
    typedef struct sc_sm_parameter_s sc_sm_parameter_t;

    /** Array Header source map wrapper */
    struct sc_sm_header_collection_s;
    typedef struct sc_sm_header_collection_s sc_sm_header_collection_t;

    /** class Header source map wrapper */
    struct sc_sm_header_s;
    typedef struct sc_sm_header_s sc_sm_header_t;

    /** Array Value source map wrapper */
    struct sc_sm_value_collection_s;
    typedef struct sc_sm_value_collection_s sc_sm_value_collection_t;

    /** class Value source map wrapper */
    struct sc_sm_value_s;
    typedef struct sc_sm_value_s sc_sm_value_t;

    /** Array Action source map wrapper */
    struct sc_sm_action_collection_s;
    typedef struct sc_sm_action_collection_s sc_sm_action_collection_t;

    /** class Action source map wrapper */
    struct sc_sm_action_s;
    typedef struct sc_sm_action_s sc_sm_action_t;

    /** Array Transaction Example source map wrapper */
    struct sc_sm_transaction_example_collection_s;
    typedef struct sc_sm_transaction_example_collection_s sc_sm_transaction_example_collection_t;

    /** class Transaction Example source map wrapper */
    struct sc_sm_transaction_example_s;
    typedef struct sc_sm_transaction_example_s sc_sm_transaction_example_t;

    /*----------------------------------------------------------------------*/

    /** \returns size of the source map */
    SC_API size_t sc_source_map_size(const sc_source_map_t* source_map);

    /** \returns Location of the source map item at index */
    SC_API size_t sc_source_map_length(const sc_source_map_t* source_map, size_t index);

    /** \returns Length of the source map item at index */
    SC_API size_t sc_source_map_location(const sc_source_map_t* source_map, size_t index);

    /*----------------------------------------------------------------------*/

    /** \returns pointer to allocated Blueprint source map */
    SC_API sc_sm_blueprint_t* sc_sm_blueprint_new();

    /** \deallocate Blueprint source map from pointer */
    SC_API void sc_sm_blueprint_free(sc_sm_blueprint_t* blueprint);

    /** \returns Blueprint name source map */
    SC_API const sc_source_map_t* sc_sm_blueprint_name(const sc_sm_blueprint_t* blueprint);

    /** \returns Blueprint description source map */
    SC_API const sc_source_map_t* sc_sm_blueprint_description(const sc_sm_blueprint_t* blueprint);

    /*----------------------------------------------------------------------*/

    /** \returns Metadata source map array handle */
    SC_API const sc_sm_metadata_collection_t* sc_sm_metadata_collection_handle(const sc_sm_blueprint_t* blueprint);

    /** \returns size of Metadata source map array */
    SC_API size_t sc_sm_metadata_collection_size(const sc_sm_metadata_collection_t* metadata);

    /*----------------------------------------------------------------------*/

    /** \returns Metadata source map at `index` handle */
    SC_API const sc_sm_metadata_t* sc_sm_metadata_handle(const sc_sm_metadata_collection_t* metadata_col, size_t index);

    /** \returns Metadata source map */
    SC_API const sc_source_map_t* sc_sm_metadata(const sc_sm_metadata_t* metadata);

    /*----------------------------------------------------------------------*/

    /** \returns Resource Group Collection source map array handle */
    SC_API const sc_sm_resource_group_collection_t* sc_sm_resource_group_collection_handle(const sc_sm_blueprint_t* blueprint);

    /** \returns size of Resource Group Collection source map array */
    SC_API size_t sc_sm_resource_group_collection_size(const sc_sm_resource_group_collection_t* resource);

    /*----------------------------------------------------------------------*/

    /** \returns Resource Group source map handle */
    SC_API const sc_sm_resource_group_t* sc_sm_resource_group_handle(const sc_sm_resource_group_collection_t* resource, size_t index);

    /** \returns Resource Group name source map */
    SC_API const sc_source_map_t* sc_sm_resource_group_name(const sc_sm_resource_group_t* handle);

    /** \returns Resource Group description source map */
    SC_API const sc_source_map_t* sc_sm_resource_group_description(const sc_sm_resource_group_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Resource source map array handle */
    SC_API const sc_sm_resource_collection_t* sc_sm_resource_collection_handle(const sc_sm_resource_group_t* handle);

    /** \returns Resource source map array size */
    SC_API size_t sc_sm_resource_collection_size(const sc_sm_resource_collection_t* resource);

    /*----------------------------------------------------------------------*/

    /** \returns Resource source map handle */
    SC_API const sc_sm_resource_t* sc_sm_resource_handle(const sc_sm_resource_collection_t* resource, size_t index);

    /** \returns Resource URITemplate source map */
    SC_API const sc_source_map_t* sc_sm_resource_uritemplate(const sc_sm_resource_t* handle);

    /** \returns Resource name source map */
    SC_API const sc_source_map_t* sc_sm_resource_name(const sc_sm_resource_t* handle);

    /** \returns Resource description source map */
    SC_API const sc_source_map_t* sc_sm_resource_description(const sc_sm_resource_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Payload Collection source map array handle from TransactionExample for requests*/
    SC_API const sc_sm_payload_collection_t* sc_sm_payload_collection_handle_requests(const sc_sm_transaction_example_t* handle);

    /** \returns Payload Collection source map array handle from TransactionExample for responses*/
    SC_API const sc_sm_payload_collection_t* sc_sm_payload_collection_handle_responses(const sc_sm_transaction_example_t* handle);

    /** \returns Payload Collection array size */
    SC_API size_t sc_sm_payload_collection_size(const sc_sm_payload_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Payload source map at `index` handle */
    SC_API const sc_sm_payload_t* sc_sm_payload_handle(const sc_sm_payload_collection_t* handle, size_t index);

    /** \returns Payload source map handle from resource source map */
    SC_API const sc_sm_payload_t* sc_sm_payload_handle_resource(const sc_sm_resource_t* handle);

    /** \returns Payload name source map */
    SC_API const sc_source_map_t* sc_sm_payload_name(const sc_sm_payload_t* handle);

    /** \returns Payload description source map */
    SC_API const sc_source_map_t* sc_sm_payload_description(const sc_sm_payload_t* handle);

    /** \returns Payload body source map */
    SC_API const sc_source_map_t* sc_sm_payload_body(const sc_sm_payload_t* handle);

    /** \returns Payload schema source map */
    SC_API const sc_source_map_t* sc_sm_payload_schema(const sc_sm_payload_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Parameter Collection source map array handle from Payload source map */
    SC_API const sc_sm_parameter_collection_t* sc_sm_parameter_collection_handle_payload(const sc_sm_payload_t* handle);

    /** \returns Parameter Collection source map array handle from Resource source map */
    SC_API const sc_sm_parameter_collection_t* sc_sm_parameter_collection_handle_resource(const sc_sm_resource_t* handle);

    /** \returns Parameter Collection source map array handle from Action source map */
    SC_API const sc_sm_parameter_collection_t* sc_sm_parameter_collection_handle_action(const sc_sm_action_t* handle);

    /** \returns Parameter Collection source map array size */
    SC_API size_t sc_sm_parameter_collection_size(const sc_sm_parameter_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Parameter source map handle */
    SC_API const sc_sm_parameter_t* sc_sm_parameter_handle(const sc_sm_parameter_collection_t* handle, size_t index);

    /** \returns Parameter name source map */
    SC_API const sc_source_map_t* sc_sm_parameter_name(const sc_sm_parameter_t* handle);

    /** \returns Parameter description source map */
    SC_API const sc_source_map_t* sc_sm_parameter_description(const sc_sm_parameter_t* handle);

    /** \returns Parameter type source map */
    SC_API const sc_source_map_t* sc_sm_parameter_type(const sc_sm_parameter_t* handle);

    /** \returns Parameter use source map */
    SC_API const sc_source_map_t* sc_sm_parameter_parameter_use(const sc_sm_parameter_t* handle);

    /** \returns Parameter default value source map */
    SC_API const sc_source_map_t* sc_sm_parameter_default_value(const sc_sm_parameter_t* handle);

    /** \returns Parameter example value source map */
    SC_API const sc_source_map_t* sc_sm_parameter_example_value(const sc_sm_parameter_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Value Collection source map array handle */
    SC_API const sc_sm_value_collection_t* sc_sm_value_collection_handle(const sc_sm_parameter_t* handle);

    /** \returns Value Collection source map array size */
    SC_API size_t sc_sm_value_collection_size(const sc_sm_value_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Value source map handle */
    SC_API const sc_sm_value_t* sc_sm_value_handle(const sc_sm_value_collection_t* handle, size_t index);

    /** \returns Value source map */
    SC_API const sc_source_map_t* sc_sm_value(const sc_sm_value_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Header Collection source map array handle from Payload source map */
    SC_API const sc_sm_header_collection_t* sc_sm_header_collection_handle_payload(const sc_sm_payload_t* handle);

    /** \returns Header Collection source map array handle from Resource source map */
    SC_API const sc_sm_header_collection_t* sc_sm_header_collection_handle_resource(const sc_sm_resource_t* handle);

    /** \returns Header Collection source map array handle from Action source map */
    SC_API const sc_sm_header_collection_t* sc_sm_header_collection_handle_action(const sc_sm_action_t* handle);

    /** \returns Header Collection source map array size */
    SC_API size_t sc_sm_header_collection_size(const sc_sm_header_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Header source map handle */
    SC_API const sc_sm_header_t* sc_sm_header_handle(const sc_sm_header_collection_t* handle, size_t index);

    /** \returns Header source map */
    SC_API const sc_source_map_t* sc_sm_header(const sc_sm_header_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Action Collection source map array handle */
    SC_API const sc_sm_action_collection_t* sc_sm_action_collection_handle(const sc_sm_resource_t* handle);

    /** \returns Action Collection source map array size */
    SC_API size_t sc_sm_action_collection_size(const sc_sm_action_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Action source map handle */
    SC_API const sc_sm_action_t* sc_sm_action_handle(const sc_sm_action_collection_t* handle, size_t index);

    /** \returns Action HTTPMethod source map */
    SC_API const sc_source_map_t* sc_sm_action_httpmethod(const sc_sm_action_t* handle);

    /** \returns Action name source map */
    SC_API const sc_source_map_t* sc_sm_action_name(const sc_sm_action_t* handle);

    /** \returns Action description source map */
    SC_API const sc_source_map_t* sc_sm_action_description(const sc_sm_action_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Transaction Example Collection source map array handle */
    SC_API const sc_sm_transaction_example_collection_t* sc_sm_transaction_example_collection_handle(const sc_sm_action_t* handle);

    /** \returns Transaction Example Collection source map array size */
    SC_API size_t sc_sm_transaction_example_collection_size(const sc_sm_transaction_example_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Transaction Example source map handle */
    SC_API const sc_sm_transaction_example_t* sc_sm_transaction_example_handle(const sc_sm_transaction_example_collection_t* handle, size_t index);

    /** \returns Transaction Example name source map */
    SC_API const sc_source_map_t* sc_sm_transaction_example_name(const sc_sm_transaction_example_t* handle);

    /** \returns Transaction Example description source map */
    SC_API const sc_source_map_t* sc_sm_transaction_example_description(const sc_sm_transaction_example_t* handle);

    /*----------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif
