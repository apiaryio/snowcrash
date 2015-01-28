//
//  SerializeAST.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 18/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "SerializeAST.h"

using namespace snowcrash;

sos::Object WrapValue(const mson::Value& value)
{
    sos::Object valueObject;

    // Literal
    valueObject.set(SerializeKey::Literal, sos::String(value.literal));

    // Variable
    valueObject.set(SerializeKey::Variable, sos::Boolean(value.variable));

    return valueObject;
}

sos::Object WrapSymbol(const mson::Symbol& symbol)
{
    sos::Object symbolObject;

    // Literal
    symbolObject.set(SerializeKey::Literal, sos::String(symbol.literal));

    // Variable
    symbolObject.set(SerializeKey::Variable, sos::Boolean(symbol.variable));

    return symbolObject;
}

sos::Base WrapTypeName(const mson::TypeName& typeName)
{
    if (typeName.base != mson::UndefinedTypeName) {

        std::string baseTypeName;

        switch (typeName.base) {

            case mson::BooleanTypeName:
                baseTypeName = "boolean";
                break;

            case mson::StringTypeName:
                baseTypeName = "string";
                break;

            case mson::NumberTypeName:
                baseTypeName = "number";
                break;

            case mson::ArrayTypeName:
                baseTypeName = "array";
                break;

            case mson::EnumTypeName:
                baseTypeName = "enum";
                break;

            case mson::ObjectTypeName:
                baseTypeName = "object";
                break;

            default:
                break;
        }

        return sos::String(baseTypeName);
    }

    return WrapSymbol(typeName.symbol);
}

sos::Object WrapTypeSpecification(const mson::TypeSpecification& typeSpecification)
{
    sos::Object typeSpecificationObject;

    // Name
    typeSpecificationObject.set(SerializeKey::Name, WrapTypeName(typeSpecification.name));

    // Nested Types
    sos::Array nestedTypes;

    for (mson::TypeNames::const_iterator it = typeSpecification.nestedTypes.begin();
         it != typeSpecification.nestedTypes.end();
         ++it) {

        nestedTypes.push(WrapTypeName(*it));
    }

    typeSpecificationObject.set(SerializeKey::NestedTypes, nestedTypes);

    return typeSpecificationObject;
}

sos::Array WrapTypeAttributes(const mson::TypeAttributes& typeAttributes)
{
    sos::Array typeAttributesArray;

    if (typeAttributes & mson::RequiredTypeAttribute) {
        typeAttributesArray.push(sos::String("required"));
    }
    else if (typeAttributes & mson::OptionalTypeAttribute) {
        typeAttributesArray.push(sos::String("optional"));
    }
    else if (typeAttributes & mson::DefaultTypeAttribute) {
        typeAttributesArray.push(sos::String("default"));
    }
    else if (typeAttributes & mson::SampleTypeAttribute) {
        typeAttributesArray.push(sos::String("sample"));
    }
    else if (typeAttributes & mson::FixedTypeAttribute) {
        typeAttributesArray.push(sos::String("fixed"));
    }

    return typeAttributesArray;
}

sos::Object WrapTypeDefinition(const mson::TypeDefinition& typeDefinition)
{
    sos::Object typeDefinitionObject;

    // Type Specification
    typeDefinitionObject.set(SerializeKey::TypeSpecification, WrapTypeSpecification(typeDefinition.typeSpecification));

    // Type Attributes
    typeDefinitionObject.set(SerializeKey::Attributes, WrapTypeAttributes(typeDefinition.attributes));

    return typeDefinitionObject;
}

sos::Object WrapValueDefinition(const mson::ValueDefinition& valueDefinition)
{
    sos::Object valueDefinitionObject;

    // Values
    sos::Array values;

    for (mson::Values::const_iterator it = valueDefinition.values.begin();
         it != valueDefinition.values.end();
         ++it) {

        values.push(WrapValue(*it));
    }

    valueDefinitionObject.set(SerializeKey::Values, values);

    // Type Definition
    valueDefinitionObject.set(SerializeKey::TypeDefinition, WrapTypeDefinition(valueDefinition.typeDefinition));

    return valueDefinitionObject;
}

sos::Object WrapPropertyName(const mson::PropertyName& propertyName)
{
    sos::Object propertyNameObject;

    if (!propertyName.literal.empty()) {
        propertyNameObject.set(SerializeKey::Literal, sos::String(propertyName.literal));
    }
    else if (!propertyName.variable.empty()) {
        propertyNameObject.set(SerializeKey::Variable, WrapValueDefinition(propertyName.variable));
    }

    return propertyNameObject;
}

// Forward declarations
sos::Array WrapTypeSections(const mson::TypeSections& typeSections);
sos::Array WrapMSONElements(const mson::Elements& elements);

sos::Object WrapPropertyMember(const mson::PropertyMember& propertyMember)
{
    sos::Object propertyMemberObject;

    // Name
    propertyMemberObject.set(SerializeKey::Name, WrapPropertyName(propertyMember.name));

    // Description
    propertyMemberObject.set(SerializeKey::Description, sos::String(propertyMember.description));

    // Value Definition
    propertyMemberObject.set(SerializeKey::ValueDefinition, WrapValueDefinition(propertyMember.valueDefinition));

    // Type Sections
    propertyMemberObject.set(SerializeKey::Sections, WrapTypeSections(propertyMember.sections));

    return propertyMemberObject;
}

sos::Object WrapValueMember(const mson::ValueMember& valueMember)
{
    sos::Object valueMemberObject;

    // Description
    valueMemberObject.set(SerializeKey::Description, sos::String(valueMember.description));

    // Value Definition
    valueMemberObject.set(SerializeKey::ValueDefinition, WrapValueDefinition(valueMember.valueDefinition));

    // Type Sections
    valueMemberObject.set(SerializeKey::Sections, WrapTypeSections(valueMember.sections));

    return valueMemberObject;
}

sos::Object WrapMixin(const mson::Mixin& mixin)
{
    return WrapTypeDefinition(mixin);
}

sos::Array WrapOneOf(const mson::OneOf& oneOf)
{
    return WrapMSONElements(oneOf);
}

sos::Object WrapMSONElement(const mson::Element& element)
{
    sos::Object elementObject;
    std::string klass;

    switch (element.klass) {

        case mson::Element::PropertyClass:
        {
            klass = "property";
            elementObject.set(SerializeKey::Content, WrapPropertyMember(element.content.property));
            break;
        }

        case mson::Element::ValueClass:
        {
            klass = "value";
            elementObject.set(SerializeKey::Content, WrapValueMember(element.content.value));
            break;
        }

        case mson::Element::MixinClass:
        {
            klass = "mixin";
            elementObject.set(SerializeKey::Content, WrapMixin(element.content.mixin));
            break;
        }

        case mson::Element::OneOfClass:
        {
            klass = "oneOf";
            elementObject.set(SerializeKey::Content, WrapOneOf(element.content.oneOf()));
            break;
        }

        case mson::Element::GroupClass:
        {
            klass = "group";
            elementObject.set(SerializeKey::Content, WrapMSONElements(element.content.elements()));
            break;
        }

        default:
            break;
    }

    elementObject.set(SerializeKey::Class, sos::String(klass));

    return elementObject;
}

sos::Array WrapMSONElements(const mson::Elements& elements)
{
    sos::Array elementsArray;

    for (mson::Elements::const_iterator it = elements.begin(); it != elements.end(); ++it) {
        elementsArray.push(WrapMSONElement(*it));
    }

    return elementsArray;
}

sos::Array WrapTypeSections(const mson::TypeSections& sections)
{
    sos::Array sectionsArray;

    for (mson::TypeSections::const_iterator it = sections.begin(); it != sections.end(); ++it) {

        sos::Object section;

        // Class
        std::string klass;

        switch (it->klass) {
            case mson::TypeSection::BlockDescriptionClass:
                klass = "blockDescription";
                break;

            case mson::TypeSection::MemberTypeClass:
                klass = "memberType";
                break;

            case mson::TypeSection::SampleClass:
                klass = "sample";
                break;

            case mson::TypeSection::DefaultClass:
                klass = "default";
                break;

            default:
                break;
        }

        section.set(SerializeKey::Class, sos::String(klass));

        // Content
        if (!it->content.description.empty()) {
            section.set(SerializeKey::Content, sos::String(it->content.description));
        }
        else if (!it->content.value.empty()) {
            section.set(SerializeKey::Content, sos::String(it->content.value));
        }
        else if (!it->content.elements().empty()) {
            section.set(SerializeKey::Content, WrapMSONElements(it->content.elements()));
        }

        sectionsArray.push(section);
    }

    return sectionsArray;
}

sos::Object WrapNamedType(const mson::NamedType& namedType)
{
    sos::Object namedTypeObject;

    // Name
    namedTypeObject.set(SerializeKey::Name, WrapTypeName(namedType.name));

    // Type Definition
    namedTypeObject.set(SerializeKey::TypeDefinition, WrapTypeDefinition(namedType.typeDefinition));

    // Type Sections
    namedTypeObject.set(SerializeKey::Sections, WrapTypeSections(namedType.sections));

    return namedTypeObject;
}

sos::String WrapElementClass(const Element::Class& element)
{
    std::string str;

    switch (element) {
        case Element::CategoryElement:
            str = "category";
            break;

        case Element::CopyElement:
            str = "copy";
            break;

        case Element::ResourceElement:
            str = "resource";
            break;

        case Element::DataStructureElement:
            str = "dataStructure";
            break;

        default:
            break;
    }

    return sos::String(str);
}

sos::Object WrapKeyValue(const KeyValuePair& keyValue)
{
    sos::Object keyValueObject;

    // Name
    keyValueObject.set(SerializeKey::Name, sos::String(keyValue.first));

    // Value
    keyValueObject.set(SerializeKey::Value, sos::String(keyValue.second));

    return keyValueObject;
}

sos::Object WrapMetadata(const Metadata& metadata)
{
    return WrapKeyValue(metadata);
}

sos::Object WrapHeader(const Header& header)
{
    return WrapKeyValue(header);
}

sos::Object WrapReference(const Reference& reference)
{
    sos::Object referenceObject;

    // Id
    referenceObject.set(SerializeKey::Id, sos::String(reference.id));

    return referenceObject;
}

sos::Object WrapAsset(const Asset& asset)
{
    sos::Object assetObject;

    // Source
    assetObject.set(SerializeKey::Source, sos::String(asset.source));

    // Resolved
    assetObject.set(SerializeKey::Resolved, sos::String(asset.resolved));

    return assetObject;
}

sos::Object WrapPayload(const Payload& payload)
{
    sos::Object payloadObject;

    // Name
    payloadObject.set(SerializeKey::Name, sos::String(payload.name));

    // Reference
    if (!payload.reference.id.empty()) {
        payloadObject.set(SerializeKey::Reference, WrapReference(payload.reference));
    }

    // Description
    payloadObject.set(SerializeKey::Description, sos::String(payload.description));

    // Headers
    sos::Array headers;

    for (Headers::const_iterator it = payload.headers.begin();
         it != payload.headers.end();
         ++it) {

        headers.push(WrapHeader(*it));
    }

    payloadObject.set(SerializeKey::Headers, headers);

    // Body
    payloadObject.set(SerializeKey::Body, sos::String(payload.assets.body.source));

    // Schema
    payloadObject.set(SerializeKey::Schema, sos::String(payload.assets.schema.source));

    // Assets
    sos::Object assets;

    assets.set(SerializeKey::Body, WrapAsset(payload.assets.body));
    assets.set(SerializeKey::Schema, WrapAsset(payload.assets.schema));

    payloadObject.set(SerializeKey::Assets, assets);

    return payloadObject;
}

sos::Array WrapParameters(const Parameters& parameters)
{
    sos::Array parametersArray;

    for (Parameters::const_iterator it = parameters.begin(); it != parameters.end(); ++it) {

        sos::Object parameter;

        // Name
        parameter.set(SerializeKey::Name, sos::String(it->name));

        // Description
        parameter.set(SerializeKey::Description, sos::String(it->description));

        // Type
        parameter.set(SerializeKey::Type, sos::String(it->type));

        // Use
        parameter.set(SerializeKey::Required, sos::Boolean(it->use != snowcrash::OptionalParameterUse));

        // Default Value
        parameter.set(SerializeKey::Default, sos::String(it->defaultValue));

        // Example Value
        parameter.set(SerializeKey::Example, sos::String(it->exampleValue));

        // Values
        sos::Array values;

        for (Values::const_iterator valIt = it->values.begin(); valIt != it->values.end(); ++valIt) {

            sos::Object value;

            value.set(SerializeKey::Value, sos::String(valIt->c_str()));

            values.push(value);
        }

        parameter.set(SerializeKey::Values, values);

        parametersArray.push(parameter);
    }

    return parametersArray;
}

sos::Object WrapTransactionExample(const TransactionExample& example)
{
    sos::Object exampleObject;

    // Name
    exampleObject.set(SerializeKey::Name, sos::String(example.name));

    // Description
    exampleObject.set(SerializeKey::Description, sos::String(example.description));

    // Requests
    sos::Array requests;

    for (Requests::const_iterator it = example.requests.begin();
         it != example.requests.end();
         ++it) {

        requests.push(WrapPayload(*it));
    }

    exampleObject.set(SerializeKey::Requests, requests);

    // Responses
    sos::Array responses;

    for (Responses::const_iterator it = example.responses.begin();
         it != example.responses.end();
         ++it) {

        responses.push(WrapPayload(*it));
    }

    exampleObject.set(SerializeKey::Responses, responses);

    return exampleObject;
}

sos::Object WrapAction(const Action& action)
{
    sos::Object actionObject;

    // Name
    actionObject.set(SerializeKey::Name, sos::String(action.name));

    // Description
    actionObject.set(SerializeKey::Description, sos::String(action.description));

    // HTTP Method
    actionObject.set(SerializeKey::Method, sos::String(action.method));

    // Parameters
    actionObject.set(SerializeKey::Parameters, WrapParameters(action.parameters));

    // Transaction Examples
    sos::Array transactionExamples;

    for (TransactionExamples::const_iterator it = action.examples.begin();
         it != action.examples.end();
         ++it) {

        transactionExamples.push(WrapTransactionExample(*it));
    }

    actionObject.set(SerializeKey::Examples, transactionExamples);

    return actionObject;
}

sos::Object WrapResource(const Resource& resource, bool printElementValue = false)
{
    sos::Object resourceObject;

    // Name
    resourceObject.set(SerializeKey::Name, sos::String(resource.name));

    // Description
    resourceObject.set(SerializeKey::Description, sos::String(resource.description));

    // Element
    if (printElementValue) {
        resourceObject.set(SerializeKey::Element, WrapElementClass(Element::ResourceElement));
    }

    // URI Template
    resourceObject.set(SerializeKey::URITemplate, sos::String(resource.uriTemplate));

    // Model
    sos::Object model = (resource.model.name.empty() ? sos::Object() : WrapPayload(resource.model));
    resourceObject.set(SerializeKey::Model, model);

    // Parameters
    resourceObject.set(SerializeKey::Parameters, WrapParameters(resource.parameters));

    // Actions
    sos::Array actions;

    for (Actions::const_iterator it = resource.actions.begin();
         it != resource.actions.end();
         ++it) {

        actions.push(WrapAction(*it));
    }

    resourceObject.set(SerializeKey::Actions, actions);

    return resourceObject;
}

sos::Object WrapResourceGroup(const Element& resourceGroup)
{
    sos::Object resourceGroupObject;

    // Name
    resourceGroupObject.set(SerializeKey::Name, sos::String(resourceGroup.attributes.name));

    // Description && Resources
    std::string description;
    sos::Array resources;

    for (Elements::const_iterator it = resourceGroup.content.elements().begin();
         it != resourceGroup.content.elements().end();
         ++it) {

        if (it->element == Element::ResourceElement) {
            resources.push(WrapResource(it->content.resource));
        }
        else if (it->element == Element::CopyElement) {

            if (!description.empty()) {
                TwoNewLines(description);
            }

            description += it->content.copy;
        }
    }

    resourceGroupObject.set(SerializeKey::Description, sos::String(description));
    resourceGroupObject.set(SerializeKey::Resources, resources);

    return resourceGroupObject;
}

sos::Object WrapDataStructureContent(const DataStructure& dataStructure)
{
    sos::Object dataStructureObject;

    // Source
    dataStructureObject.set(SerializeKey::Source, WrapNamedType(dataStructure.source));

    // Resolved
    dataStructureObject.set(SerializeKey::Resolved, sos::Object());

    return dataStructureObject;
}

sos::Object WrapElement(const Element& element)
{
    // Resource Element is special case
    if (element.element == Element::ResourceElement) {
        return WrapResource(element.content.resource, true);
    }

    sos::Object elementObject;

    elementObject.set(SerializeKey::Element, WrapElementClass(element.element));

    if (!element.attributes.name.empty()) {

        sos::Object attributes;

        attributes.set(SerializeKey::Name, sos::String(element.attributes.name));
        elementObject.set(SerializeKey::Attributes, attributes);
    }

    switch (element.element) {
        case Element::CopyElement:
        {
            elementObject.set(SerializeKey::Content, sos::String(element.content.copy));
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

            for (Elements::const_iterator it = element.content.elements().begin();
                 it != element.content.elements().end();
                 ++it) {

                content.push(WrapElement(*it));
            }

            elementObject.set(SerializeKey::Content, content);
            break;
        }

        default:
            break;
    }

    return elementObject;
}

sos::Object snowcrash::WrapBlueprint(const Blueprint& blueprint)
{
    sos::Object blueprintObject;

    // Version
    blueprintObject.set(SerializeKey::ASTVersion, sos::String(AST_SERIALIZATION_VERSION));

    // Metadata
    sos::Array metadata;

    for (MetadataCollection::const_iterator it = blueprint.metadata.begin();
         it != blueprint.metadata.end();
         ++it) {

        metadata.push(WrapMetadata(*it));
    }

    blueprintObject.set(SerializeKey::Metadata, metadata);

    // Name
    blueprintObject.set(SerializeKey::Name, sos::String(blueprint.name));

    // Description
    blueprintObject.set(SerializeKey::Description, sos::String(blueprint.description));

    // Element
    blueprintObject.set(SerializeKey::Element, WrapElementClass(blueprint.element));

    // Resource Groups
    sos::Array resourceGroups;

    for (Elements::const_iterator it = blueprint.content.elements().begin();
         it != blueprint.content.elements().end();
         ++it) {

        if (it->element == Element::CategoryElement &&
            it->category == Element::ResourceGroupCategory) {

            resourceGroups.push(WrapResourceGroup(*it));
        }
    }

    blueprintObject.set(SerializeKey::ResourceGroups, resourceGroups);

    // Content
    sos::Array content;

    for (Elements::const_iterator it = blueprint.content.elements().begin();
         it != blueprint.content.elements().end();
         ++it) {

        content.push(WrapElement(*it));
    }

    blueprintObject.set(SerializeKey::Content, content);

    return blueprintObject;
}
