//
//  SerializeYAML.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/3/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "Serialize.h"
#include "SerializeYAML.h"

using namespace snowcrash;

// Serialize key: value, escaping strings with new lines
static void serialize(const std::string& key, const std::string& value, size_t level, std::ostream &os)
{
    if (key.empty())
        return;
    
    for (size_t i = 0; i < level; ++i)
        os << "  ";
    
    if (!value.empty()) {
        
        os << key << ": ";

        std::string normalizedValue = value;
        if (normalizedValue.find("\"") != std::string::npos)
            normalizedValue = EscapeDoubleQuotes(normalizedValue);
        
        if (value.find("\n") != std::string::npos)
            normalizedValue = EscapeNewlines(normalizedValue);

        if (normalizedValue != value)
            os << "\"" << normalizedValue << "\"";
        else
            os << value;
        
         os << std::endl;
    }
    else
        os << key << ":\n";
}

static void serializeKeyValueCollection(const Collection<KeyValuePair>::type& collection, size_t level, std::ostream &os)
{
    for (Collection<KeyValuePair>::const_iterator it = collection.begin(); it != collection.end(); ++it) {
        
        if (it == collection.begin()) {
            
            for (size_t i = 0; i < level; ++i)
                os << "  ";
            os << "- ";
            
            serialize(it->first, it->second, 0, os);
        }
        else {
            serialize(it->first, it->second, level + 1, os);
        }
    }
}

static void serialize(const Collection<Metadata>::type& metadata, std::ostream &os)
{
    if (metadata.empty())
        return;
    
    serialize(SerializeKey::Metadata, std::string(), 0, os);
    serializeKeyValueCollection(metadata, 0, os);
}

static void serialize(const Collection<Header>::type& headers, size_t level, std::ostream &os)
{
    serialize(SerializeKey::Headers, std::string(), level, os);
    serializeKeyValueCollection(headers, level, os);
}

static void serialize(const Payload& payload, size_t level, bool array, std::ostream &os)
{
    for (size_t i = 0; i < level - 1; i++) {
        os << "  ";
    }
    
    if (array)
        os << "- ";
    else
        os << "  ";
    
    serialize(SerializeKey::Name, payload.name, 0, os);
    
    serialize(SerializeKey::Description, payload.description, level, os);
    serialize(SerializeKey::Body, payload.body, level, os);
    serialize(SerializeKey::Schema, payload.schema, level, os);
    
    if (!payload.headers.empty()) {
        serialize(payload.headers, level, os);
    }
    
    // TODO: parameters
}

// Serialize Transaction Example
static void serialize(const Transaction& transaction, std::ostream &os)
{
    os << "      - ";   // indent 4
    serialize(SerializeKey::Name, transaction.name, 0, os);
    serialize(SerializeKey::Description, transaction.description, 4, os);
    
    if (!transaction.requests.empty()) {
        serialize(SerializeKey::Requests, std::string(), 4, os);
        for (Collection<Request>::const_iterator it = transaction.requests.begin(); it != transaction.requests.end(); ++it) {
            serialize(*it, 5, true, os);
        }
    }
    
    if (!transaction.responses.empty()) {
        serialize(SerializeKey::Responses, std::string(), 4, os);
        for (Collection<Response>::const_iterator it = transaction.responses.begin(); it != transaction.responses.end(); ++it) {
            serialize(*it, 5, true, os);
        }
    }
}

// Serialize Action
static void serialize(const Action& action, std::ostream &os)
{
    os << "    - ";   // indent 3
    serialize(SerializeKey::Method, action.method, 0, os);
    serialize(SerializeKey::Name, action.name, 3, os);
    serialize(SerializeKey::Description, action.description, 3, os);
    
    // TODO: parameters
    
    if (!action.headers.empty()) {
        serialize(action.headers, 3, os);
    }
    
    // Transactions
    serialize(SerializeKey::Transactions, action.description, 3, os);
    if (!action.transactions.empty()) {
        for (Collection<Transaction>::const_iterator it = action.transactions.begin();
             it != action.transactions.end();
             ++it) {
            serialize(*it, os);
        }
    }
}

// Serialize Resource
static void serialize(const Resource& resource, std::ostream &os)
{
    os << "  - ";   // indent 2
    serialize(SerializeKey::URITemplate, resource.uriTemplate, 0, os);
    serialize(SerializeKey::Name, resource.name, 2, os);
    serialize(SerializeKey::Description, resource.description, 2, os);

    // TODO: parameters

    serialize(SerializeKey::Model, std::string(), 2, os);
    if (!resource.model.name.empty())
        serialize(resource.model, 3, false, os);
    
    if (!resource.headers.empty())
        serialize(resource.headers, 2, os);

    if (resource.actions.empty())
        return;
    
    serialize(SerializeKey::Actions, std::string(), 2, os);
    for (Collection<Action>::const_iterator it = resource.actions.begin(); it != resource.actions.end(); ++it) {
        serialize(*it, os);
    }
}

// Serialize Resource Group
static void serialize(const ResourceGroup& group, std::ostream &os)
{
    os << "- ";   // indent 1
    serialize(SerializeKey::Name, group.name, 0, os);
    serialize(SerializeKey::Description, group.description, 1, os);

    if (group.resources.empty())
        return;
    
    serialize(SerializeKey::Resources, std::string(), 1, os);
    for (Collection<Resource>::const_iterator it = group.resources.begin(); it != group.resources.end(); ++it) {
        serialize(*it, os);
    }
}

// Serialize Blueprint
static void serialize(const Blueprint& blueprint, std::ostream &os)
{
    serialize(blueprint.metadata, os);
    serialize(SerializeKey::Name, blueprint.name, 0, os);
    serialize(SerializeKey::Description, blueprint.description, 0, os);
    
    if (blueprint.resourceGroups.empty())
        return;
    
    serialize(SerializeKey::ResourceGroups, std::string(), 0, os);
    for (Collection<ResourceGroup>::type::const_iterator it = blueprint.resourceGroups.begin();
         it != blueprint.resourceGroups.end();
         ++it) {
        
        serialize(*it, os);
    }
}

void snowcrash::SerializeYAML(const snowcrash::Blueprint& blueprint, std::ostream &os)
{
    serialize(blueprint, os);
}
