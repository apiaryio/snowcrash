//
//  CBluePrint.cc
//  snowcrash
//  C Implementation of BluePrint.h for binding purposes
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "CBlueprint.h"
#include "snowcrash.h"


using namespace snowcrash;

#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)

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
    const Blueprint* b = AS_CTYPE(snowcrash::Blueprint, blueprint);
    if (!b)
        return "";

    return b->name.c_str();
}

SC_API const char* sc_blueprint_description(const sc_blueprint_t* blueprint)
{
    const Blueprint* b = AS_CTYPE(snowcrash::Blueprint, blueprint);
    if (!b)
        return "";

    return b->description.c_str();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_metadata_collection_t* sc_metadata_collection_handle(const sc_blueprint_t* blueprint)
{
    const Blueprint* b = AS_CTYPE(snowcrash::Blueprint, blueprint);
    if (!b)
        return NULL;

    return AS_CTYPE(sc_metadata_collection_t, &b->metadata);
}

SC_API const int sc_metadata_collection_size(const sc_metadata_collection_t* metadata)
{
    const snowcrash::Collection<Metadata>::type* b = AS_CTYPE(snowcrash::Collection<Metadata>::type, metadata);
    if (!b)
        return 0;

    return b->size();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_metadata_t* cs_metadata_handle(const sc_metadata_collection_t* metadata_col, int i)
{
    const snowcrash::Collection<Metadata>::type* b = AS_CTYPE(snowcrash::Collection<Metadata>::type, metadata_col);
    if (!b)
        return NULL;

    return AS_CTYPE(sc_metadata_t, &b->at(i));
}

SC_API const char* cs_metadata_key(const sc_metadata_t* metadata)
{
    const snowcrash::Metadata* b = AS_CTYPE(snowcrash::Metadata, metadata);
    if (!b)
        return "";

    return b->first.c_str();
}

SC_API const char* cs_metadata_value(const sc_metadata_t* metadata)
{
    const snowcrash::Metadata* b = AS_CTYPE(snowcrash::Metadata, metadata);
    if (!b)
        return "";

    return b->second.c_str();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_resource_groups_collection_t* sc_resource_groups_collection_handle(const sc_blueprint_t* blueprint)
{
    const Blueprint* b = AS_CTYPE(snowcrash::Blueprint, blueprint);
    if (!b)
        return NULL;

    return AS_CTYPE(sc_resource_groups_collection_t, &b->resourceGroups);
}

SC_API const int sc_resource_groups_collection_size(const sc_resource_groups_collection_t* resource)
{
    const snowcrash::Collection<Resource>::type* b = AS_CTYPE(snowcrash::Collection<Resource>::type, resource);
    if (!b)
        return 0;

    return b->size();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_resource_groups_t* sc_resource_groups_handle(const sc_resource_groups_collection_t* resource, int i)
{
    const snowcrash::Collection<ResourceGroup>::type* b = AS_CTYPE(snowcrash::Collection<ResourceGroup>::type, resource);
    if (!b)
        return NULL;

    return AS_CTYPE(sc_resource_groups_t, &b->at(i));
}

SC_API const char* sc_resource_groups_name(const sc_resource_groups_t* handle)
{
    const snowcrash::ResourceGroup* b = AS_CTYPE(snowcrash::ResourceGroup, handle);
    if(!b)
        return "";

    return b->name.c_str();
}

SC_API const char* sc_resource_groups_description(const sc_resource_groups_t* handle)
{
    const snowcrash::ResourceGroup* b = AS_CTYPE(snowcrash::ResourceGroup, handle);
    if(!b)
        return "";

    return b->description.c_str();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_resource_collection_t* sc_resource_collection_handle(const sc_resource_groups_t* handle)
{
    const snowcrash::ResourceGroup* b = AS_CTYPE(snowcrash::ResourceGroup, handle);
    if (!b)
        return NULL;

    return AS_CTYPE(sc_resource_collection_t, &b->resources);
}

SC_API const int sc_resource_collection_size(const sc_resource_collection_t* resource)
{
    const snowcrash::Collection<Resource>::type* b = AS_CTYPE(snowcrash::Collection<Resource>::type, resource);
    if (!b)
        return NULL;

    return b->size();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_resource_t* sc_resource_handle(const sc_resource_collection_t* resource , int i)
{
    const snowcrash::Collection<Resource>::type* b = AS_CTYPE(snowcrash::Collection<Resource>::type, resource);
    if (!b)
        return NULL;

    return AS_CTYPE(sc_resource_t, &b->at(i));
}

SC_API const char* sc_resource_uritemplate(const sc_resource_t* handle)
{
    const snowcrash::Resource* b = AS_CTYPE(snowcrash::Resource, handle);
    if(!b)
        return "";

    return b->uriTemplate.c_str();
}

SC_API const char* sc_resource_name(const sc_resource_t* handle)
{
    const snowcrash::Resource* b = AS_CTYPE(snowcrash::Resource, handle);
    if(!b)
        return "";

    return b->name.c_str();
}

SC_API const char* sc_resource_description(const sc_resource_t* handle)
{
    const snowcrash::Resource* b = AS_CTYPE(snowcrash::Resource, handle);
    if(!b)
        return "";

    return b->description.c_str();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_payload_collection_t* sc_payload_collection_handle_requests(const sc_transaction_example_t* handle)
{
    const snowcrash::TransactionExample* b = AS_CTYPE(snowcrash::TransactionExample, handle);
    if (!b)
        return NULL;

    return AS_CTYPE(sc_payload_collection_t, &b->requests);
}

SC_API const sc_payload_collection_t* sc_payload_collection_handle_responses(const sc_transaction_example_t* handle)
{
    const snowcrash::TransactionExample* b = AS_CTYPE(snowcrash::TransactionExample, handle);
    if (!b)
        return NULL;

    return AS_CTYPE(sc_payload_collection_t, &b->responses);
}

SC_API const int sc_payload_collection_size(const sc_payload_collection_t* handle)
{
    const snowcrash::Collection<Payload>::type* b = AS_CTYPE(snowcrash::Collection<Payload>::type, handle);
    if (!b)
        return NULL;

    return b->size();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_payload_t* sc_payload_handle(const sc_payload_collection_t* handle , int i)
{
    const snowcrash::Collection<Payload>::type* b = AS_CTYPE(snowcrash::Collection<Payload>::type, handle);
    if(!b)
        return 0;

    return AS_CTYPE(sc_payload_t, &b->at(i));
}

SC_API const sc_payload_t* sc_payload_handle_resource(const sc_resource_t* handle)
{
    const snowcrash::Resource* b = AS_CTYPE(snowcrash::Resource, handle);
    if(!b)
        return NULL;

    return AS_CTYPE(sc_payload_t , &b->model);
}

SC_API const char* sc_payload_name(const sc_payload_t* handle)
{
    const snowcrash::Payload* b = AS_CTYPE(snowcrash::Payload, handle);
    if(!b)
        return NULL;

    return b->name.c_str();
}

SC_API const char* sc_payload_description(const sc_payload_t* handle)
{
    const snowcrash::Payload* b = AS_CTYPE(snowcrash::Payload, handle);
    if(!b)
        return NULL;

    return b->description.c_str();
}

SC_API const char* sc_payload_body(const sc_payload_t* handle)
{
    const snowcrash::Payload* b = AS_CTYPE(snowcrash::Payload, handle);
    if(!b)
        return NULL;

    return b->body.c_str();
}

SC_API const char* sc_payload_schema(const sc_payload_t* handle)
{
    const snowcrash::Payload* b = AS_CTYPE(snowcrash::Payload, handle);
    if(!b)
        return NULL;

    return b->schema.c_str();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_parameter_collection_t* sc_parameter_collection_handle_payload(const sc_payload_t* handle)
{
    const snowcrash::Payload* b = AS_CTYPE(snowcrash::Payload, handle);
    if(!b)
        return NULL;
    
    return AS_CTYPE(sc_parameter_collection_t, &b->parameters);
}

SC_API const sc_parameter_collection_t* sc_parameter_collection_handle_resource(const sc_resource_t* handle)
{
    const snowcrash::Resource* b = AS_CTYPE(snowcrash::Resource, handle);
    if(!b)
        return NULL;

    return AS_CTYPE(sc_parameter_collection_t, &b->parameters);
}

SC_API const sc_parameter_collection_t* sc_parameter_collection_handle_action(const sc_action_t* handle)
{
    const snowcrash::Action* b = AS_CTYPE(snowcrash::Action, handle);
    if(!b)
        return NULL;

    return AS_CTYPE(sc_parameter_collection_t, &b->parameters);
}

SC_API const int sc_parameter_collection_size(const sc_parameter_collection_t* handle)
{
    const Collection<Parameter>::type* b = AS_CTYPE(Collection<Parameter>::type, handle);
    if(!b)
        return 0;

    return b->size();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_parameter_t* sc_parameter_handle(const sc_parameter_collection_t* handle , int i)
{
    const snowcrash::Collection<Parameter>::type* b = AS_CTYPE(snowcrash::Collection<Parameter>::type, handle);
    if(!b)
        return 0;

    return AS_CTYPE(sc_parameter_t, &b->at(i));
}

SC_API const char* sc_parameter_name(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* b = AS_CTYPE(snowcrash::Parameter, handle);
    if (!b)
        return "";

    return b->name.c_str();
}

SC_API const char* sc_parameter_description(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* b = AS_CTYPE(snowcrash::Parameter, handle);
    if (!b)
        return "";

    return b->description.c_str();
}

SC_API const char* sc_parameter_type(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* b = AS_CTYPE(snowcrash::Parameter, handle);
    if (!b)
        return "";

    return b->type.c_str();
}

SC_API const int sc_parameter_parameter_use(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* b = AS_CTYPE(snowcrash::Parameter, handle);
    if (!b)
        return 0;

    return b->use;
}

SC_API const char* sc_parameter_default_value(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* b = AS_CTYPE(snowcrash::Parameter, handle);
    if (!b)
        return "";

    return b->defaultValue.c_str();
}

SC_API const char* sc_parameter_example_value(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* b = AS_CTYPE(snowcrash::Parameter, handle);
    if (!b)
        return "";

    return b->exampleValue.c_str();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_value_collection_t* sc_value_collection_handle(const sc_parameter_t* handle)
{
    const snowcrash::Parameter* b = AS_CTYPE(snowcrash::Parameter, handle);
    if (!b)
        return NULL;

    return AS_CTYPE(sc_value_collection_t, &b->values);
}

SC_API const int sc_value_collection_size(const sc_value_collection_t* handle)
{
    const snowcrash::Collection<Value>::type* b = AS_CTYPE(snowcrash::Collection<Value>::type, handle);
    if (!b)
        return 0;

    return b->size();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_value_t* sc_value_handle(const sc_value_collection_t* handle , int i)
{
    const snowcrash::Collection<Value>::type* b = AS_CTYPE(snowcrash::Collection<Value>::type, handle);
    if (!b)
        return NULL;

    return AS_CTYPE( sc_value_t, &b->at(i));
}

SC_API const char* sc_value_string(const sc_value_t* handle)
{
    const snowcrash::Value* b = AS_CTYPE(snowcrash::Value, handle);
    if(!b)
        return "";

    return b->c_str();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_header_collection_t* sc_header_collection_handle_payload(const sc_payload_t* handle)
{
    const snowcrash::Payload* b = AS_CTYPE(snowcrash::Payload, handle);
    if (!b)
        return NULL;

    return AS_CTYPE(sc_header_collection_t, &b->headers);
}

SC_API const sc_header_collection_t* sc_header_collection_handle_resource(const sc_resource_t* handle)
{
    const snowcrash::Resource* b = AS_CTYPE(snowcrash::Resource, handle);
    if (!b)
        return NULL;

    return AS_CTYPE(sc_header_collection_t, &b->headers);
}

SC_API const sc_header_collection_t* sc_header_collection_handle_action(const sc_action_t* handle)
{
    const snowcrash::Action* b = AS_CTYPE(snowcrash::Action, handle);
    if (!b)
        return NULL;

    return AS_CTYPE(sc_header_collection_t, &b->headers);
}

SC_API const int sc_header_collection_size(const sc_header_collection_t* handle)
{
    const snowcrash::Collection<Header>::type* b = AS_CTYPE(snowcrash::Collection<Header>::type, handle);
    if (!b)
        return 0;

    return b->size();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_header_t* sc_header_handle(const sc_header_collection_t* handle , int i)
{
    const snowcrash::Collection<Header>::type* b = AS_CTYPE(snowcrash::Collection<Header>::type, handle);
    if (!b)
        return NULL;

    return AS_CTYPE( sc_header_t, &b->at(i));
}

SC_API const char* sc_header_key(const sc_header_t* handle)
{
    const snowcrash::Header* b = AS_CTYPE(snowcrash::Header, handle);
    if(!b)
        return "";

    return b->first.c_str();
}

SC_API const char* sc_header_value(const sc_header_t* handle)
{
    const snowcrash::Header* b = AS_CTYPE(snowcrash::Header, handle);
    if(!b)
        return "";

    return b->second.c_str();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_action_collection_t* sc_action_collection_handle(const sc_resource_t* handle)
{
    const snowcrash::Resource* b = AS_CTYPE(snowcrash::Resource, handle);
    if(!b)
        return NULL;

    return AS_CTYPE(sc_action_collection_t, &b->actions);
}

SC_API const int sc_action_collection_size(const sc_action_collection_t* handle)
{
    const snowcrash::Collection<Action>::type* b = AS_CTYPE(snowcrash::Collection<Action>::type, handle);
    if (!b)
        return 0;

    return b->size();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_action_t* sc_action_handle(const sc_action_collection_t* handle , int i)
{
    const snowcrash::Collection<Action>::type* b = AS_CTYPE(snowcrash::Collection<Action>::type, handle);
    if(!b)
        return 0;

    return AS_CTYPE(sc_action_t, &b->at(i));
}

SC_API const char* sc_action_httpmethod(const sc_action_t* handle)
{
    const snowcrash::Action* b = AS_CTYPE(snowcrash::Action, handle);
    if (!b)
        return "";

    return b->method.c_str();
}

SC_API const char* sc_action_name(const sc_action_t* handle)
{
    const snowcrash::Action* b = AS_CTYPE(snowcrash::Action, handle);
    if (!b)
        return "";

    return b->name.c_str();
}

SC_API const char* sc_action_description(const sc_action_t* handle)
{
    const snowcrash::Action* b = AS_CTYPE(snowcrash::Action, handle);
    if (!b)
        return "";

    return b->description.c_str();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_transaction_example_collection_t* sc_transaction_example_collection_handle(const sc_action_t* handle)
{
    const snowcrash::Action* b = AS_CTYPE(snowcrash::Action, handle);
    if(!b)
        return NULL;

    return AS_CTYPE(sc_transaction_example_collection_t, &b->examples);
}

SC_API const int sc_transaction_example_collection_size(const sc_transaction_example_collection_t* handle)
{
    const snowcrash::Collection<TransactionExample>::type* b = AS_CTYPE(snowcrash::Collection<TransactionExample>::type, handle);
    if (!b)
        return 0;

    return b->size();
}

//////////////////////////////////////////////////////////////////////////

SC_API const sc_transaction_example_t* sc_transaction_example_handle(const sc_transaction_example_collection_t* handle , int i)
{
    const snowcrash::Collection<TransactionExample>::type* b = AS_CTYPE(snowcrash::Collection<TransactionExample>::type, handle);
    if(!b)
        return 0;

    return AS_CTYPE(sc_transaction_example_t, &b->at(i));
}

SC_API const char* sc_transaction_example_name(const sc_transaction_example_t* handle)
{
    const snowcrash::TransactionExample* b = AS_CTYPE(snowcrash::TransactionExample, handle);
    if (!b)
        return "";

    return b->name.c_str();
}

SC_API const char* sc_transaction_example_description(const sc_transaction_example_t* handle)
{
    const snowcrash::TransactionExample* b = AS_CTYPE(snowcrash::TransactionExample, handle);
    if (!b)
        return "";

    return b->description.c_str();
}

//////////////////////////////////////////////////////////////////////////
