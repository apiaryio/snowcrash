//
//  SerializeYAML.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/3/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
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

        if (value.find("\n") != std::string::npos)
            os << "\"" << EscapeNewlines(value) << "\"";
        else
            os << value;
        
         os << std::endl;
    }
    else
        os << key << ":\n";
}

// Serialize Metadata
static void serialize(const Collection<Metadata>::type& metadata, std::ostream &os)
{
    if (metadata.empty())
        return;
    
    serialize(SerializeKey::Name, std::string(), 0, os);
    for (Collection<Metadata>::type::const_iterator it = metadata.begin(); it != metadata.end(); ++it) {
        serialize(it->first, it->second, 1, os);
    }
}

// Serialize Resource Groups
static void serialize(const ResourceGroup& group, std::ostream &os)
{
    // Group Name
    serialize(SerializeKey::Name, group.name, 1, os);
    
    // Group Description
    serialize(SerializeKey::Description, group.description, 1, os);

    // TODO: resources
}

// Serialize Resource Groups
static void serialize(const Collection<ResourceGroup>::type& resourceGroups, std::ostream &os)
{
    if (resourceGroups.empty())
        return;
    
    serialize("groups", std::string(), 0, os);
    
    for (Collection<ResourceGroup>::type::const_iterator it = resourceGroups.begin(); it != resourceGroups.end(); ++it) {
        serialize(*it, os);
    }
}

// Serialize Blueprint
static void serialize(const Blueprint& blueprint, std::ostream &os)
{
    // Metadata
    serialize(blueprint.metadata, os);
    
    // Name
    serialize(SerializeKey::Name, blueprint.name, 0, os);
    
    // Description
    serialize(SerializeKey::Description, blueprint.description, 0, os);
    
    // Resource Groups
    serialize(blueprint.resourceGroups, os);
}

void snowcrash::SerializeYAML(const snowcrash::Blueprint& blueprint, std::ostream &os)
{
    serialize(blueprint, os);
}