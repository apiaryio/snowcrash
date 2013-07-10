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

/// \brief Serialize a JSON string.
/// \param value    JSON string to serialize
/// \param os       An output stream to serialize into
static void serialize(const std::string& value, std::ostream &os)
{
    os << "\"" << value << "\"";
}

/// \brief Inserts indentation into an output stream.
/// \brief level    Level of indentation
/// \brief os       An output stream to serialize into
static void indent(size_t level, std::ostream &os)
{
    for (size_t i = 0; i < level; ++i) {
        os << IndentBlock;
    }
}

/// \brief Serialize key value pair into output stream.
/// \param key      Key to serialize
/// \param value    Value to serialize
/// \param level    Indentation level
/// \param object   Flag to indicate whether the pair should be serialized as an object
/// \param os       An output stream to serialize into
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

/// \brief Serialize array of key value pairs.
/// \param collection   Collection to serialize
/// \param level        Level of indentation
/// \param os           An output stream to serialize into
static void serializeKeyValueCollection(const Collection<KeyValuePair>::type& collection, size_t level, std::ostream &os)
{
    os << "[\n";

    size_t i = 0;
    for (Collection<KeyValuePair>::const_iterator it = collection.begin(); it != collection.end(); ++i, ++it) {
        
        if (i > 0 && i < collection.size())
            os << NewLineItemBlock;
        
        serialize(it->first, it->second, level + 1, true, os);
    }
    
    if (!collection.empty()) {
        os << "\n";
        indent(level, os);
    }

    os << "]";
}

/// \brief Serialize Metadata into output stream.
/// \brief metadata     Metadata to serialize
/// \brief os           An output stream to serialize into
static void serialize(const Collection<Metadata>::type& metadata, std::ostream &os)
{
    if (metadata.empty())
        return;
    
    indent(1, os);
    
    serialize(SerializeKey::Metadata, os);
    os << ": ";
    
    serializeKeyValueCollection(metadata, 1, os);
    
    os << NewLineItemBlock;
}

/// \brief Serialize HTTP headers into output stream.
/// \param headers      Headers to serialize
/// \param level        Level of indentation
/// \brief os           An output stream to serialize into
static void serialize(const Collection<Header>::type& headers, size_t level, std::ostream &os)
{
    indent(level, os);
    serialize(SerializeKey::Headers, os);
    os << ": ";
    
    serializeKeyValueCollection(headers, level, os);
}

/// \brief Serialize a payload into output stream.
/// \param payload      A payload to serialize
/// \brief os           An output stream to serialize into
static void serialize(const Payload& payload, size_t level, std::ostream &os)
{
//    indent(level, os);
    os << "{\n";
    
    serialize(SerializeKey::Name, payload.name, level + 1, false, os);
    os << NewLineItemBlock;

    serialize(SerializeKey::Description, payload.description, level + 1, false, os);
    os << NewLineItemBlock;
    
    serialize(SerializeKey::Body, payload.body, level + 1, false, os);
    os << NewLineItemBlock;
    
    serialize(SerializeKey::Schema, payload.schema, level + 1, false, os);
    //os << NewLineItemBlock;
    
    if (!payload.headers.empty()) {
        os << NewLineItemBlock;
        serialize(payload.headers, level + 1, os);
    }
    
    // TODO: parameters
    
    os << "\n";
    indent(level, os);
    os << "}";
}

/// \brief Serialize a method into output stream.
/// \param method       A method to serialize
/// \brief os           An output stream to serialize into
static void serialize(const Method& method, std::ostream &os)
{
    indent(6, os);
    os << "{\n";
    
    serialize(SerializeKey::Method, method.method, 7, false, os);
    os << NewLineItemBlock;
    
    serialize(SerializeKey::Name, method.name, 7, false, os);
    os << NewLineItemBlock;
    
    serialize(SerializeKey::Description, method.description, 7, false, os);
    
    // TODO: parameters
    
    // Headers
    if (!method.headers.empty()) {
        os << NewLineItemBlock;
        serialize(method.headers, 7, os);
    }
    
    // Requests
    if (!method.requests.empty()) {
        os << NewLineItemBlock;
        indent(7, os);
        serialize(SerializeKey::Requests, os);
        os << ": ";
        os << "[\n";
        
        size_t i = 0;
        for (Collection<Request>::const_iterator it = method.requests.begin();
             it != method.requests.end();
             ++i, ++it) {
            
            if (i > 0 && i < method.requests.size())
                os << NewLineItemBlock;
            
            indent(8, os);
            serialize(*it, 8, os);
        }
        
        os << "\n";
        indent(7, os);
        os << "]";
    }
    
    // Responses
    if (!method.responses.empty()) {
        os << NewLineItemBlock;
        indent(7, os);
        serialize(SerializeKey::Responses, os);
        os << ": ";
        os << "[\n";
        
        size_t i = 0;
        for (Collection<Response>::const_iterator it = method.responses.begin();
             it != method.responses.end();
             ++i, ++it) {
            
            if (i > 0 && i < method.responses.size())
                os << NewLineItemBlock;
            
            indent(8, os);
            serialize(*it, 8, os);
        }
        
        os << "\n";
        indent(7, os);
        os << "]";
    }
    
    // Close the method
    os << "\n";
    indent(6, os);
    os << "}";
}

/// \brief Serialize a resources into output stream.
/// \param resource     A resource to serialize
/// \brief os           An output stream to serialize into
static void serialize(const Resource& resource, std::ostream &os)
{
    indent(4, os);
    os << "{\n";

    // URI template
    serialize(SerializeKey::URITemplate, resource.uriTemplate, 5, false, os);
    os << NewLineItemBlock;
    
    // Name
    serialize(SerializeKey::Name, resource.name, 5, false, os);
    os << NewLineItemBlock;
    
    // Description
    serialize(SerializeKey::Description, resource.description, 5, false, os);
    os << NewLineItemBlock;
    
    // Object
    indent(5, os);
    serialize(SerializeKey::Object, os);
    if (resource.object.name.empty()) {
        os << ": {}";
    }
    else {
        os << ": ";
        
        serialize(resource.object, 6, os);
        
//        os << "\n";
//        indent(5, os);
//        os << "}";
    }
    
    // Headers
    if (!resource.headers.empty()) {
        os << NewLineItemBlock;
        serialize(resource.headers, 5, os);
    }
    
    // Methods
    if (!resource.methods.empty()) {
    
        os << NewLineItemBlock;
        indent(5, os);
        serialize(SerializeKey::Methods, os);
        os << ": ";
        os << "[\n";
        
        size_t i = 0;
        for (Collection<Method>::const_iterator it = resource.methods.begin();
             it != resource.methods.end();
             ++i, ++it) {
            
            if (i > 0 && i < resource.methods.size())
                os << NewLineItemBlock;
            
            serialize(*it, os);
        }
        
        os << "\n";
        indent(5, os);    
        os << "]";
    }
    
    // Close the resource
    os << "\n";
    indent(4, os);
    os << "}";    
}

/// \brief Serialize a group of resources into output stream.
/// \param resourceGroup    A group to serialize
/// \brief os               An output stream to serialize into
static void serialize(const ResourceGroup& resourceGroup, std::ostream &os)
{
    indent(2, os);
    os << "{\n";
    
    // Name
    serialize(SerializeKey::Name, resourceGroup.name, 3, false, os);
    os << NewLineItemBlock;
    
    // Description
    serialize(SerializeKey::Description, resourceGroup.description, 3, false, os);
    
    // Resources
    if (!resourceGroup.resources.empty()) {

        os << NewLineItemBlock;
        indent(3, os);
        serialize(SerializeKey::Resources, os);
        os << ": ";
        os << "[\n";
        
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
        
        os << "]";
    }
    
    // Close the group
    os << "\n";
    indent(2, os);
    os << "}";
    
}

/// \brief Serialize Resource Group into output stream.
/// \brief resourceGroup    Resource Groups to serialize
/// \brief os               An output stream to serialize into
static void serialize(const Collection<ResourceGroup>::type& resourceGroups, std::ostream &os)
{
    if (resourceGroups.empty())
        return;
    
    indent(1, os);
    serialize(SerializeKey::ResourceGroups, os);
    os << ": ";
    os << "[\n";
    
    size_t i = 0;
    for (Collection<ResourceGroup>::const_iterator it = resourceGroups.begin(); it != resourceGroups.end(); ++i, ++it) {
        
        if (i > 0 && i < resourceGroups.size())
            os << NewLineItemBlock;
        
        serialize(*it, os);
    }
    
    if (!resourceGroups.empty()) {
        os << "\n";
        indent(1, os);
    }
    
    os << "]";
    
}

static void serialize(const Blueprint& blueprint, std::ostream &os)
{
    os << "{\n";
    
    // Metadata
    serialize(blueprint.metadata, os);

    // Name
    serialize(SerializeKey::Name, blueprint.name, 1, false, os);
    os << NewLineItemBlock;

    // Description
    serialize(SerializeKey::Description, blueprint.description, 1, false, os);

    // Resource Groups
    if (!blueprint.resourceGroups.empty()) {
        os << NewLineItemBlock;
        
        serialize(blueprint.resourceGroups, os);
    }
    
    os << "\n}\n";
}

void snowcrash::SerializeJSON(const snowcrash::Blueprint& blueprint, std::ostream &os)
{
    serialize(blueprint, os);
}
