//
//  SerializeSourcemap.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 18/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "SerializeSourcemap.h"

using namespace snowcrash;

sos::Array wrapSourcemap(const SourceMapBase& value)
{
    sos::Array sourceMap = sos::Array();

    for (mdp::RangeSet<mdp::BytesRange>::const_iterator it = value.sourceMap.begin();
         it != value.sourceMap.end();
         ++it) {

        sos::Array sourceMapRow = sos::Array();

        sourceMapRow.push(sos::Number(it->location));
        sourceMapRow.push(sos::Number(it->length));

        sourceMap.push(sourceMapRow);
    }

    return sourceMap;
}

sos::Object wrapPayloadSourcemap(const SourceMap<Payload>& payload)
{
    sos::Object payloadObject = sos::Object();

    // Name
    payloadObject.set(SerializeKey::Name.c_str(), wrapSourcemap(payload.name));

    // Description
    payloadObject.set(SerializeKey::Description.c_str(), wrapSourcemap(payload.description));

    // Reference
    if (!payload.reference.sourceMap.empty()) {
        payloadObject.set(SerializeKey::Reference.c_str(), wrapSourcemap(payload.reference));
    }

    // Headers
    sos::Array headers = sos::Array();

    for (Collection<SourceMap<Header> >::const_iterator it = payload.headers.collection.begin();
         it != payload.headers.collection.end();
         ++it) {

        headers.push(wrapSourcemap(*it));
    }

    payloadObject.set(SerializeKey::Headers.c_str(), headers);

    // Body
    payloadObject.set(SerializeKey::Body.c_str(), wrapSourcemap(payload.body));

    // Schema
    payloadObject.set(SerializeKey::Schema.c_str(), wrapSourcemap(payload.schema));

    return payloadObject;
}

sos::Array wrapParametersSourcemap(const SourceMap<Parameters>& parameters)
{
    sos::Array parametersArray = sos::Array();

    for (Collection<SourceMap<Parameter> >::const_iterator it = parameters.collection.begin();
         it != parameters.collection.end();
         ++it) {

        sos::Object parameter = sos::Object();

        // Name
        parameter.set(SerializeKey::Name.c_str(), wrapSourcemap(it->name));

        // Description
        parameter.set(SerializeKey::Description.c_str(), wrapSourcemap(it->description));

        // Type
        parameter.set(SerializeKey::Type.c_str(), wrapSourcemap(it->type));

        // Use
        parameter.set(SerializeKey::Required.c_str(), wrapSourcemap(it->use));

        // Example Value
        parameter.set(SerializeKey::Example.c_str(), wrapSourcemap(it->exampleValue));

        // Default Value
        parameter.set(SerializeKey::Default.c_str(), wrapSourcemap(it->defaultValue));

        // Values
        sos::Array values = sos::Array();

        for (Collection<SourceMap<Value> >::const_iterator valIt = it->values.collection.begin();
             valIt != it->values.collection.end();
             ++valIt) {

            sos::Object value = sos::Object();

            value.set(SerializeKey::Value.c_str(), wrapSourcemap(*valIt));

            values.push(value);
        }

        parameter.set(SerializeKey::Values.c_str(), values);
    }

    return parametersArray;
}

sos::Object wrapTransactionExampleSourcemap(const SourceMap<TransactionExample>& example)
{
    sos::Object exampleObject = sos::Object();

    // Name
    exampleObject.set(SerializeKey::Name.c_str(), wrapSourcemap(example.name));

    // Description
    exampleObject.set(SerializeKey::Description.c_str(), wrapSourcemap(example.description));

    // Requests
    sos::Array requests = sos::Array();

    for (Collection<SourceMap<Request> >::const_iterator it = example.requests.collection.begin();
         it != example.requests.collection.end();
         ++it) {

        requests.push(wrapPayloadSourcemap(*it));
    }

    exampleObject.set(SerializeKey::Requests.c_str(), requests);

    // Responses
    sos::Array responses = sos::Array();

    for (Collection<SourceMap<Response> >::const_iterator it = example.responses.collection.begin();
         it != example.responses.collection.end();
         ++it) {

        responses.push(wrapPayloadSourcemap(*it));
    }

    exampleObject.set(SerializeKey::Responses.c_str(), responses);

    return exampleObject;
}

sos::Object wrapActionSourcemap(const SourceMap<Action>& action)
{
    sos::Object actionObject = sos::Object();

    // Name
    actionObject.set(SerializeKey::Name.c_str(), wrapSourcemap(action.name));

    // Description
    actionObject.set(SerializeKey::Description.c_str(), wrapSourcemap(action.description));

    // HTTP Method
    actionObject.set(SerializeKey::Method.c_str(), wrapSourcemap(action.method));

    // Parameters
    actionObject.set(SerializeKey::Parameters.c_str(), wrapParametersSourcemap(action.parameters));

    // Transaction Examples
    sos::Array transactionExamples = sos::Array();

    for (Collection<SourceMap<TransactionExample> >::const_iterator it = action.examples.collection.begin();
         it != action.examples.collection.end();
         ++it) {
        
        transactionExamples.push(wrapTransactionExampleSourcemap(*it));
    }
    
    actionObject.set(SerializeKey::Examples.c_str(), transactionExamples);
    
    return actionObject;
}

sos::Object wrapResourceSourcemap(const SourceMap<Resource>& resource)
{
    sos::Object resourceObject = sos::Object();

    // Name
    resourceObject.set(SerializeKey::Name.c_str(), wrapSourcemap(resource.name));

    // Description
    resourceObject.set(SerializeKey::Description.c_str(), wrapSourcemap(resource.description));

    // URI Template
    resourceObject.set(SerializeKey::URITemplate.c_str(), wrapSourcemap(resource.uriTemplate));

    // Model
    sos::Object model = (resource.model.name.sourceMap.empty() ? sos::Object() : wrapPayloadSourcemap(resource.model));
    resourceObject.set(SerializeKey::Model.c_str(), model);

    // Parameters
    resourceObject.set(SerializeKey::Parameters.c_str(), wrapParametersSourcemap(resource.parameters));

    // Actions
    sos::Array actions = sos::Array();

    for (Collection<SourceMap<Action> >::const_iterator it = resource.actions.collection.begin();
         it != resource.actions.collection.end();
         ++it) {

        actions.push(wrapActionSourcemap(*it));
    }

    resourceObject.set(SerializeKey::Actions.c_str(), actions);
    
    return resourceObject;
}

sos::Object wrapResourceGroupSourcemap(const SourceMap<ResourceGroup>& resourceGroup)
{
    sos::Object resourceGroupObject = sos::Object();

    // Name
    resourceGroupObject.set(SerializeKey::Name.c_str(), wrapSourcemap(resourceGroup.name));

    // Description
    resourceGroupObject.set(SerializeKey::Description.c_str(), wrapSourcemap(resourceGroup.description));

    // Resources
    sos::Array resources = sos::Array();

    for (Collection<SourceMap<Resource> >::const_iterator it = resourceGroup.resources.collection.begin();
         it != resourceGroup.resources.collection.end();
         ++it) {

        resources.push(wrapResourceSourcemap(*it));
    }

    return resourceGroupObject;
}

sos::Object wrapBlueprintSourcemap(const SourceMap<Blueprint>& blueprint)
{
    sos::Object blueprintObject = sos::Object();

    // Metadata
    sos::Array metadata = sos::Array();

    for (Collection<SourceMap<Metadata> >::const_iterator it = blueprint.metadata.collection.begin();
         it != blueprint.metadata.collection.end();
         ++it) {

        metadata.push(wrapSourcemap(*it));
    }

    blueprintObject.set(SerializeKey::Metadata.c_str(), metadata);

    // Name
    blueprintObject.set(SerializeKey::Name.c_str(), wrapSourcemap(blueprint.name));

    // Description
    blueprintObject.set(SerializeKey::Description.c_str(), wrapSourcemap(blueprint.description));

    // Resource Groups
    sos::Array resourceGroups = sos::Array();

    for (Collection<SourceMap<ResourceGroup> >::const_iterator it = blueprint.resourceGroups.collection.begin();
         it != blueprint.resourceGroups.collection.end();
         ++it) {

        resourceGroups.push(wrapResourceGroupSourcemap(*it));
    }

    blueprintObject.set(SerializeKey::ResourceGroups.c_str(), resourceGroups);

    return blueprintObject;
}