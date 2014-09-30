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

static std::string ReservedCharacters = "#-[]:|>!*&%@`,{}?\'";

/** Normalizes string value for use in YAML and checks whether quotation is need */
static std::string NormalizeStringValue(const std::string& value, bool& needsQuotation)
{
    std::string normalizedValue = value;
    
    if (normalizedValue.find("\"") != std::string::npos)
        normalizedValue = EscapeDoubleQuotes(normalizedValue);
    
    if (value.find("\n") != std::string::npos)
        normalizedValue = EscapeNewlines(normalizedValue);
    
    needsQuotation = (normalizedValue != value);

    if (!needsQuotation) {
        for (std::string::const_iterator it = value.begin() ; it < value.end() ; ++it){
            needsQuotation = ReservedCharacters.find(*it) != std::string::npos;
            if (needsQuotation)
                break;
        }
    }
    
    return normalizedValue;
}


/** Insert array item mark */
static void ArrayItemLeadIn(size_t level, std::ostream &os)
{
    if (level < 1)
        return;
    
    for (size_t i = 0; i < level - 1; ++i)
        os << "  ";

    os << "- ";
}

/** Serialize key value pair */
static void serialize(const std::string& key, const std::string& value, size_t level, std::ostream &os, bool implicitQuotation = true)
{
    if (key.empty())
        return;
    
    for (size_t i = 0; i < level; ++i)
        os << "  ";
    
    if (!value.empty()) {
        
        os << key << ": ";

        bool needsQuotation = false;
        std::string normalizedValue = NormalizeStringValue(value, needsQuotation);
        
        if (implicitQuotation) {
            // Always use quotation
            os << "\"" << normalizedValue << "\"";
        }
        else {
            if (needsQuotation)
                os << "\"" << normalizedValue << "\"";
            else
                os << value;
        }
        
        os << std::endl;
    }
    else
        os << key << ":\n";
}

/** Serialize source map without key into output stream */
static void serialize(const SourceMapBase& set, size_t level, std::ostream &os)
{
    if (!set.sourceMap.empty()) {
        size_t i = 0;

        for (mdp::RangeSet<mdp::BytesRange>::const_iterator it = set.sourceMap.begin(); it != set.sourceMap.end(); ++i, ++it) {

            ArrayItemLeadIn(level + 1, os);
            os << " - " << it->location << "\n";
            ArrayItemLeadIn(level + 2, os);
            os << it->length << "\n";
        }
    } else {
        os << "[]";
    }
}

/** Serialize a key and source map value into output stream. */
static void serialize(const std::string& key, const SourceMapBase& value, size_t level, std::ostream &os)
{
    if (key.empty())
        return;

    for (size_t i = 0; i < level; ++i)
        os << "  ";

    os << key << ": ";
    serialize(value, level + 1, os);
    os << "\n";
}

/** Serializes source map collection */
static void serializeSourceMapCollection(const Collection<SourceMap<KeyValuePair> >::type& collection, size_t level, std::ostream &os)
{
    for (Collection<SourceMap<KeyValuePair> >::const_iterator it = collection.begin();
         it != collection.end();
         ++it) {

        // Array item
        ArrayItemLeadIn(level + 1, os);

        // Source Map
        serialize(*it, level + 1, os);
    }
}

/** Serializes key value collection */
static void serializeKeyValueCollection(const Collection<KeyValuePair>::type& collection, size_t level, std::ostream &os)
{
    for (Collection<KeyValuePair>::const_iterator it = collection.begin();
         it != collection.end();
         ++it) {

        // Array item
        ArrayItemLeadIn(level + 1, os);

        // Name
        serialize(SerializeKey::Name, it->first, 0, os);

        // Value
        serialize(SerializeKey::Value, it->second, level + 1, os);
    }
}

/** Serialize Metadata */
static void serialize(const Collection<Metadata>::type& metadata, std::ostream &os)
{
    serialize(SerializeKey::Metadata, std::string(), 0, os);
    
    if (metadata.empty())
        return;

    serializeKeyValueCollection(metadata, 0, os);
}

/** Serialize Metadata source map */
static void serialize(const Collection<SourceMap<Metadata> >::type& metadata, std::ostream &os)
{
    serialize(SerializeKey::Metadata, std::string(), 0, os);

    if (metadata.empty())
        return;

    serializeSourceMapCollection(metadata, 0, os);
}


/** Serialize Headers */
static void serialize(const Headers& headers, size_t level, std::ostream &os)
{
    serializeKeyValueCollection(headers, level, os);
}

/** Serialize Headers source map */
static void serialize(const Collection<SourceMap<Header> >::type& headers, size_t level, std::ostream &os)
{
    serializeSourceMapCollection(headers, level, os);
}

/** Serialize Parameters */
static void serialize(const Parameters& parameters, size_t level, std::ostream &os)
{
    for (Collection<Parameter>::const_iterator it = parameters.begin(); it != parameters.end(); ++it) {

        // Array item
        ArrayItemLeadIn(level + 1, os);

        // Key / name
        serialize(SerializeKey::Name, it->name, 0, os);

        // Description
        serialize(SerializeKey::Description, it->description, level + 1, os);
        
        // Type
        serialize(SerializeKey::Type, it->type, level + 1, os);
        
        // Required
        serialize(SerializeKey::Required, (it->use == OptionalParameterUse) ? "false" : "true", level + 1, os, false);
        
        // Default
        serialize(SerializeKey::Default, it->defaultValue, level + 1, os);
        
        // Example
        serialize(SerializeKey::Example, it->exampleValue, level + 1, os);
        
        // Values
        serialize(SerializeKey::Values, std::string(), level + 1, os);
        
        if (!it->values.empty()) {
            for (Collection<Value>::const_iterator val_it = it->values.begin();
                 val_it != it->values.end();
                 ++val_it) {

                ArrayItemLeadIn(level + 2, os);
                
                serialize(SerializeKey::Value, *val_it, 0, os);
            }
        }
    }
}

/** Serialize Parameters source map */
static void serialize(const Collection<SourceMap<Parameter> >::type& parameters, size_t level, std::ostream &os)
{
    for (Collection<SourceMap<Parameter> >::const_iterator it = parameters.begin();
         it != parameters.end();
         ++it) {

        // Array item
        ArrayItemLeadIn(level + 1, os);

        // Key / name
        serialize(SerializeKey::Name, it->name, 0, os);

        // Description
        serialize(SerializeKey::Description, it->description, level + 1, os);

        // Type
        serialize(SerializeKey::Type, it->type, level + 1, os);

        // Required
        serialize(SerializeKey::Required, it->use, level + 1, os);

        // Default
        serialize(SerializeKey::Default, it->defaultValue, level + 1, os);

        // Example
        serialize(SerializeKey::Example, it->exampleValue, level + 1, os);

        // Values
        serialize(SerializeKey::Values, std::string(), level + 1, os);

        if (!it->values.collection.empty()) {
            for (Collection<SourceMap<Value> >::const_iterator val_it = it->values.collection.begin();
                 val_it != it->values.collection.end();
                 ++val_it) {

                ArrayItemLeadIn(level + 2, os);

                serialize(SerializeKey::Value, *val_it, 0, os);
            }
        }
    }
}

/** Serialize Payload */
static void serialize(const Payload& payload, size_t level, bool array, std::ostream &os)
{
    for (size_t i = 0; i < level - 1; i++) {
        os << "  ";
    }
    
    if (array)
        os << "- ";
    else
        os << "  ";

    // Name
    serialize(SerializeKey::Name, payload.name, 0, os);

    // Symbol
    if (!payload.symbol.empty()) {
        serialize(SerializeKey::Symbol, payload.symbol, level, os);
    }

    // Description
    serialize(SerializeKey::Description, payload.description, level, os);
    
    // Headers
    serialize(SerializeKey::Headers, std::string(), level, os);

    if (!payload.headers.empty()) {
        serialize(payload.headers, level, os);
    }
    
    // Body
    serialize(SerializeKey::Body, payload.body, level, os);
    
    // Schema
    serialize(SerializeKey::Schema, payload.schema, level, os);
}

/** Serialize Payload source map */
static void serialize(const SourceMap<Payload>& payload, size_t level, bool array, std::ostream &os)
{
    for (size_t i = 0; i < level - 1; i++) {
        os << "  ";
    }

    if (array)
        os << "- ";
    else
        os << "  ";

    // Name
    serialize(SerializeKey::Name, payload.name, 0, os);

    // Symbol
    if (!payload.symbol.sourceMap.empty()) {
        serialize(SerializeKey::Symbol, payload.symbol, level, os);
    }

    // Description
    serialize(SerializeKey::Description, payload.description, level, os);

    // Headers
    serialize(SerializeKey::Headers, std::string(), level, os);

    if (!payload.headers.collection.empty()) {
        serialize(payload.headers.collection, level, os);
    }

    // Body
    serialize(SerializeKey::Body, payload.body, level, os);

    // Schema
    serialize(SerializeKey::Schema, payload.schema, level, os);
}

/** Serialize Transaction Example */
static void serialize(const TransactionExample& example, std::ostream &os)
{
    os << "      - ";   // indent 4

    // Name
    serialize(SerializeKey::Name, example.name, 0, os);
    
    // Description
    serialize(SerializeKey::Description, example.description, 4, os);
    
    // Requests
    serialize(SerializeKey::Requests, std::string(), 4, os);

    if (!example.requests.empty()) {
        for (Collection<Request>::const_iterator it = example.requests.begin();
             it != example.requests.end();
             ++it) {

            serialize(*it, 5, true, os);
        }
    }
    
    // Responses
    serialize(SerializeKey::Responses, std::string(), 4, os);

    if (!example.responses.empty()) {
        for (Collection<Response>::const_iterator it = example.responses.begin();
             it != example.responses.end();
             ++it) {

            serialize(*it, 5, true, os);
        }
    }
}

/** Serialize Transaction Example source map */
static void serialize(const SourceMap<TransactionExample>& example, std::ostream &os)
{
    os << "      - ";   // indent 4

    // Name
    serialize(SerializeKey::Name, example.name, 0, os);

    // Description
    serialize(SerializeKey::Description, example.description, 4, os);

    // Requests
    serialize(SerializeKey::Requests, std::string(), 4, os);

    if (!example.requests.collection.empty()) {
        for (Collection<SourceMap<Request> >::const_iterator it = example.requests.collection.begin();
             it != example.requests.collection.end();
             ++it) {

            serialize(*it, 5, true, os);
        }
    }

    // Responses
    serialize(SerializeKey::Responses, std::string(), 4, os);

    if (!example.responses.collection.empty()) {
        for (Collection<SourceMap<Response> >::const_iterator it = example.responses.collection.begin();
             it != example.responses.collection.end();
             ++it) {

            serialize(*it, 5, true, os);
        }
    }
}

/** Serialize Action */
static void serialize(const Action& action, std::ostream &os)
{
    os << "    - ";   // indent 3
    
    // Name
    serialize(SerializeKey::Name, action.name, 0, os);
    
    // Description
    serialize(SerializeKey::Description, action.description, 3, os);

    // HTTP method
    serialize(SerializeKey::Method, action.method, 3, os);
    
    // Parameters
    serialize(SerializeKey::Parameters, std::string(), 3, os);

    if (!action.parameters.empty())
        serialize(action.parameters, 3, os);
    
    // Examples
    serialize(SerializeKey::Examples, std::string(), 3, os);

    if (!action.examples.empty()) {
        for (Collection<TransactionExample>::const_iterator it = action.examples.begin();
             it != action.examples.end();
             ++it) {

            serialize(*it, os);
        }
    }
}

/** Serialize Action source map */
static void serialize(const SourceMap<Action>& action, std::ostream &os)
{
    os << "    - ";   // indent 3

    // Name
    serialize(SerializeKey::Name, action.name, 0, os);

    // Description
    serialize(SerializeKey::Description, action.description, 3, os);

    // HTTP method
    serialize(SerializeKey::Method, action.method, 3, os);

    // Parameters
    serialize(SerializeKey::Parameters, std::string(), 3, os);

    if (!action.parameters.collection.empty())
        serialize(action.parameters.collection, 3, os);

    // Examples
    serialize(SerializeKey::Examples, std::string(), 3, os);

    if (!action.examples.collection.empty()) {
        for (Collection<SourceMap<TransactionExample> >::const_iterator it = action.examples.collection.begin();
             it != action.examples.collection.end();
             ++it) {

            serialize(*it, os);
        }
    }
}

/** Serialize Resource */
static void serialize(const Resource& resource, std::ostream &os)
{
    os << "  - ";   // indent 2
    
    // Name
    serialize(SerializeKey::Name, resource.name, 0, os);
    
    // Description
    serialize(SerializeKey::Description, resource.description, 2, os);
    
    // URI Template
    serialize(SerializeKey::URITemplate, resource.uriTemplate, 2, os);
    
    // Model
    serialize(SerializeKey::Model, std::string(), 2, os);

    if (!resource.model.name.empty())
        serialize(resource.model, 3, false, os);
    
    // Parameters
    serialize(SerializeKey::Parameters, std::string(), 2, os);

    if (!resource.parameters.empty())
        serialize(resource.parameters, 2, os);
    
    // Actions
    serialize(SerializeKey::Actions, std::string(), 2, os);
    
    if (resource.actions.empty())
        return;
    
    for (Collection<Action>::const_iterator it = resource.actions.begin();
         it != resource.actions.end();
         ++it) {

        serialize(*it, os);
    }
}

/** Serialize Resource source map */
static void serialize(const SourceMap<Resource>& resource, std::ostream &os)
{
    os << "  - ";   // indent 2

    // Name
    serialize(SerializeKey::Name, resource.name, 0, os);

    // Description
    serialize(SerializeKey::Description, resource.description, 2, os);

    // URI Template
    serialize(SerializeKey::URITemplate, resource.uriTemplate, 2, os);

    // Model
    serialize(SerializeKey::Model, std::string(), 2, os);

    if (!resource.model.name.sourceMap.empty())
        serialize(resource.model, 3, false, os);

    // Parameters
    serialize(SerializeKey::Parameters, std::string(), 2, os);

    if (!resource.parameters.collection.empty())
        serialize(resource.parameters.collection, 2, os);

    // Actions
    serialize(SerializeKey::Actions, std::string(), 2, os);

    if (resource.actions.collection.empty())
        return;

    for (Collection<SourceMap<Action> >::const_iterator it = resource.actions.collection.begin();
         it != resource.actions.collection.end();
         ++it) {

        serialize(*it, os);
    }
}

/** Serialize Resource Group */
static void serialize(const ResourceGroup& group, std::ostream &os)
{
    os << "- ";   // indent 1

    // Name
    serialize(SerializeKey::Name, group.name, 0, os);
    
    // Description
    serialize(SerializeKey::Description, group.description, 1, os);

    // Resources
    serialize(SerializeKey::Resources, std::string(), 1, os);
    
    if (group.resources.empty())
        return;

    for (Collection<Resource>::const_iterator it = group.resources.begin();
         it != group.resources.end();
         ++it) {

        serialize(*it, os);
    }
}

/** Serialize Resource Group source map */
static void serialize(const SourceMap<ResourceGroup>& group, std::ostream &os)
{
    os << "- ";   // indent 1

    // Name
    serialize(SerializeKey::Name, group.name, 0, os);

    // Description
    serialize(SerializeKey::Description, group.description, 1, os);

    // Resources
    serialize(SerializeKey::Resources, std::string(), 1, os);

    if (group.resources.collection.empty())
        return;

    for (Collection<SourceMap<Resource> >::const_iterator it = group.resources.collection.begin();
         it != group.resources.collection.end();
         ++it) {

        serialize(*it, os);
    }
}

/** Serialize Blueprint */
static void serialize(const Blueprint& blueprint, std::ostream &os)
{
    // AST Version
    serialize(SerializeKey::ASTVersion, AST_SERIALIZATION_VERSION, 0, os, false);
    
    // Metadata
    serialize(blueprint.metadata, os);
    
    // API Name
    serialize(SerializeKey::Name, blueprint.name, 0, os);
    
    // API Description
    serialize(SerializeKey::Description, blueprint.description, 0, os);
    
    // Resource Groups
    serialize(SerializeKey::ResourceGroups, std::string(), 0, os);

    if (blueprint.resourceGroups.empty())
        return;

    for (Collection<ResourceGroup>::type::const_iterator it = blueprint.resourceGroups.begin();
         it != blueprint.resourceGroups.end();
         ++it) {
        
        serialize(*it, os);
    }
}

/** Serialize Blueprint */
static void serialize(const SourceMap<Blueprint>& blueprint, std::ostream &os)
{
    // Metadata
    serialize(blueprint.metadata.collection, os);

    // API Name
    serialize(SerializeKey::Name, blueprint.name, 0, os);

    // API Description
    serialize(SerializeKey::Description, blueprint.description, 0, os);

    // Resource Groups
    serialize(SerializeKey::ResourceGroups, std::string(), 0, os);

    if (blueprint.resourceGroups.collection.empty())
        return;

    for (Collection<SourceMap<ResourceGroup> >::type::const_iterator it = blueprint.resourceGroups.collection.begin();
         it != blueprint.resourceGroups.collection.end();
         ++it) {

        serialize(*it, os);
    }
}

void snowcrash::SerializeYAML(const snowcrash::Blueprint& blueprint, std::ostream &os)
{
    serialize(blueprint, os);
}

void snowcrash::SerializeSourceMapYAML(const snowcrash::SourceMap<snowcrash::Blueprint>& blueprint, std::ostream &os)
{
    serialize(blueprint, os);
}
