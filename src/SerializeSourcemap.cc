//
//  SerializeSourcemap.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 18/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "SerializeSourcemap.h"

using namespace snowcrash;

sos::Array WrapSourcemap(const SourceMapBase& value)
{
    sos::Array sourceMap;

    for (mdp::RangeSet<mdp::BytesRange>::const_iterator it = value.sourceMap.begin();
         it != value.sourceMap.end();
         ++it) {

        sos::Array sourceMapRow;

        sourceMapRow.push(sos::Number(it->location));
        sourceMapRow.push(sos::Number(it->length));

        sourceMap.push(sourceMapRow);
    }

    return sourceMap;
}

// Forward declarations
sos::Array WrapTypeSectionsSourcemap(const SourceMap<mson::TypeSections>& typeSections);
sos::Array WrapElementsSourcemap(const SourceMap<mson::Elements>& elements);

sos::Object WrapPropertyMemberSourcemap(const SourceMap<mson::PropertyMember>& propertyMember)
{
    sos::Object propertyMemberObject;

    // Name
    propertyMemberObject.set(SerializeKey::Name, WrapSourcemap(propertyMember.name));

    // Description
    propertyMemberObject.set(SerializeKey::Description, WrapSourcemap(propertyMember.description));

    // Value Definition
    propertyMemberObject.set(SerializeKey::ValueDefinition, WrapSourcemap(propertyMember.valueDefinition));

    // Type Sections
    propertyMemberObject.set(SerializeKey::Sections, WrapTypeSectionsSourcemap(propertyMember.sections));

    return propertyMemberObject;
}

sos::Object WrapValueMemberSourcemap(const SourceMap<mson::ValueMember>& valueMember)
{
    sos::Object valueMemberObject;

    // Description
    valueMemberObject.set(SerializeKey::Description, WrapSourcemap(valueMember.description));

    // Value Definition
    valueMemberObject.set(SerializeKey::ValueDefinition, WrapSourcemap(valueMember.valueDefinition));

    // Type Sections
    valueMemberObject.set(SerializeKey::Sections, WrapTypeSectionsSourcemap(valueMember.sections));

    return valueMemberObject;
}

sos::Array WrapMixinSourcemap(const SourceMap<mson::Mixin>& mixin)
{
    return WrapSourcemap(mixin);
}

sos::Array WrapOneOf(const SourceMap<mson::OneOf>& oneOf)
{
    return WrapElementsSourcemap(oneOf);
}

sos::Base WrapElementSourcemap(const SourceMap<mson::Element>& element)
{
    if (!element.elements().collection.empty()) {
        // Same for oneOf
        return WrapElementsSourcemap(element.elements());
    }
    else if (!element.mixin.sourceMap.empty()) {
        return WrapMixinSourcemap(element.mixin);
    }
    else if (!element.value.empty()) {
        return WrapValueMemberSourcemap(element.value);
    }
    else if (!element.property.empty()) {
        return WrapPropertyMemberSourcemap(element.property);
    }

    return sos::Null();
}

sos::Array WrapElementsSourcemap(const SourceMap<mson::Elements>& elements)
{
    sos::Array elementsArray;

    for (Collection<SourceMap<mson::Element> >::const_iterator it = elements.collection.begin();
         it != elements.collection.end();
         ++it) {

        elementsArray.push(WrapElementSourcemap(*it));
    }

    return elementsArray;
}

sos::Array WrapTypeSectionsSourcemap(const SourceMap<mson::TypeSections>& sections)
{
    sos::Array sectionsArray;

    for (Collection<SourceMap<mson::TypeSection> >::const_iterator it = sections.collection.begin();
         it != sections.collection.end();
         ++it) {

        if (!it->description.sourceMap.empty()) {
            sectionsArray.push(WrapSourcemap(it->description));
        }
        else if (!it->value.sourceMap.empty()) {
            sectionsArray.push(WrapSourcemap(it->value));
        }
        else if (!it->elements().collection.empty()) {
            sectionsArray.push(WrapElementsSourcemap(it->elements()));
        }
    }

    return sectionsArray;
}

sos::Object WrapNamedTypeSourcemap(const SourceMap<mson::NamedType>& namedType)
{
    sos::Object namedTypeObject;

    // Name
    namedTypeObject.set(SerializeKey::Name, WrapSourcemap(namedType.name));

    // Type Definition
    namedTypeObject.set(SerializeKey::TypeDefinition, WrapSourcemap(namedType.typeDefinition));

    // Type Sections
    namedTypeObject.set(SerializeKey::Sections, WrapTypeSectionsSourcemap(namedType.sections));

    return namedTypeObject;
}

sos::Object WrapPayloadSourcemap(const SourceMap<Payload>& payload)
{
    sos::Object payloadObject;

    // Name
    payloadObject.set(SerializeKey::Name, WrapSourcemap(payload.name));

    // Description
    payloadObject.set(SerializeKey::Description, WrapSourcemap(payload.description));

    // Reference
    if (!payload.reference.sourceMap.empty()) {
        payloadObject.set(SerializeKey::Reference, WrapSourcemap(payload.reference));
    }

    // Headers
    sos::Array headers;

    for (Collection<SourceMap<Header> >::const_iterator it = payload.headers.collection.begin();
         it != payload.headers.collection.end();
         ++it) {

        headers.push(WrapSourcemap(*it));
    }

    payloadObject.set(SerializeKey::Headers, headers);

    // Body
    payloadObject.set(SerializeKey::Body, WrapSourcemap(payload.assets.body));

    // Schema
    payloadObject.set(SerializeKey::Schema, WrapSourcemap(payload.assets.schema));

    // Assets
    sos::Object assets;

    assets.set(SerializeKey::Body, WrapSourcemap(payload.assets.body));
    assets.set(SerializeKey::Schema, WrapSourcemap(payload.assets.schema));

    payloadObject.set(SerializeKey::Assets, assets);

    return payloadObject;
}

sos::Array WrapParametersSourcemap(const SourceMap<Parameters>& parameters)
{
    sos::Array parametersArray;

    for (Collection<SourceMap<Parameter> >::const_iterator it = parameters.collection.begin();
         it != parameters.collection.end();
         ++it) {

        sos::Object parameter;

        // Name
        parameter.set(SerializeKey::Name, WrapSourcemap(it->name));

        // Description
        parameter.set(SerializeKey::Description, WrapSourcemap(it->description));

        // Type
        parameter.set(SerializeKey::Type, WrapSourcemap(it->type));

        // Use
        parameter.set(SerializeKey::Required, WrapSourcemap(it->use));

        // Example Value
        parameter.set(SerializeKey::Example, WrapSourcemap(it->exampleValue));

        // Default Value
        parameter.set(SerializeKey::Default, WrapSourcemap(it->defaultValue));

        // Values
        sos::Array values;

        for (Collection<SourceMap<Value> >::const_iterator valIt = it->values.collection.begin();
             valIt != it->values.collection.end();
             ++valIt) {

            sos::Object value;

            value.set(SerializeKey::Value, WrapSourcemap(*valIt));

            values.push(value);
        }

        parameter.set(SerializeKey::Values, values);
    }

    return parametersArray;
}

sos::Object WrapTransactionExampleSourcemap(const SourceMap<TransactionExample>& example)
{
    sos::Object exampleObject;

    // Name
    exampleObject.set(SerializeKey::Name, WrapSourcemap(example.name));

    // Description
    exampleObject.set(SerializeKey::Description, WrapSourcemap(example.description));

    // Requests
    sos::Array requests;

    for (Collection<SourceMap<Request> >::const_iterator it = example.requests.collection.begin();
         it != example.requests.collection.end();
         ++it) {

        requests.push(WrapPayloadSourcemap(*it));
    }

    exampleObject.set(SerializeKey::Requests, requests);

    // Responses
    sos::Array responses;

    for (Collection<SourceMap<Response> >::const_iterator it = example.responses.collection.begin();
         it != example.responses.collection.end();
         ++it) {

        responses.push(WrapPayloadSourcemap(*it));
    }

    exampleObject.set(SerializeKey::Responses, responses);

    return exampleObject;
}

sos::Object WrapActionSourcemap(const SourceMap<Action>& action)
{
    sos::Object actionObject;

    // Name
    actionObject.set(SerializeKey::Name, WrapSourcemap(action.name));

    // Description
    actionObject.set(SerializeKey::Description, WrapSourcemap(action.description));

    // HTTP Method
    actionObject.set(SerializeKey::Method, WrapSourcemap(action.method));

    // Parameters
    actionObject.set(SerializeKey::Parameters, WrapParametersSourcemap(action.parameters));

    // Transaction Examples
    sos::Array transactionExamples;

    for (Collection<SourceMap<TransactionExample> >::const_iterator it = action.examples.collection.begin();
         it != action.examples.collection.end();
         ++it) {

        transactionExamples.push(WrapTransactionExampleSourcemap(*it));
    }

    actionObject.set(SerializeKey::Examples, transactionExamples);

    return actionObject;
}

sos::Object WrapResourceSourcemap(const SourceMap<Resource>& resource)
{
    sos::Object resourceObject;

    // Name
    resourceObject.set(SerializeKey::Name, WrapSourcemap(resource.name));

    // Description
    resourceObject.set(SerializeKey::Description, WrapSourcemap(resource.description));

    // URI Template
    resourceObject.set(SerializeKey::URITemplate, WrapSourcemap(resource.uriTemplate));

    // Model
    sos::Object model = (resource.model.name.sourceMap.empty() ? sos::Object() : WrapPayloadSourcemap(resource.model));
    resourceObject.set(SerializeKey::Model, model);

    // Parameters
    resourceObject.set(SerializeKey::Parameters, WrapParametersSourcemap(resource.parameters));

    // Actions
    sos::Array actions;

    for (Collection<SourceMap<Action> >::const_iterator it = resource.actions.collection.begin();
         it != resource.actions.collection.end();
         ++it) {

        actions.push(WrapActionSourcemap(*it));
    }

    resourceObject.set(SerializeKey::Actions, actions);

    return resourceObject;
}

sos::Object WrapResourceGroupSourcemap(const SourceMap<Element>& resourceGroup)
{
    sos::Object resourceGroupObject;

    // Name
    resourceGroupObject.set(SerializeKey::Name, WrapSourcemap(resourceGroup.attributes.name));

    // Description & Resources
    SourceMap<Description> description;
    sos::Array resources;

    for (Collection<SourceMap<Element> >::const_iterator it = resourceGroup.content.elements().collection.begin();
         it != resourceGroup.content.elements().collection.end();
         ++it) {

        if (it->element == Element::ResourceElement) {
            resources.push(WrapResourceSourcemap(it->content.resource));
        }
        else if (it->element == Element::CopyElement) {
            description.sourceMap.append(it->content.copy.sourceMap);
        }
    }

    resourceGroupObject.set(SerializeKey::Description, WrapSourcemap(description));
    resourceGroupObject.set(SerializeKey::Resources, resources);

    return resourceGroupObject;
}


sos::Object WrapDataStructureContent(const SourceMap<DataStructure>& dataStructure)
{
    sos::Object dataStructureObject;

    // Source
    dataStructureObject.set(SerializeKey::Source, WrapNamedTypeSourcemap(dataStructure));

    return dataStructureObject;
}

sos::Object WrapElementSourcemap(const SourceMap<Element>& element)
{
    // Resource Element is special case
    if (element.element == Element::ResourceElement) {
        return WrapResourceSourcemap(element.content.resource);
    }

    sos::Object elementObject;

    if (!element.attributes.name.sourceMap.empty()) {

        sos::Object attributes;

        attributes.set(SerializeKey::Name, WrapSourcemap(element.attributes.name));
        elementObject.set(SerializeKey::Attributes, attributes);
    }

    switch (element.element) {
        case Element::CopyElement:
        {
            elementObject.set(SerializeKey::Content, WrapSourcemap(element.content.copy));
            break;
        }

        case Element::DataStructureElement:
        {
            elementObject.set(SerializeKey::Content, WrapDataStructureContent(element.content.dataStructure));
            break;
        }

        case Element::CategoryElement:
        {
            sos::Array content;

            for (Collection<SourceMap<Element> >::const_iterator it = element.content.elements().collection.begin();
                 it != element.content.elements().collection.end();
                 ++it) {

                content.push(WrapElementSourcemap(*it));
            }

            elementObject.set(SerializeKey::Content, content);
            break;
        }

        default:
            break;
    }

    return elementObject;
}

sos::Object snowcrash::WrapBlueprintSourcemap(const SourceMap<Blueprint>& blueprint)
{
    sos::Object blueprintObject;

    // Metadata
    sos::Array metadata;

    for (Collection<SourceMap<Metadata> >::const_iterator it = blueprint.metadata.collection.begin();
         it != blueprint.metadata.collection.end();
         ++it) {

        metadata.push(WrapSourcemap(*it));
    }

    blueprintObject.set(SerializeKey::Metadata, metadata);

    // Name
    blueprintObject.set(SerializeKey::Name, WrapSourcemap(blueprint.name));

    // Description
    blueprintObject.set(SerializeKey::Description, WrapSourcemap(blueprint.description));

    // Resource Groups
    sos::Array resourceGroups;

    for (Collection<SourceMap<Element> >::const_iterator it = blueprint.content.elements().collection.begin();
         it != blueprint.content.elements().collection.end();
         ++it) {

        if (it->element == Element::CategoryElement &&
            it->category == Element::ResourceGroupCategory) {

            resourceGroups.push(WrapResourceGroupSourcemap(*it));
        }
    }

    blueprintObject.set(SerializeKey::ResourceGroups, resourceGroups);

    // Content
    sos::Array content;

    for (Collection<SourceMap<Element> >::const_iterator it = blueprint.content.elements().collection.begin();
         it != blueprint.content.elements().collection.end();
         ++it) {

        content.push(WrapElementSourcemap(*it));
    }

    blueprintObject.set(SerializeKey::Content, content);

    return blueprintObject;
}
