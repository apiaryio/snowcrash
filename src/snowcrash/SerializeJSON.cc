//
//  SerializeJSON.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/27/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "SerializeJSON.h"

// TODO: JSON serialization is outdated

using namespace snowcrash;

// Serialize string
static void serialize(const std::string& value, std::ostream &os)
{
    os << "\"" << value << "\"";
}

// Serialize key:value
static void serialize(const std::string& key, const std::string& value, std::ostream &os)
{
    serialize(key, os);
    os << ": ";
    serialize(value, os);
}

// Serialize Metadata
static void serialize(const Collection<Metadata>::type& metadata, std::ostream &os)
{
    if (metadata.empty())
        return;
}

// Serialize Resource Groups
static void serialize(const ResourceGroup& group, std::ostream &os)
{
    os << "{ ";
    
    serialize("name", group.name, os);
    os << ", ";
    
    serialize("description", group.description, os);
    
    os << " }";
}

// Serialize Resource Groups
static void serialize(const Collection<ResourceGroup>::type& resourceGroups, std::ostream &os)
{
    if (resourceGroups.empty())
        return;
    
    serialize("groups", os);
    os << ": [ ";
    
    for (Collection<ResourceGroup>::type::const_iterator it = resourceGroups.begin(); it != resourceGroups.end(); ++it) {
        if (it != resourceGroups.begin())
            os << ", ";
        
        serialize(*it, os);        
    }
    
    os << " ]";
}

static void serialize(const Blueprint& blueprint, std::ostream &os)
{
    os << "{ ";
    
    // Metadata
    if (!blueprint.metadata.empty()) {
        serialize(blueprint.metadata, os);
        os << ", ";
    }

    // Name
    serialize("name", blueprint.name, os);
    os << ", ";

    // Description
    serialize("description", blueprint.description, os);
    if (!blueprint.resourceGroups.empty())
        os << ", ";
    
    // Resource Groups
    serialize(blueprint.resourceGroups, os);
    
    os << " }\n";
}

void snowcrash::SerializeJSON(const snowcrash::Blueprint& blueprint, std::ostream &os)
{
    serialize(blueprint, os);
}

