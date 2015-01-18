//
//  SerializeAST.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 18/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "SerializeAST.h"

using namespace snowcrash;

sos::Object wrapKeyValue(const KeyValuePair& keyValue)
{
    sos::Object keyValueObject = sos::Object();

    // Name
    keyValueObject.set(SerializeKey::Name.c_str(), sos::String(keyValue.first.c_str()));

    // Value
    keyValueObject.set(SerializeKey::Value.c_str(), sos::String(keyValue.second.c_str()));

    return keyValueObject;
}

sos::Object wrapMetadata(const Metadata& metadata)
{
    return wrapKeyValue(metadata);
}

sos::Object wrapHeader(const Header& header)
{
    return wrapKeyValue(header);
}

sos::Object wrapReference(const Reference& reference)
{
    sos::Object referenceObject = sos::Object();

    // Id
    referenceObject.set(SerializeKey::Id.c_str(), sos::String(reference.id.c_str()));

    return referenceObject;
}

sos::Object wrapPayload(const Payload& payload)
{
    sos::Object payloadObject = sos::Object();

    // Name
    payloadObject.set(SerializeKey::Name.c_str(), sos::String(payload.name.c_str()));

    // Description
    payloadObject.set(SerializeKey::Description.c_str(), sos::String(payload.description.c_str()));

    // Reference
    if (!payload.reference.id.empty()) {
        payloadObject.set(SerializeKey::Reference.c_str(), wrapReference(payload.reference));
    }

    // Headers
    sos::Array headers = sos::Array();

    for (Headers::const_iterator it = payload.headers.begin();
         it != payload.headers.end();
         ++it) {

        headers.push(wrapHeader(*it));
    }

    payloadObject.set(SerializeKey::Headers.c_str(), headers);

    // Body
    payloadObject.set(SerializeKey::Body.c_str(), sos::String(payload.body.c_str()));

    // Schema
    payloadObject.set(SerializeKey::Schema.c_str(), sos::String(payload.schema.c_str()));

    return payloadObject;
}

sos::Array wrapParameters(const Parameters& parameters)
{
    sos::Array parametersArray = sos::Array();

    for (Parameters::const_iterator it = parameters.begin(); it != parameters.end(); ++it) {

        sos::Object parameter = sos::Object();

        // Name
        parameter.set(SerializeKey::Name.c_str(), sos::String(it->name.c_str()));

        // Description
        parameter.set(SerializeKey::Description.c_str(), sos::String(it->description.c_str()));

        // Type
        parameter.set(SerializeKey::Type.c_str(), sos::String(it->type.c_str()));

        // Use
        parameter.set(SerializeKey::Required.c_str(), sos::Boolean(it->use != snowcrash::OptionalParameterUse));

        // Example Value
        parameter.set(SerializeKey::Example.c_str(), sos::String(it->exampleValue.c_str()));

        // Default Value
        parameter.set(SerializeKey::Default.c_str(), sos::String(it->defaultValue.c_str()));

        // Values
        sos::Array values = sos::Array();

        for (Values::const_iterator valIt = it->values.begin(); valIt != it->values.end(); ++valIt) {

            sos::Object value = sos::Object();

            value.set(SerializeKey::Value.c_str(), sos::String(valIt->c_str()));

            values.push(value);
        }

        parameter.set(SerializeKey::Values.c_str(), values);
    }

    return parametersArray;
}

sos::Object wrapTransactionExample(const TransactionExample& example)
{
    sos::Object exampleObject = sos::Object();

    // Name
    exampleObject.set(SerializeKey::Name.c_str(), sos::String(example.name.c_str()));

    // Description
    exampleObject.set(SerializeKey::Description.c_str(), sos::String(example.description.c_str()));

    // Requests
    sos::Array requests = sos::Array();

    for (Requests::const_iterator it = example.requests.begin();
         it != example.requests.end();
         ++it) {

        requests.push(wrapPayload(*it));
    }

    exampleObject.set(SerializeKey::Requests.c_str(), requests);

    // Responses
    sos::Array responses = sos::Array();

    for (Responses::const_iterator it = example.responses.begin();
         it != example.responses.end();
         ++it) {

        responses.push(wrapPayload(*it));
    }

    exampleObject.set(SerializeKey::Responses.c_str(), responses);

    return exampleObject;
}

sos::Object wrapAction(const Action& action)
{
    sos::Object actionObject = sos::Object();

    // Name
    actionObject.set(SerializeKey::Name.c_str(), sos::String(action.name.c_str()));

    // Description
    actionObject.set(SerializeKey::Description.c_str(), sos::String(action.description.c_str()));

    // HTTP Method
    actionObject.set(SerializeKey::Method.c_str(), sos::String(action.method.c_str()));

    // Parameters
    actionObject.set(SerializeKey::Parameters.c_str(), wrapParameters(action.parameters));

    // Transaction Examples
    sos::Array transactionExamples = sos::Array();

    for (TransactionExamples::const_iterator it = action.examples.begin();
         it != action.examples.end();
         ++it) {

        transactionExamples.push(wrapTransactionExample(*it));
    }

    actionObject.set(SerializeKey::Examples.c_str(), transactionExamples);

    return actionObject;
}

sos::Object wrapResource(const Resource& resource)
{
    sos::Object resourceObject = sos::Object();

    // Name
    resourceObject.set(SerializeKey::Name.c_str(), sos::String(resource.name.c_str()));

    // Description
    resourceObject.set(SerializeKey::Description.c_str(), sos::String(resource.description.c_str()));

    // URI Template
    resourceObject.set(SerializeKey::URITemplate.c_str(), sos::String(resource.uriTemplate.c_str()));

    // Model
    sos::Object model = (resource.model.name.empty() ? sos::Object() : wrapPayload(resource.model));
    resourceObject.set(SerializeKey::Model.c_str(), model);

    // Parameters
    resourceObject.set(SerializeKey::Parameters.c_str(), wrapParameters(resource.parameters));

    // Actions
    sos::Array actions = sos::Array();

    for (Actions::const_iterator it = resource.actions.begin();
         it != resource.actions.end();
         ++it) {

        actions.push(wrapAction(*it));
    }

    resourceObject.set(SerializeKey::Actions.c_str(), actions);

    return resourceObject;
}

sos::Object wrapResourceGroup(const ResourceGroup& resourceGroup)
{
    sos::Object resourceGroupObject = sos::Object();

    // Name
    resourceGroupObject.set(SerializeKey::Name.c_str(), sos::String(resourceGroup.name.c_str()));

    // Description
    resourceGroupObject.set(SerializeKey::Description.c_str(), sos::String(resourceGroup.description.c_str()));

    // Resources
    sos::Array resources = sos::Array();

    for (Resources::const_iterator it = resourceGroup.resources.begin();
         it != resourceGroup.resources.end();
         ++it) {

        resources.push(wrapResource(*it));
    }

    return resourceGroupObject;
}

sos::Object wrapBlueprint(const Blueprint& blueprint)
{
    sos::Object blueprintObject = sos::Object();

    // Metadata
    sos::Array metadata = sos::Array();

    for (MetadataCollection::const_iterator it = blueprint.metadata.begin();
         it != blueprint.metadata.end();
         ++it) {

        metadata.push(wrapMetadata(*it));
    }

    blueprintObject.set(SerializeKey::Metadata.c_str(), metadata);

    // Name
    blueprintObject.set(SerializeKey::Name.c_str(), sos::String(blueprint.name.c_str()));

    // Description
    blueprintObject.set(SerializeKey::Description.c_str(), sos::String(blueprint.description.c_str()));

    // Resource Groups
    sos::Array resourceGroups = sos::Array();

    for (ResourceGroups::const_iterator it = blueprint.resourceGroups.begin();
         it != blueprint.resourceGroups.end();
         ++it) {

        resourceGroups.push(wrapResourceGroup(*it));
    }

    blueprintObject.set(SerializeKey::ResourceGroups.c_str(), resourceGroups);

    return blueprintObject;
}