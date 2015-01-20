//
//  SerializeAST.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 18/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "SerializeAST.h"

using namespace snowcrash;

sos::Object wrapValue(const mson::Value& value)
{
    sos::Object valueObject = sos::Object();

    // Literal
    valueObject.set(SerializeKey::Literal.c_str(), sos::String(value.literal));

    // Variable
    valueObject.set(SerializeKey::Variable.c_str(), sos::Boolean(value.variable));

    return valueObject;
}

sos::Object wrapSymbol(const mson::Symbol& symbol)
{
    sos::Object symbolObject = sos::Object();

    // Literal
    symbolObject.set(SerializeKey::Literal.c_str(), sos::String(symbol.literal));

    // Variable
    symbolObject.set(SerializeKey::Variable.c_str(), sos::Boolean(symbol.variable));

    return symbolObject;
}

sos::Base wrapTypeName(const mson::TypeName& typeName)
{
    if (typeName.base != mson::UndefinedTypeName) {

        std::string baseTypeName = "";

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

    return wrapSymbol(typeName.symbol);
}

sos::Object wrapTypeSpecification(const mson::TypeSpecification& typeSpecification)
{
    sos::Object typeSpecificationObject = sos::Object();

    // Name
    typeSpecificationObject.set(SerializeKey::Name.c_str(), wrapTypeName(typeSpecification.name));

    // Nested Types
    sos::Array nestedTypes = sos::Array();

    for (mson::TypeNames::const_iterator it = typeSpecification.nestedTypes.begin();
         it != typeSpecification.nestedTypes.end();
         ++it) {

        nestedTypes.push(wrapTypeName(*it));
    }

    typeSpecificationObject.set(SerializeKey::NestedTypes.c_str(), nestedTypes);

    return typeSpecificationObject;
}

sos::Array wrapTypeAttributes(const mson::TypeAttributes& typeAttributes)
{
    sos::Array typeAttributesArray = sos::Array();

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

sos::Object wrapTypeDefinition(const mson::TypeDefinition& typeDefinition)
{
    sos::Object typeDefinitionObject = sos::Object();

    // Type Specification
    typeDefinitionObject.set(SerializeKey::TypeSpecification.c_str(), wrapTypeSpecification(typeDefinition.typeSpecification));

    // Type Attributes
    typeDefinitionObject.set(SerializeKey::Attributes.c_str(), wrapTypeAttributes(typeDefinition.attributes));

    return typeDefinitionObject;
}

sos::Object wrapValueDefinition(const mson::ValueDefinition& valueDefinition)
{
    sos::Object valueDefinitionObject = sos::Object();

    // Values
    sos::Array values = sos::Array();

    for (mson::Values::const_iterator it = valueDefinition.values.begin();
         it != valueDefinition.values.end();
         ++it) {

        values.push(wrapValue(*it));
    }

    valueDefinitionObject.set(SerializeKey::Values.c_str(), values);

    // Type Definition
    valueDefinitionObject.set(SerializeKey::TypeDefinition.c_str(), wrapTypeDefinition(valueDefinition.typeDefinition));

    return valueDefinitionObject;
}

sos::Object wrapPropertyName(const mson::PropertyName& propertyName)
{
    sos::Object propertyNameObject = sos::Object();

    if (!propertyName.literal.empty()) {
        propertyNameObject.set(SerializeKey::Literal.c_str(), sos::String(propertyName.literal));
    }
    else if (!propertyName.variable.empty()) {
        propertyNameObject.set(SerializeKey::Variable.c_str(), wrapValueDefinition(propertyName.variable));
    }

    return propertyNameObject;
}

// Forward declarations
sos::Array wrapTypeSections(const mson::TypeSections& typeSections);
sos::Array wrapElements(const mson::Elements& elements);

sos::Object wrapPropertyMember(const mson::PropertyMember& propertyMember)
{
    sos::Object propertyMemberObject = sos::Object();

    // Name
    propertyMemberObject.set(SerializeKey::Name.c_str(), wrapPropertyName(propertyMember.name));

    // Description
    propertyMemberObject.set(SerializeKey::Description.c_str(), sos::String(propertyMember.description.c_str()));

    // Value Definition
    propertyMemberObject.set(SerializeKey::ValueDefinition.c_str(), wrapValueDefinition(propertyMember.valueDefinition));

    // Type Sections
    propertyMemberObject.set(SerializeKey::Sections.c_str(), wrapTypeSections(propertyMember.sections));

    return propertyMemberObject;
}

sos::Object wrapValueMember(const mson::ValueMember& valueMember)
{
    sos::Object valueMemberObject = sos::Object();

    // Description
    valueMemberObject.set(SerializeKey::Description.c_str(), sos::String(valueMember.description.c_str()));

    // Value Definition
    valueMemberObject.set(SerializeKey::ValueDefinition.c_str(), wrapValueDefinition(valueMember.valueDefinition));

    // Type Sections
    valueMemberObject.set(SerializeKey::Sections.c_str(), wrapTypeSections(valueMember.sections));

    return valueMemberObject;
}

sos::Object wrapMixin(const mson::Mixin& mixin)
{
    return wrapTypeDefinition(mixin);
}

sos::Array wrapOneOf(const mson::OneOf& oneOf)
{
    return wrapElements(oneOf);
}

sos::Object wrapElement(const mson::Element& element)
{
    sos::Object elementObject = sos::Object();
    std::string klass = "";

    switch (element.klass) {

        case mson::Element::PropertyClass:
        {
            klass = "property";
            elementObject.set(SerializeKey::Content.c_str(), wrapPropertyMember(element.content.property));
            break;
        }

        case mson::Element::ValueClass:
        {
            klass = "value";
            elementObject.set(SerializeKey::Content.c_str(), wrapValueMember(element.content.value));
            break;
        }

        case mson::Element::MixinClass:
        {
            klass = "mixin";
            elementObject.set(SerializeKey::Content.c_str(), wrapMixin(element.content.mixin));
            break;
        }

        case mson::Element::OneOfClass:
        {
            klass = "oneOf";
            elementObject.set(SerializeKey::Content.c_str(), wrapOneOf(element.content.oneOf()));
            break;
        }

        case mson::Element::GroupClass:
        {
            klass = "group";
            elementObject.set(SerializeKey::Content.c_str(), wrapElements(element.content.elements()));
            break;
        }

        default:
            break;
    }

    elementObject.set(SerializeKey::Class.c_str(), sos::String(klass));

    return elementObject;
}

sos::Array wrapElements(const mson::Elements& elements)
{
    sos::Array elementsArray = sos::Array();

    for (mson::Elements::const_iterator it = elements.begin(); it != elements.end(); ++it) {
        elementsArray.push(wrapElement(*it));
    }

    return elementsArray;
}

sos::Array wrapTypeSections(const mson::TypeSections& sections)
{
    sos::Array sectionsArray = sos::Array();

    for (mson::TypeSections::const_iterator it = sections.begin(); it != sections.end(); ++it) {

        sos::Object section = sos::Object();

        // Class
        std::string klass = "";

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

        section.set(SerializeKey::Class.c_str(), sos::String(klass));

        // Content
        if (!it->content.description.empty()) {
            section.set(SerializeKey::Content.c_str(), sos::String(it->content.description));
        }
        else if (!it->content.value.empty()) {
            section.set(SerializeKey::Content.c_str(), sos::String(it->content.value));
        }
        else if (!it->content.elements().empty()) {
            section.set(SerializeKey::Content.c_str(), wrapElements(it->content.elements()));
        }

        sectionsArray.push(section);
    }

    return sectionsArray;
}

sos::Object wrapNamedType(const mson::NamedType& namedType)
{
    sos::Object namedTypeObject = sos::Object();

    // Name
    namedTypeObject.set(SerializeKey::Name.c_str(), wrapTypeName(namedType.name));

    // Type Definition
    namedTypeObject.set(SerializeKey::TypeDefinition.c_str(), wrapTypeDefinition(namedType.typeDefinition));

    // Type Sections
    namedTypeObject.set(SerializeKey::Sections.c_str(), wrapTypeSections(namedType.sections));

    return namedTypeObject;
}

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

    // Reference
    if (!payload.reference.id.empty()) {
        payloadObject.set(SerializeKey::Reference.c_str(), wrapReference(payload.reference));
    }

    // Description
    payloadObject.set(SerializeKey::Description.c_str(), sos::String(payload.description.c_str()));

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

        // Default Value
        parameter.set(SerializeKey::Default.c_str(), sos::String(it->defaultValue.c_str()));

        // Example Value
        parameter.set(SerializeKey::Example.c_str(), sos::String(it->exampleValue.c_str()));

        // Values
        sos::Array values = sos::Array();

        for (Values::const_iterator valIt = it->values.begin(); valIt != it->values.end(); ++valIt) {

            sos::Object value = sos::Object();

            value.set(SerializeKey::Value.c_str(), sos::String(valIt->c_str()));

            values.push(value);
        }

        parameter.set(SerializeKey::Values.c_str(), values);

        parametersArray.push(parameter);
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

    resourceGroupObject.set(SerializeKey::Resources.c_str(), resources);

    return resourceGroupObject;
}

sos::Object snowcrash::wrapBlueprint(const Blueprint& blueprint)
{
    sos::Object blueprintObject = sos::Object();

    // Version
    blueprintObject.set(SerializeKey::ASTVersion, sos::String(AST_SERIALIZATION_VERSION));

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
