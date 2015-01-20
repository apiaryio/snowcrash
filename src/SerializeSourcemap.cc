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

// Forward declarations
sos::Array wrapTypeSectionsSourcemap(const SourceMap<mson::TypeSections>& typeSections);
sos::Array wrapElementsSourcemap(const SourceMap<mson::Elements>& elements);

sos::Object wrapPropertyMemberSourcemap(const SourceMap<mson::PropertyMember>& propertyMember)
{
    sos::Object propertyMemberObject = sos::Object();

    // Name
    propertyMemberObject.set(SerializeKey::Name.c_str(), wrapSourcemap(propertyMember.name));

    // Description
    propertyMemberObject.set(SerializeKey::Description.c_str(), wrapSourcemap(propertyMember.description));

    // Value Definition
    propertyMemberObject.set(SerializeKey::ValueDefinition.c_str(), wrapSourcemap(propertyMember.valueDefinition));

    // Type Sections
    propertyMemberObject.set(SerializeKey::Sections.c_str(), wrapTypeSectionsSourcemap(propertyMember.sections));

    return propertyMemberObject;
}

sos::Object wrapValueMemberSourcemap(const SourceMap<mson::ValueMember>& valueMember)
{
    sos::Object valueMemberObject = sos::Object();

    // Description
    valueMemberObject.set(SerializeKey::Description.c_str(), wrapSourcemap(valueMember.description));

    // Value Definition
    valueMemberObject.set(SerializeKey::ValueDefinition.c_str(), wrapSourcemap(valueMember.valueDefinition));

    // Type Sections
    valueMemberObject.set(SerializeKey::Sections.c_str(), wrapTypeSectionsSourcemap(valueMember.sections));

    return valueMemberObject;
}

sos::Array wrapMixinSourcemap(const SourceMap<mson::Mixin>& mixin)
{
    return wrapSourcemap(mixin);
}

sos::Array wrapOneOf(const SourceMap<mson::OneOf>& oneOf)
{
    return wrapElementsSourcemap(oneOf);
}

sos::Base wrapElementSourcemap(const SourceMap<mson::Element>& element)
{
    if (!element.elements().collection.empty()) {
        // Same for oneOf
        return wrapElementsSourcemap(element.elements());
    }
    else if (!element.mixin.sourceMap.empty()) {
        return wrapMixinSourcemap(element.mixin);
    }
    else if (!element.value.empty()) {
        return wrapValueMemberSourcemap(element.value);
    }
    else if (!element.property.empty()) {
        return wrapPropertyMemberSourcemap(element.property);
    }

    return sos::Null();
}

sos::Array wrapElementsSourcemap(const SourceMap<mson::Elements>& elements)
{
    sos::Array elementsArray = sos::Array();

    for (Collection<SourceMap<mson::Element> >::const_iterator it = elements.collection.begin();
         it != elements.collection.end();
         ++it) {

        elementsArray.push(wrapElementSourcemap(*it));
    }

    return elementsArray;
}

sos::Array wrapTypeSectionsSourcemap(const SourceMap<mson::TypeSections>& sections)
{
    sos::Array sectionsArray = sos::Array();

    for (Collection<SourceMap<mson::TypeSection> >::const_iterator it = sections.collection.begin();
         it != sections.collection.end();
         ++it) {

        if (!it->description.sourceMap.empty()) {
            sectionsArray.push(wrapSourcemap(it->description));
        }
        else if (!it->value.sourceMap.empty()) {
            sectionsArray.push(wrapSourcemap(it->value));
        }
        else if (!it->elements().collection.empty()) {
            sectionsArray.push(wrapElementsSourcemap(it->elements()));
        }
    }

    return sectionsArray;
}

sos::Object wrapNamedTypeSourcemap(const SourceMap<mson::NamedType>& namedType)
{
    sos::Object namedTypeObject = sos::Object();

    // Name
    namedTypeObject.set(SerializeKey::Name.c_str(), wrapSourcemap(namedType.name));

    // Type Definition
    namedTypeObject.set(SerializeKey::TypeDefinition.c_str(), wrapSourcemap(namedType.typeDefinition));

    // Type Sections
    namedTypeObject.set(SerializeKey::Sections.c_str(), wrapTypeSectionsSourcemap(namedType.sections));

    return namedTypeObject;
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

sos::Object snowcrash::wrapBlueprintSourcemap(const SourceMap<Blueprint>& blueprint)
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
