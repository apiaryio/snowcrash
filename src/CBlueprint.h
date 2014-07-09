//
//  CBlueprint.h
//  snowcrash
//  C Implementation of Blueprint.h for binding purposes
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SC_C_BLUEPRINT_H
#define SC_C_BLUEPRINT_H

#include "Platform.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

    /** brief Blueprint Parser Options */
    typedef unsigned int sc_blueprint_parser_options;

    /** brief Blueprint Parser Options Enums */
    enum sc_blueprint_parser_option {
        SC_RENDER_DESCRIPTIONS_OPTION = (1 << 0),       /// < Render Markdown in description.
        SC_REQUIRE_BLUEPRINT_NAME_OPTION = (1 << 1),    /// < Treat missing blueprint name as error
    };

    /** Parameter Use flag */
    typedef enum sc_parameter_use {
        SC_UNDEFINED_PARAMETER_USE,
        SC_OPTIONAL_PARAMETER_USE,
        SC_REQUIRED_PARAMETER_USE
    } sc_parameter_use;

    /** Class Blueprint wrapper */
    struct sc_blueprint_s;
    typedef struct sc_blueprint_s sc_blueprint_t;

    /** Array Metadata wrapper */
    struct sc_metadata_collection_s;
    typedef struct sc_metadata_collection_s sc_metadata_collection_t;

    /** Class Metadata wrapper */
    struct sc_metadata_s;
    typedef struct sc_metadata_s sc_metadata_t;

    /** Array Resource Groups wrapper */
    struct sc_resource_groups_collection_s;
    typedef struct sc_resource_groups_collection_s sc_resource_groups_collection_t;

    /** Class Resource Group wrapper */
    struct sc_resource_groups_s;
    typedef struct sc_resource_groups_s sc_resource_groups_t;

    /** Array Resources Collection wrapper */
    struct sc_resource_collection_s;
    typedef struct sc_resource_collection_s sc_resource_collection_t;

    /** Class Resources wrapper */
    struct sc_resource_s;
    typedef struct sc_resource_s sc_resource_t;

    /** Class Resource Model wrapper */
    struct sc_resource_model_s;
    typedef struct sc_resource_model_s sc_resource_model_t;

    /** Array Payload Collection wrapper */
    struct sc_payload_collection_s;
    typedef struct sc_payload_collection_s sc_payload_collection_t;

    /** Class Payload wrapper */
    struct sc_payload_s;
    typedef struct sc_payload_s sc_payload_t;

    /** Array Parameter wrapper */
    struct sc_parameter_collection_s;
    typedef struct sc_parameter_collection_s sc_parameter_collection_t;

    /** class Parameter wrapper */
    struct sc_parameter_s;
    typedef struct sc_parameter_s sc_parameter_t;

    /** Array Header wrapper */
    struct sc_header_collection_s;
    typedef struct sc_header_collection_s sc_header_collection_t;

    /** class Header wrapper */
    struct sc_header_s;
    typedef struct sc_header_s sc_header_t;

    /** Array Value wrapper */
    struct sc_value_collection_s;
    typedef struct sc_value_collection_s sc_value_collection_t;

    /** class Value wrapper */
    struct sc_value_s;
    typedef struct sc_value_s sc_value_t;

    /** Array Header wrapper */
    struct sc_header_collection_s;
    typedef struct sc_header_collection_s sc_header_collection_t;

    /** class Header wrapper */
    struct sc_header_s;
    typedef struct sc_header_s sc_header_t;

    /** Array Action wrapper */
    struct sc_action_collection_s;
    typedef struct sc_action_collection_s sc_action_collection_t;

    /** class Action wrapper */
    struct sc_action_s;
    typedef struct sc_action_s sc_action_t;

    /** Array Transaction Example wrapper */
    struct sc_transaction_example_collection_s;
    typedef struct sc_transaction_example_collection_s sc_transaction_example_collection_t;

    /** class Transaction Example wrapper */
    struct sc_transaction_example_s;
    typedef struct sc_transaction_example_s sc_transaction_example_t;

    /*----------------------------------------------------------------------*/

    /** \returns pointer to allocated Blueprint*/
    SC_API sc_blueprint_t* sc_blueprint_new();

    /** \deallocate Blueprint from pointer*/
    SC_API void sc_blueprint_free(sc_blueprint_t* blueprint);

    /** \returns Blueprint name */
    SC_API const char* sc_blueprint_name(const sc_blueprint_t* blueprint);

    /** \returns Blueprint description */
    SC_API const char* sc_blueprint_description(const sc_blueprint_t* blueprint);

    /*----------------------------------------------------------------------*/

    /** \returns Metadata Handle */
    SC_API const sc_metadata_collection_t* sc_metadata_collection_handle(const sc_blueprint_t* blueprint);

    /** \returns size of Metadata array */
    SC_API size_t sc_metadata_collection_size(const sc_metadata_collection_t* metadata);

    /*----------------------------------------------------------------------*/

    /** \returns Metadata at `index` handle */
    SC_API const sc_metadata_t* sc_metadata_handle(const sc_metadata_collection_t* metadata_col, size_t index);

    /** \returns Metadata key */
    SC_API const char* sc_metadata_key(const sc_metadata_t* metadata);

    /** \returns Metadata value */
    SC_API const char* sc_metadata_value(const sc_metadata_t* metadata);

    /*----------------------------------------------------------------------*/

    /** \returns Resource Groups Array Handle */
    SC_API const sc_resource_groups_collection_t* sc_resource_groups_collection_handle(const sc_blueprint_t* blueprint);

    /** \returns size of Resource Groups array */
    SC_API size_t sc_resource_groups_collection_size(const sc_resource_groups_collection_t* resource);

    /*----------------------------------------------------------------------*/

    /** \returns Resource Groups Handle */
    SC_API const sc_resource_groups_t* sc_resource_groups_handle(const sc_resource_groups_collection_t* resource, size_t index);

    /** \returns Resource Groups name */
    SC_API const char* sc_resource_groups_name(const sc_resource_groups_t* handle);

    /** \returns Resource Groups Description */
    SC_API const char* sc_resource_groups_description(const sc_resource_groups_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Resource Array Handle */
    SC_API const sc_resource_collection_t* sc_resource_collection_handle(const sc_resource_groups_t* handle);

    /** \returns Resource array size*/
    SC_API size_t sc_resource_collection_size(const sc_resource_collection_t* resource);

    /*----------------------------------------------------------------------*/

    /** \returns Resource Handle */
    SC_API const sc_resource_t* sc_resource_handle(const sc_resource_collection_t* resource, size_t index);

    /** \returns Resource URITemplate */
    SC_API const char* sc_resource_uritemplate(const sc_resource_t* handle);

    /** \returns Resource name */
    SC_API const char* sc_resource_name(const sc_resource_t* handle);

    /** \returns Resource description */
    SC_API const char* sc_resource_description(const sc_resource_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Payload Collection handle from TransactionExample for requests*/
    SC_API const sc_payload_collection_t* sc_payload_collection_handle_requests(const sc_transaction_example_t* handle);

    /** \returns Payload Collection handle from TransactionExample for responses*/
    SC_API const sc_payload_collection_t* sc_payload_collection_handle_responses(const sc_transaction_example_t* handle);

    /** \returns Payload Collection size */
    SC_API size_t sc_payload_collection_size(const sc_payload_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Payload at `index` Handle*/
    SC_API const sc_payload_t* sc_payload_handle(const sc_payload_collection_t* handle, size_t index);

    /** \returns Payload Handle from resource*/
    SC_API const sc_payload_t* sc_payload_handle_resource(const sc_resource_t* handle);

    /** \returns Payload name */
    SC_API const char* sc_payload_name(const sc_payload_t* handle);

    /** \returns Payload description */
    SC_API const char* sc_payload_description(const sc_payload_t* handle);

    /** \returns Payload body */
    SC_API const char* sc_payload_body(const sc_payload_t* handle);

    /** \returns Payload schema */
    SC_API const char* sc_payload_schema(const sc_payload_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Parameter Collection handle from Payload*/
    SC_API const sc_parameter_collection_t* sc_parameter_collection_handle_payload(const sc_payload_t* handle);

    /** \returns Parameter Collection handle from Resource*/
    SC_API const sc_parameter_collection_t* sc_parameter_collection_handle_resource(const sc_resource_t* handle);

    /** \returns Parameter Collection handle from Action*/
    SC_API const sc_parameter_collection_t* sc_parameter_collection_handle_action(const sc_action_t* handle);

    /** \returns Parameter Collection size */
    SC_API size_t sc_parameter_collection_size(const sc_parameter_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Parameter handle */
    SC_API const sc_parameter_t* sc_parameter_handle(const sc_parameter_collection_t* handle, size_t index);

    /** \returns Parameter name */
    SC_API const char* sc_parameter_name(const sc_parameter_t* handle);

    /** \returns Parameter description */
    SC_API const char* sc_parameter_description(const sc_parameter_t* handle);

    /** \returns Parameter Type */
    SC_API const char* sc_parameter_type(const sc_parameter_t* handle);

    /** \returns Parameter Parameter Use */
    SC_API sc_parameter_use sc_parameter_parameter_use(const sc_parameter_t* handle);

    /** \returns Parameter Default Value */
    SC_API const char* sc_parameter_default_value(const sc_parameter_t* handle);

    /** \returns Parameter Example Value */
    SC_API const char* sc_parameter_example_value(const sc_parameter_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Value Collection handle */
    SC_API const sc_value_collection_t* sc_value_collection_handle(const sc_parameter_t* handle);

    /** \returns Value Collection size */
    SC_API size_t sc_value_collection_size(const sc_value_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Value handle */
    SC_API const sc_value_t* sc_value_handle(const sc_value_collection_t* handle, size_t index);

    /** \returns Value string */
    SC_API const char* sc_value_string(const sc_value_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Header Collection handle from Payload*/
    SC_API const sc_header_collection_t* sc_header_collection_handle_payload(const sc_payload_t* handle);

    /** \returns Header Collection handle from Resource*/
    SC_API const sc_header_collection_t* sc_header_collection_handle_resource(const sc_resource_t* handle);

    /** \returns Header Collection handle from Action*/
    SC_API const sc_header_collection_t* sc_header_collection_handle_action(const sc_action_t* handle);

    /** \returns Header Collection size */
    SC_API size_t sc_header_collection_size(const sc_header_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Header handle */
    SC_API const sc_header_t* sc_header_handle(const sc_header_collection_t* handle, size_t index);

    /** \returns Header Key */
    SC_API const char* sc_header_key(const sc_header_t* handle);

    /** \returns Header Value */
    SC_API const char* sc_header_value(const sc_header_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Action Collection handle */
    SC_API const sc_action_collection_t* sc_action_collection_handle(const sc_resource_t* handle);

    /** \returns Action Collection size */
    SC_API size_t sc_action_collection_size(const sc_action_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Action handle */
    SC_API const sc_action_t* sc_action_handle(const sc_action_collection_t* handle, size_t index);

    /** \returns Action HTTPMethod */
    SC_API const char* sc_action_httpmethod(const sc_action_t* handle);

    /** \returns Action name */
    SC_API const char* sc_action_name(const sc_action_t* handle);

    /** \returns Action Description */
    SC_API const char* sc_action_description(const sc_action_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Transaction Example Collection handle */
    SC_API const sc_transaction_example_collection_t* sc_transaction_example_collection_handle(const sc_action_t* handle);

    /** \returns Transaction Example Collection size */
    SC_API size_t sc_transaction_example_collection_size(const sc_transaction_example_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Transaction Example handle */
    SC_API const sc_transaction_example_t* sc_transaction_example_handle(const sc_transaction_example_collection_t* handle, size_t index);

    /** \returns Transaction Example name */
    SC_API const char* sc_transaction_example_name(const sc_transaction_example_t* handle);

    /** \returns Transaction Example Description */
    SC_API const char* sc_transaction_example_description(const sc_transaction_example_t* handle);

    /*----------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif
