//
//  SerializeJSON.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/27/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "SerializeJSON.h"
#include "Serialize.h"

using namespace snowcrash;

static const std::string IndentBlock = "  ";
static const std::string NewLineItemBlock = ",\n";

/**
 * \brief Serialize a JSON string.
 * \param value    JSON string to serialize
 * \param os       An output stream to serialize into
 */
static void serialize(const std::string& value, std::ostream &os)
{
    os << "\"" << value << "\"";
}

/**
 * \brief Inserts indentation into an output stream.
 * \param level    Level of indentation
 * \param os       An output stream to serialize into
 */
static void indent(size_t level, std::ostream &os)
{
    for (size_t i = 0; i < level; ++i) {
        os << IndentBlock;
    }
}

/**
 * \brief Serialize key value pair into output stream.
 * \param key      Key to serialize
 * \param value    Value to serialize
 * \param level    Indentation level
 * \param object   Flag to indicate whether the pair should be serialized as an object
 * \param os       An output stream to serialize into
 */
static void serialize(const std::string& key, const std::string& value, size_t level, bool object, std::ostream &os)
{
    indent(level, os);
    
    if (object) {
        os << "{\n";
        indent(level + 1, os);
    }
    
    serialize(key, os);
    os << ": ";
    
    std::string normValue = EscapeDoubleQuotes(value);
    if (normValue.find("\n") != std::string::npos)
        serialize(EscapeNewlines(normValue), os);
    else
        serialize(normValue, os);
    
    if (object) {
        os << "\n";
        indent(level, os);
        os << "}";
    }
}

/**
 * \brief Serialize key boolean value pair into output stream
 * \param key      Key to serialize
 * \param value    Value to serialize
 * \param level    Indentation level
 * \param os       An output stream to serialize into
 */
static void serialize(const std::string& key, bool value, size_t level, std::ostream &os)
{
    indent(level, os);
    serialize(key, os);
    os << ": ";
    os << ((value) ? "true" : "false");
}

/**
 * \brief Serialize key value pair as an object into output stream
 * \param key      Key to serialize
 * \param value    Value to serialize
 * \param level    Indentation level
 * \param os       An output stream to serialize into
 */
static void serialize(const std::string& key, const std::string& value, size_t level, std::ostream &os)
{
    indent(level, os);

    os << "{\n";
    
    serialize(SerializeKey::Name, key, level + 1, false, os);
    os << NewLineItemBlock;
    
    serialize(SerializeKey::Value, value, level + 1, false, os);
    os << "\n";
    
    indent(level, os);
    os << "}";
}

/**
 * \brief Serialize an array of key value pairs.
 * \param collection    Collection to serialize
 * \param level         Level of indentation
 * \param os            An output stream to serialize into
 */
static void serializeKeyValueCollection(const Collection<KeyValuePair>::type& collection, size_t level, std::ostream &os)
{
    os << "[";
    
    if (!collection.empty()) {
        os << "\n";
        size_t i = 0;
        for (Collection<KeyValuePair>::const_iterator it = collection.begin(); it != collection.end(); ++i, ++it) {
            
            if (i > 0 && i < collection.size())
                os << NewLineItemBlock;
            
            
            serialize(it->first, it->second, level + 1, os);
        }
        
        os << "\n";
        indent(level, os);
    }

    os << "]";
}
/**
 * \brief Serialize Metadata into output stream.
 * \param metadata  Metadata to serialize
 * \param os        An output stream to serialize into
 */
static void serialize(const Collection<Metadata>::type& metadata, std::ostream &os)
{
    indent(1, os);
    serialize(SerializeKey::Metadata, os);
    os << ": ";
    
    serializeKeyValueCollection(metadata, 1, os);
    
    os << NewLineItemBlock;
}

/**
 * \brief Serialize Parameters into output stream.
 * \param prarameters   Parameters to serialize.
 * \param level         Level of indentation.
 * \param os            An output stream to serialize into.
 */
static void serialize(const Collection<Parameter>::type& parameters, size_t level, std::ostream &os)
{
    indent(level, os);
    serialize(SerializeKey::Parameters, os);
    os << ": [";
    
    if (!parameters.empty()) {
        os << "\n";
        size_t i = 0;
        for (Collection<Parameter>::const_iterator it = parameters.begin(); it != parameters.end(); ++i, ++it) {
            
            if (i > 0 && i < parameters.size())
                os << NewLineItemBlock;

            indent(level + 1, os);
            os << "{\n";
            
            // Name
            serialize(SerializeKey::Name, it->name, level + 2, false, os);
            os << NewLineItemBlock;
            
            // Description
            serialize(SerializeKey::Description, it->description, level + 2, false, os);
            os << NewLineItemBlock;
            
            // Type
            serialize(SerializeKey::Type, it->type, level + 2, false, os);
            os << NewLineItemBlock;
            
            // Requried
            serialize(SerializeKey::Required, (it->use != OptionalParameterUse), level + 2, os);
            os << NewLineItemBlock;
            
            // Default
            serialize(SerializeKey::Default, it->defaultValue, level + 2, false, os);
            os << NewLineItemBlock;
            
            // Example
            serialize(SerializeKey::Example, it->exampleValue, level + 2, false, os);
            os << NewLineItemBlock;
            
            // Values
            indent(level + 2, os);
            serialize(SerializeKey::Values, os);
            os << ": [";

            if (!it->values.empty()) {
                os << "\n";
                size_t j = 0;
                for (Collection<Value>::const_iterator val_it = it->values.begin(); val_it != it->values.end(); ++j, ++val_it) {
                    
                    if (j > 0 && j < it->values.size())
                        os << NewLineItemBlock;
                    
                    serialize(SerializeKey::Value, *val_it, level + 3, true, os);
                }
                
                os << "\n";
                indent(level + 2, os);
            }
            
            // Close Values
            os << "]";
            
            // Close Key / name object
            os << "\n";
            indent(level + 1, os);
            os << "}";
        }
        
        os << std::endl;
        indent(level, os);
    }
    
    os << "]";
}

/**
 * \brief Serialize HTTP headers into output stream.
 * \param headers   Headers to serialize.
 * \param level Level of indentation.
 * \param os    An output stream to serialize into.
 */
static void serialize(const Collection<Header>::type& headers, size_t level, std::ostream &os)
{
    indent(level, os);
    serialize(SerializeKey::Headers, os);
    os << ": ";
    
    serializeKeyValueCollection(headers, level, os);
}

/**
 * \brief Serialize a payload into output stream.
 * \param payload   A payload to serialize.
 * \param os    An output stream to serialize into.
 */
static void serialize(const Payload& payload, size_t level, std::ostream &os)
{
    os << "{\n";
    
    // Name
    serialize(SerializeKey::Name, payload.name, level + 1, false, os);
    os << NewLineItemBlock;

    // Description
    serialize(SerializeKey::Description, payload.description, level + 1, false, os);
    os << NewLineItemBlock;
    
    // Headers
    serialize(payload.headers, level + 1, os);
    os << NewLineItemBlock;
    
    // Body
    serialize(SerializeKey::Body, payload.body, level + 1, false, os);
    os << NewLineItemBlock;
    
    // Schema
    serialize(SerializeKey::Schema, payload.schema, level + 1, false, os);

    os << "\n";
    indent(level, os);
    os << "}";
}

/**
 * \brief Serialize a transaction example into output stream.
 * \param transaction   A transaction example to serialize.
 * \param os            An output stream to serialize into.
 */
static void serialize(const TransactionExample& example, std::ostream &os)
{
    indent(8, os);
    os << "{\n";
    
    // Name
    serialize(SerializeKey::Name, example.name, 9, false, os);
    os << NewLineItemBlock;
    
    // Description
    serialize(SerializeKey::Description, example.description, 9, false, os);
    os << NewLineItemBlock;
    
    // Requests
    indent(9, os);
    serialize(SerializeKey::Requests, os);
    os << ": ";
    os << "[";
    
    if (!example.requests.empty()) {
        os << "\n";
        size_t i = 0;
        for (Collection<Request>::const_iterator it = example.requests.begin();
             it != example.requests.end();
             ++i, ++it) {
            
            if (i > 0 && i < example.requests.size())
                os << NewLineItemBlock;
            
            indent(10, os);
            serialize(*it, 10, os);
        }
        
        os << "\n";
        indent(9, os);
    }
    os << "]";
    os << NewLineItemBlock;
    
    // Responses
    indent(9, os);
    serialize(SerializeKey::Responses, os);
    os << ": ";
    os << "[";
    
    if (!example.responses.empty()) {
        os << "\n";
        size_t i = 0;
        for (Collection<Response>::const_iterator it = example.responses.begin();
             it != example.responses.end();
             ++i, ++it) {
            
            if (i > 0 && i < example.responses.size())
                os << NewLineItemBlock;
            
            indent(10, os);
            serialize(*it, 10, os);
        }
        
        os << "\n";
        indent(9, os);
    }
    os << "]";
    
    // Close the transaction
    os << "\n";
    indent(8, os);
    os << "}";
}

/**
 * \brief Serialize an action into output stream.
 * \param action    The action to serialize.
 * \param os        An output stream to serialize into.
 */
static void serialize(const Action& action, std::ostream &os)
{
    indent(6, os);
    os << "{\n";
    
    // Name
    serialize(SerializeKey::Name, action.name, 7, false, os);
    os << NewLineItemBlock;
    
    // Description
    serialize(SerializeKey::Description, action.description, 7, false, os);
    os << NewLineItemBlock;
    
    // HTTP Method
    serialize(SerializeKey::Method, action.method, 7, false, os);
    os << NewLineItemBlock;
    
    // Parameters
    serialize(action.parameters, 7, os);
    os << NewLineItemBlock;
    
    // Transactions
    indent(7, os);
    serialize(SerializeKey::Examples, os);
    os << ": ";
    os << "[";
    
    if (!action.examples.empty()) {
        os << "\n";
        size_t i = 0;
        for (Collection<TransactionExample>::const_iterator it = action.examples.begin();
             it != action.examples.end();
             ++i, ++it) {
            
            if (i > 0 && i < action.examples.size())
                os << NewLineItemBlock;
            
            serialize(*it, os);
        }
        
        os << "\n";
        indent(7, os);
    }
    
    os << "]";
    
    // Close the action
    os << "\n";
    indent(6, os);
    os << "}";
}

/**
 * \brief Serialize a resources into output stream.
 * \param resource     A resource to serialize
 * \param os           An output stream to serialize into
 */
static void serialize(const Resource& resource, std::ostream &os)
{
    indent(4, os);
    os << "{\n";

    // Name
    serialize(SerializeKey::Name, resource.name, 5, false, os);
    os << NewLineItemBlock;
    
    // Description
    serialize(SerializeKey::Description, resource.description, 5, false, os);
    os << NewLineItemBlock;
    
    // URI template
    serialize(SerializeKey::URITemplate, resource.uriTemplate, 5, false, os);
    os << NewLineItemBlock;
    
    // Model
    indent(5, os);
    serialize(SerializeKey::Model, os);
    if (resource.model.name.empty()) {
        os << ": {}";
    }
    else {
        os << ": ";
        
        serialize(resource.model, 5, os);
    }
    os << NewLineItemBlock;
    
    // Parameters
    serialize(resource.parameters, 5, os);
    os << NewLineItemBlock;
    
    // Actions
    indent(5, os);
    serialize(SerializeKey::Actions, os);
    os << ": ";
    os << "[";

    if (!resource.actions.empty()) {
        os << "\n";
        
        size_t i = 0;
        for (Collection<Action>::const_iterator it = resource.actions.begin();
             it != resource.actions.end();
             ++i, ++it) {
            
            if (i > 0 && i < resource.actions.size())
                os << NewLineItemBlock;
            
            serialize(*it, os);
        }
        
        os << "\n";
        indent(5, os);
    }
    os << "]";
    
    // Close the resource
    os << "\n";
    indent(4, os);
    os << "}";    
}

/**
 * \brief Serialize a group of resources into output stream.
 * \param resourceGroup A group to serialize.
 * \brief os    An output stream to serialize into.
 */
static void serialize(const ResourceGroup& resourceGroup, std::ostream &os)
{
    indent(2, os);
    os << "{\n";
    
    // Name
    serialize(SerializeKey::Name, resourceGroup.name, 3, false, os);
    os << NewLineItemBlock;
    
    // Description
    serialize(SerializeKey::Description, resourceGroup.description, 3, false, os);
    os << NewLineItemBlock;
    
    // Resources
    indent(3, os);
    serialize(SerializeKey::Resources, os);
    os << ": ";
    os << "[";
    
    if (!resourceGroup.resources.empty()) {
        os << "\n";
        size_t i = 0;
        for (Collection<Resource>::const_iterator it = resourceGroup.resources.begin();
             it != resourceGroup.resources.end();
             ++i, ++it) {
            
            if (i > 0 && i < resourceGroup.resources.size())
                os << NewLineItemBlock;
            
            serialize(*it, os);
        }
        
        if (!resourceGroup.resources.empty()) {
            os << "\n";
            indent(3, os);
        }
    }
    
    os << "]";
    
    // Close the group
    os << "\n";
    indent(2, os);
    os << "}";
}

/**
 * \brief Serialize Resource Group into output stream.
 * \param resourceGroup Resource Groups to serialize.
 * \param os            An output stream to serialize into.
 */
static void serialize(const Collection<ResourceGroup>::type& resourceGroups, std::ostream &os)
{
    indent(1, os);
    serialize(SerializeKey::ResourceGroups, os);
    os << ": ";
    os << "[";
    
    if (!resourceGroups.empty()) {
        os << "\n";
        size_t i = 0;
        for (Collection<ResourceGroup>::const_iterator it = resourceGroups.begin(); it != resourceGroups.end(); ++i, ++it) {
            
            if (i > 0 && i < resourceGroups.size())
                os << NewLineItemBlock;
            
            serialize(*it, os);
        }
        
        os << "\n";
        indent(1, os);
    }
    
    os << "]";
}

/**
 * \brief Serialize a blueprint into output stream.
 * \param blueprint     The blueprint to serialize.
 * \param os            An output stream to serialize into.
 */
static void serialize(const Blueprint& blueprint, std::ostream &os)
{
    os << "{\n";
    
    // AST Version
    serialize(SerializeKey::ASTVersion, AST_SERIALIZATION_VERSION, 1, false, os);
    os << NewLineItemBlock;
    
    // Metadata
    serialize(blueprint.metadata, os);

    // Name
    serialize(SerializeKey::Name, blueprint.name, 1, false, os);
    os << NewLineItemBlock;

    // Description
    serialize(SerializeKey::Description, blueprint.description, 1, false, os);
    os << NewLineItemBlock;

    // Resource Groups
    serialize(blueprint.resourceGroups, os);
    
    os << "\n}\n";
}

void snowcrash::SerializeJSON(const snowcrash::Blueprint& blueprint, std::ostream &os)
{
    serialize(blueprint, os);
}
