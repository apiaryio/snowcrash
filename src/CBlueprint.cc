//
//  CBlueprint.cc
//  snowcrash
//  C Implementation of Blueprint.h for binding purposes
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "CBlueprint.h"
#include "snowcrash.h"

SC_API sc_blueprint_t* sc_blueprint_new()
{
    return AS_TYPE(sc_blueprint_t, ::new snowcrash::Blueprint);
}

SC_API void sc_blueprint_free(sc_blueprint_t* blueprint)
{
    ::delete AS_TYPE(snowcrash::Blueprint, blueprint);
}

SC_API const char* sc_blueprint_name(const sc_blueprint_t* blueprint)
{
    const snowcrash::Blueprint* p = AS_CTYPE(snowcrash::Blueprint, blueprint);
    if (!p)
        return "";

    return p->name.c_str();
}

SC_API const char* sc_blueprint_description(const sc_blueprint_t* blueprint)
{
    const snowcrash::Blueprint* p = AS_CTYPE(snowcrash::Blueprint, blueprint);
    if (!p)
        return "";

    return p->description.c_str();
}

/*----------------------------------------------------------------------*/

SC_API const sc_metadata_collection_t* sc_metadata_collection_handle(const sc_blueprint_t* blueprint)
{
    const snowcrash::Blueprint* p = AS_CTYPE(snowcrash::Blueprint, blueprint);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_metadata_collection_t, &p->metadata);
}

SC_API size_t sc_metadata_collection_size(const sc_metadata_collection_t* metadata)
{
    const snowcrash::MetadataCollection* p = AS_CTYPE(snowcrash::MetadataCollection, metadata);
    if (!p)
        return 0;

    return p->size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_metadata_t* sc_metadata_handle(const sc_metadata_collection_t* metadata_col, size_t index)
{
    const snowcrash::MetadataCollection* p = AS_CTYPE(snowcrash::MetadataCollection, metadata_col);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_metadata_t, &p->at(index));
}

SC_API const char* sc_metadata_key(const sc_metadata_t* metadata)
{
    const snowcrash::Metadata* p = AS_CTYPE(snowcrash::Metadata, metadata);
    if (!p)
        return "";

    return p->first.c_str();
}

SC_API const char* sc_metadata_value(const sc_metadata_t* metadata)
{
    const snowcrash::Metadata* p = AS_CTYPE(snowcrash::Metadata, metadata);
    if (!p)
        return "";

    return p->second.c_str();
}

/*----------------------------------------------------------------------*/

SC_API const sc_resource_group_collection_t* sc_resource_group_collection_handle(const sc_blueprint_t* blueprint)
{
    const snowcrash::Blueprint* p = AS_CTYPE(snowcrash::Blueprint, blueprint);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_resource_group_collection_t, &p->resourceGroups);
}

SC_API size_t sc_resource_group_collection_size(const sc_resource_group_collection_t* resource)
{
    const snowcrash::ResourceGroups* p = AS_CTYPE(snowcrash::ResourceGroups, resource);
    if (!p)
        return 0;

    return p->size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_resource_group_t* sc_resource_group_handle(const sc_resource_group_collection_t* resource, size_t index)
{
    const snowcrash::ResourceGroups* p = AS_CTYPE(snowcrash::ResourceGroups, resource);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_resource_group_t, &p->at(index));
}

SC_API const char* sc_resource_group_name(const sc_resource_group_t* handle)
{
    const snowcrash::ResourceGroup* p = AS_CTYPE(snowcrash::ResourceGroup, handle);
    if(!p)
        return "";

    return p->name.c_str();
}

SC_API const char* sc_resource_group_description(const sc_resource_group_t* handle)
{
    const snowcrash::ResourceGroup* p = AS_CTYPE(snowcrash::ResourceGroup, handle);
    if(!p)
        return "";

    return p->description.c_str();
}

/*----------------------------------------------------------------------*/

SC_API const sc_resource_collection_t* sc_resource_collection_handle(const sc_resource_group_t* handle)
{
    const snowcrash::ResourceGroup* p = AS_CTYPE(snowcrash::ResourceGroup, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_resource_collection_t, &p->resources);
}

SC_API size_t sc_resource_collection_size(const sc_resource_collection_t* resource)
{
    const snowcrash::Resources* p = AS_CTYPE(snowcrash::Resources, resource);
    if (!p)
        return 0;

    return p->size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_resource_t* sc_resource_handle(const sc_resource_collection_t* resource, size_t index)
{
    const snowcrash::Resources* p = AS_CTYPE(snowcrash::Resources, resource);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_resource_t, &p->at(index));
}

SC_API const char* sc_resource_uritemplate(const sc_resource_t* handle)
{
    const snowcrash::Resource* p = AS_CTYPE(snowcrash::Resource, handle);
    if(!p)
        return "";

    return p->uriTemplate.c_str();
}

SC_API const char* sc_resource_name(const sc_resource_t* handle)
{
    const snowcrash::Resource* p = AS_CTYPE(snowcrash::Resource, handle);
    if(!p)
        return "";

    return p->name.c_str();
}

SC_API const char* sc_resource_description(const sc_resource_t* handle)
{
    const snowcrash::Resource* p = AS_CTYPE(snowcrash::Resource, handle);
    if(!p)
        return "";

    return p->description.c_str();
}

/*----------------------------------------------------------------------*/

SC_API const sc_payload_collection_t* sc_payload_collection_handle_requests(const sc_transaction_example_t* handle)
{
    const snowcrash::TransactionExample* p = AS_CTYPE(snowcrash::TransactionExample, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_payload_collection_t, &p->requests);
}

SC_API const sc_payload_collection_t* sc_payload_collection_handle_responses(const sc_transaction_example_t* handle)
{
    const snowcrash::TransactionExample* p = AS_CTYPE(snowcrash::TransactionExample, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_payload_collection_t, &p->responses);
}

SC_API size_t sc_payload_collection_size(const sc_payload_collection_t* handle)
{
    const snowcrash::Collection<snowcrash::Payload>::type* p = AS_CTYPE(snowcrash::Collection<snowcrash::Payload>::type, handle);
    if (!p)
        return 0;

    return p->size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_payload_t* sc_payload_handle(const sc_payload_collection_t* handle, size_t index)
{
    const snowcrash::Collection<snowcrash::Payload>::type* p = AS_CTYPE(snowcrash::Collection<snowcrash::Payload>::type, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_payload_t, &p->at(index));
}

SC_API const sc_payload_t* sc_payload_handle_resource(const sc_resource_t* handle)
{
    const snowcrash::Resource* p = AS_CTYPE(snowcrash::Resource, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_payload_t, &p->model);
}

SC_API const char* sc_payload_name(const sc_payload_t* handle)
{
    const snowcrash::Payload* p = AS_CTYPE(snowcrash::Payload, handle);
    if(!p)
        return "";

    return p->name.c_str();
}

SC_API const char* sc_payload_description(const sc_payload_t* handle)
{
    const snowcrash::Payload* p = AS_CTYPE(snowcrash::Payload, handle);
    if(!p)
        return "";

    return p->description.c_str();
}

SC_API const char* sc_payload_body(const sc_payload_t* handle)
{
    const snowcrash::Payload* p = AS_CTYPE(snowcrash::Payload, handle);
    if(!p)
        return "";

    return p->body.c_str();
}

SC_API const char* sc_payload_schema(const sc_payload_t* handle)
{
    const snowcrash::Payload* p = AS_CTYPE(snowcrash::Payload, handle);
    if(!p)
        return "";

    return p->schema.c_str();
}

/*----------------------------------------------------------------------*/

SC_API const sc_reference_t* sc_reference_handle_payload(const sc_payload_t* handle)
{
    const snowcrash::Payload* p = AS_CTYPE(snowcrash::Payload, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_reference_t, &p->reference);
}

SC_API const char* sc_reference_id(const sc_reference_t* handle)
{
    const snowcrash::Reference* p = AS_CTYPE(snowcrash::Reference, handle);
    if (!p)
        return "";

    return p->id.c_str();
}

SC_API sc_reference_type_t sc_reference_type(const sc_reference_t* handle)
{
    const snowcrash::Reference* p = AS_CTYPE(snowcrash::Reference, handle);
    if (!p)
        return (sc_reference_type_t)0;

    return (sc_reference_type_t)p->type;
}

/*----------------------------------------------------------------------*/

SC_API const sc_parameter_collection_t* sc_parameter_collection_handle_payload(const sc_payload_t* handle)
{
    const snowcrash::Payload* p = AS_CTYPE(snowcrash::Payload, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_parameter_collection_t, &p->parameters);
}

SC_API const sc_parameter_collection_t* sc_parameter_collection_handle_resource(const sc_resource_t* handle)
{
    const snowcrash::Resource* p = AS_CTYPE(snowcrash::Resource, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_parameter_collection_t, &p->parameters);
}

SC_API const sc_parameter_collection_t* sc_parameter_collection_handle_action(const sc_action_t* handle)
{
    const snowcrash::Action* p = AS_CTYPE(snowcrash::Action, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_parameter_collection_t, &p->parameters);
}

SC_API size_t sc_parameter_collection_size(const sc_parameter_collection_t* handle)
{
    const snowcrash::Parameters* p = AS_CTYPE(snowcrash::Parameters, handle);
    if(!p)
        return 0;

    return p->size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_parameter_t* sc_parameter_handle(const sc_parameter_collection_t* handle, size_t index)
{
    const snowcrash::Parameters* p = AS_CTYPE(snowcrash::Parameters, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_parameter_t, &p->at(index));
}

SC_API const char* sc_parameter_name(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* p = AS_CTYPE(snowcrash::Parameter, handle);
    if (!p)
        return "";

    return p->name.c_str();
}

SC_API const char* sc_parameter_description(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* p = AS_CTYPE(snowcrash::Parameter, handle);
    if (!p)
        return "";

    return p->description.c_str();
}

SC_API const char* sc_parameter_type(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* p = AS_CTYPE(snowcrash::Parameter, handle);
    if (!p)
        return "";

    return p->type.c_str();
}

SC_API sc_parameter_use sc_parameter_parameter_use(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* p = AS_CTYPE(snowcrash::Parameter, handle);
    if (!p)
        return SC_UNDEFINED_PARAMETER_USE;

    return (sc_parameter_use)p->use;
}

SC_API const char* sc_parameter_default_value(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* p = AS_CTYPE(snowcrash::Parameter, handle);
    if (!p)
        return "";

    return p->defaultValue.c_str();
}

SC_API const char* sc_parameter_example_value(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* p = AS_CTYPE(snowcrash::Parameter, handle);
    if (!p)
        return "";

    return p->exampleValue.c_str();
}

/*----------------------------------------------------------------------*/

SC_API const sc_value_collection_t* sc_value_collection_handle(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* p = AS_CTYPE(snowcrash::Parameter, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_value_collection_t, &p->values);
}

SC_API size_t sc_value_collection_size(const sc_value_collection_t* handle)
{
    const snowcrash::Values* p = AS_CTYPE(snowcrash::Values, handle);
    if (!p)
        return 0;

    return p->size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_value_t* sc_value_handle(const sc_value_collection_t* handle, size_t index)
{
    const snowcrash::Values* p = AS_CTYPE(snowcrash::Values, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_value_t, &p->at(index));
}

SC_API const char* sc_value(const sc_value_t* handle)
{
    const snowcrash::Value* p = AS_CTYPE(snowcrash::Value, handle);
    if(!p)
        return "";

    return p->c_str();
}

/*----------------------------------------------------------------------*/

SC_API const sc_header_collection_t* sc_header_collection_handle_payload(const sc_payload_t* handle)
{
    const snowcrash::Payload* p = AS_CTYPE(snowcrash::Payload, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_header_collection_t, &p->headers);
}

SC_API const sc_header_collection_t* sc_header_collection_handle_resource(const sc_resource_t* handle)
{
    const snowcrash::Resource* p = AS_CTYPE(snowcrash::Resource, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_header_collection_t, &p->headers);
}

SC_API const sc_header_collection_t* sc_header_collection_handle_action(const sc_action_t* handle)
{
    const snowcrash::Action* p = AS_CTYPE(snowcrash::Action, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_header_collection_t, &p->headers);
}

SC_API size_t sc_header_collection_size(const sc_header_collection_t* handle)
{
    const snowcrash::Headers* p = AS_CTYPE(snowcrash::Headers, handle);
    if (!p)
        return 0;

    return p->size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_header_t* sc_header_handle(const sc_header_collection_t* handle, size_t index)
{
    const snowcrash::Headers* p = AS_CTYPE(snowcrash::Headers, handle);
    if (!p)
        return NULL;

    return AS_CTYPE( sc_header_t, &p->at(index));
}

SC_API const char* sc_header_key(const sc_header_t* handle)
{
    const snowcrash::Header* p = AS_CTYPE(snowcrash::Header, handle);
    if(!p)
        return "";

    return p->first.c_str();
}

SC_API const char* sc_header_value(const sc_header_t* handle)
{
    const snowcrash::Header* p = AS_CTYPE(snowcrash::Header, handle);
    if(!p)
        return "";

    return p->second.c_str();
}

/*----------------------------------------------------------------------*/

SC_API const sc_action_collection_t* sc_action_collection_handle(const sc_resource_t* handle)
{
    const snowcrash::Resource* p = AS_CTYPE(snowcrash::Resource, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_action_collection_t, &p->actions);
}

SC_API size_t sc_action_collection_size(const sc_action_collection_t* handle)
{
    const snowcrash::Actions* p = AS_CTYPE(snowcrash::Actions, handle);
    if (!p)
        return 0;

    return p->size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_action_t* sc_action_handle(const sc_action_collection_t* handle, size_t index)
{
    const snowcrash::Actions* p = AS_CTYPE(snowcrash::Actions, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_action_t, &p->at(index));
}

SC_API const char* sc_action_httpmethod(const sc_action_t* handle)
{
    const snowcrash::Action* p = AS_CTYPE(snowcrash::Action, handle);
    if (!p)
        return "";

    return p->method.c_str();
}

SC_API const char* sc_action_name(const sc_action_t* handle)
{
    const snowcrash::Action* p = AS_CTYPE(snowcrash::Action, handle);
    if (!p)
        return "";

    return p->name.c_str();
}

SC_API const char* sc_action_description(const sc_action_t* handle)
{
    const snowcrash::Action* p = AS_CTYPE(snowcrash::Action, handle);
    if (!p)
        return "";

    return p->description.c_str();
}

/*----------------------------------------------------------------------*/

SC_API const sc_transaction_example_collection_t* sc_transaction_example_collection_handle(const sc_action_t* handle)
{
    const snowcrash::Action* p = AS_CTYPE(snowcrash::Action, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_transaction_example_collection_t, &p->examples);
}

SC_API size_t sc_transaction_example_collection_size(const sc_transaction_example_collection_t* handle)
{
    const snowcrash::TransactionExamples* p = AS_CTYPE(snowcrash::TransactionExamples, handle);
    if (!p)
        return 0;

    return p->size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_transaction_example_t* sc_transaction_example_handle(const sc_transaction_example_collection_t* handle, size_t index)
{
    const snowcrash::TransactionExamples* p = AS_CTYPE(snowcrash::TransactionExamples, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_transaction_example_t, &p->at(index));
}

SC_API const char* sc_transaction_example_name(const sc_transaction_example_t* handle)
{
    const snowcrash::TransactionExample* p = AS_CTYPE(snowcrash::TransactionExample, handle);
    if (!p)
        return "";

    return p->name.c_str();
}

SC_API const char* sc_transaction_example_description(const sc_transaction_example_t* handle)
{
    const snowcrash::TransactionExample* p = AS_CTYPE(snowcrash::TransactionExample, handle);
    if (!p)
        return "";

    return p->description.c_str();
}

/*----------------------------------------------------------------------*/
