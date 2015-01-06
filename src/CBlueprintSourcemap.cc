//
//  CBlueprintSourcemap.cc
//  snowcrash
//  C Implementation of BlueprintSourcemap.h for binding purposes
//
//  Created by Pavan Kumar Sunkara on 15/9/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "CBlueprintSourcemap.h"
#include "snowcrash.h"

/*----------------------------------------------------------------------*/

SC_API sc_sm_blueprint_t* sc_sm_blueprint_new()
{
    return AS_TYPE(sc_sm_blueprint_t, ::new snowcrash::SourceMap<snowcrash::Blueprint>);
}

SC_API void sc_sm_blueprint_free(sc_sm_blueprint_t* blueprint)
{
    ::delete AS_TYPE(snowcrash::SourceMap<snowcrash::Blueprint>, blueprint);
}

SC_API const sc_source_map_t* sc_sm_blueprint_name(const sc_sm_blueprint_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Blueprint>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Blueprint>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->name.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_blueprint_description(const sc_sm_blueprint_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Blueprint>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Blueprint>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->description.sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_metadata_collection_t* sc_sm_metadata_collection_handle(const sc_sm_blueprint_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Blueprint>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Blueprint>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_sm_metadata_collection_t, &p->metadata);
}

SC_API size_t sc_sm_metadata_collection_size(const sc_sm_metadata_collection_t* handle)
{
    const snowcrash::SourceMap<snowcrash::MetadataCollection>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::MetadataCollection>, handle);
    if (!p)
        return 0;

    return p->collection.size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_metadata_t* sc_sm_metadata_handle(const sc_sm_metadata_collection_t* handle, size_t index)
{
    const snowcrash::SourceMap<snowcrash::MetadataCollection>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::MetadataCollection>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_sm_metadata_t, &p->collection.at(index));
}

SC_API const sc_source_map_t* sc_sm_metadata(const sc_sm_metadata_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Metadata>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Metadata>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_resource_group_collection_t* sc_sm_resource_group_collection_handle(const sc_sm_blueprint_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Blueprint>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Blueprint>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_sm_resource_group_collection_t, &p->resourceGroups);
}

SC_API size_t sc_sm_resource_group_collection_size(const sc_sm_resource_group_collection_t* handle)
{
    const snowcrash::SourceMap<snowcrash::ResourceGroups>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::ResourceGroups>, handle);
    if (!p)
        return 0;

    return p->collection.size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_resource_group_t* sc_sm_resource_group_handle(const sc_sm_resource_group_collection_t* handle, size_t index)
{
    const snowcrash::SourceMap<snowcrash::ResourceGroups>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::ResourceGroups>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_sm_resource_group_t, &p->collection.at(index));
}

SC_API const sc_source_map_t* sc_sm_resource_group_name(const sc_sm_resource_group_t* handle)
{
    const snowcrash::SourceMap<snowcrash::ResourceGroup>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::ResourceGroup>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->name.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_resource_group_description(const sc_sm_resource_group_t* handle)
{
    const snowcrash::SourceMap<snowcrash::ResourceGroup>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::ResourceGroup>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->description.sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_resource_collection_t* sc_sm_resource_collection_handle(const sc_sm_resource_group_t* handle)
{
    const snowcrash::SourceMap<snowcrash::ResourceGroup>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::ResourceGroup>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_sm_resource_collection_t, &p->resources);
}

SC_API size_t sc_sm_resource_collection_size(const sc_sm_resource_collection_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Resources>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Resources>, handle);
    if (!p)
        return 0;

    return p->collection.size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_resource_t* sc_sm_resource_handle(const sc_sm_resource_collection_t* handle, size_t index)
{
    const snowcrash::SourceMap<snowcrash::Resources>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Resources>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_sm_resource_t, &p->collection.at(index));
}

SC_API const sc_source_map_t* sc_sm_resource_uritemplate(const sc_sm_resource_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Resource>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Resource>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->uriTemplate.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_resource_name(const sc_sm_resource_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Resource>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Resource>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->name.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_resource_description(const sc_sm_resource_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Resource>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Resource>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->description.sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_payload_collection_t* sc_sm_payload_collection_handle_requests(const sc_sm_transaction_example_t* handle)
{
    const snowcrash::SourceMap<snowcrash::TransactionExample>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::TransactionExample>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_sm_payload_collection_t, &p->requests);
}

SC_API const sc_sm_payload_collection_t* sc_sm_payload_collection_handle_responses(const sc_sm_transaction_example_t* handle)
{
    const snowcrash::SourceMap<snowcrash::TransactionExample>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::TransactionExample>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_sm_payload_collection_t, &p->responses);
}

SC_API size_t sc_sm_payload_collection_size(const sc_sm_payload_collection_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Requests>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Requests>, handle);
    if (!p)
        return 0;

    return p->collection.size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_payload_t* sc_sm_payload_handle(const sc_sm_payload_collection_t* handle, size_t index)
{
    const snowcrash::SourceMap<snowcrash::Requests>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Requests>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_sm_payload_t, &p->collection.at(index));
}

SC_API const sc_sm_payload_t* sc_sm_payload_handle_resource(const sc_sm_resource_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Resource>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Resource>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_sm_payload_t, &p->model);
}

SC_API const sc_source_map_t* sc_sm_payload_name(const sc_sm_payload_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Payload>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Payload>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->name.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_payload_description(const sc_sm_payload_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Payload>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Payload>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->description.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_payload_body(const sc_sm_payload_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Payload>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Payload>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->body.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_payload_schema(const sc_sm_payload_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Payload>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Payload>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->schema.sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_reference_t* sc_sm_reference_handle(const sc_sm_payload_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Payload>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Payload>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_sm_reference_t, &p->reference);
}

SC_API const sc_source_map_t* sc_sm_reference(const sc_sm_reference_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Reference>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Reference>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_parameter_collection_t* sc_sm_parameter_collection_handle_payload(const sc_sm_payload_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Payload>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Payload>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_sm_parameter_collection_t, &p->parameters);
}

SC_API const sc_sm_parameter_collection_t* sc_sm_parameter_collection_handle_resource(const sc_sm_resource_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Resource>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Resource>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_sm_parameter_collection_t, &p->parameters);
}

SC_API const sc_sm_parameter_collection_t* sc_sm_parameter_collection_handle_action(const sc_sm_action_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Action>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Action>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_sm_parameter_collection_t, &p->parameters);
}

SC_API size_t sc_sm_parameter_collection_size(const sc_sm_parameter_collection_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Parameters>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Parameters>, handle);
    if(!p)
        return 0;

    return p->collection.size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_parameter_t* sc_sm_parameter_handle(const sc_sm_parameter_collection_t* handle, size_t index)
{
    const snowcrash::SourceMap<snowcrash::Parameters>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Parameters>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_sm_parameter_t, &p->collection.at(index));
}

SC_API const sc_source_map_t* sc_sm_parameter_name(const sc_sm_parameter_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Parameter>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Parameter>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->name.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_parameter_description(const sc_sm_parameter_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Parameter>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Parameter>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->description.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_parameter_type(const sc_sm_parameter_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Parameter>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Parameter>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->type.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_parameter_parameter_use(const sc_sm_parameter_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Parameter>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Parameter>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->use.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_parameter_default_value(const sc_sm_parameter_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Parameter>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Parameter>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->defaultValue.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_parameter_example_value(const sc_sm_parameter_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Parameter>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Parameter>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->exampleValue.sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_value_collection_t* sc_sm_value_collection_handle(const sc_sm_parameter_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Parameter>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Parameter>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_sm_value_collection_t, &p->values);
}

SC_API size_t sc_sm_value_collection_size(const sc_sm_value_collection_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Values>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Values>, handle);
    if (!p)
        return 0;

    return p->collection.size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_value_t* sc_sm_value_handle(const sc_sm_value_collection_t* handle, size_t index)
{
    const snowcrash::SourceMap<snowcrash::Values>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Values>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_sm_value_t, &p->collection.at(index));
}

SC_API const sc_source_map_t* sc_sm_value(const sc_sm_value_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Value>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Value>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_header_collection_t* sc_sm_header_collection_handle_payload(const sc_sm_payload_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Payload>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Payload>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_sm_header_collection_t, &p->headers);
}

SC_API const sc_sm_header_collection_t* sc_sm_header_collection_handle_resource(const sc_sm_resource_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Resource>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Resource>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_sm_header_collection_t, &p->headers);
}

SC_API const sc_sm_header_collection_t* sc_sm_header_collection_handle_action(const sc_sm_action_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Action>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Action>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_sm_header_collection_t, &p->headers);
}

SC_API size_t sc_sm_header_collection_size(const sc_sm_header_collection_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Headers>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Headers>, handle);
    if (!p)
        return 0;

    return p->collection.size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_header_t* sc_sm_header_handle(const sc_sm_header_collection_t* handle, size_t index)
{
    const snowcrash::SourceMap<snowcrash::Headers>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Headers>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE( sc_sm_header_t, &p->collection.at(index));
}

SC_API const sc_source_map_t* sc_sm_header(const sc_sm_header_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Header>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Header>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_action_collection_t* sc_sm_action_collection_handle(const sc_sm_resource_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Resource>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Resource>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_sm_action_collection_t, &p->actions);
}

SC_API size_t sc_sm_action_collection_size(const sc_sm_action_collection_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Actions>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Actions>, handle);
    if (!p)
        return 0;

    return p->collection.size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_action_t* sc_sm_action_handle(const sc_sm_action_collection_t* handle, size_t index)
{
    const snowcrash::SourceMap<snowcrash::Actions>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Actions>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_sm_action_t, &p->collection.at(index));
}

SC_API const sc_source_map_t* sc_sm_action_httpmethod(const sc_sm_action_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Action>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Action>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->method.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_action_name(const sc_sm_action_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Action>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Action>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->name.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_action_description(const sc_sm_action_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Action>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Action>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->description.sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_transaction_example_collection_t* sc_sm_transaction_example_collection_handle(const sc_sm_action_t* handle)
{
    const snowcrash::SourceMap<snowcrash::Action>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::Action>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_sm_transaction_example_collection_t, &p->examples);
}

SC_API size_t sc_sm_transaction_example_collection_size(const sc_sm_transaction_example_collection_t* handle)
{
    const snowcrash::SourceMap<snowcrash::TransactionExamples>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::TransactionExamples>, handle);
    if (!p)
        return 0;

    return p->collection.size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_sm_transaction_example_t* sc_sm_transaction_example_handle(const sc_sm_transaction_example_collection_t* handle, size_t index)
{
    const snowcrash::SourceMap<snowcrash::TransactionExamples>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::TransactionExamples>, handle);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_sm_transaction_example_t, &p->collection.at(index));
}

SC_API const sc_source_map_t* sc_sm_transaction_example_name(const sc_sm_transaction_example_t* handle)
{
    const snowcrash::SourceMap<snowcrash::TransactionExample>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::TransactionExample>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->name.sourceMap);
}

SC_API const sc_source_map_t* sc_sm_transaction_example_description(const sc_sm_transaction_example_t* handle)
{
    const snowcrash::SourceMap<snowcrash::TransactionExample>* p = AS_CTYPE(snowcrash::SourceMap<snowcrash::TransactionExample>, handle);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_source_map_t, &p->description.sourceMap);
}

/*----------------------------------------------------------------------*/
