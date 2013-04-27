//
//  Serialize.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/27/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "Serialize.h"

using namespace snowcrash;

// Serialize string
void serialize(const std::string& value, std::ostream &os)
{
    os << "\"" << value << "\"";
}

// Serialize key:value
void serialize(const std::string& key, const std::string& value, std::ostream &os)
{
    serialize(key, os);
    os << ": ";
    serialize(value, os);
}

// Serialize Metadata
void serialize(const Collection<Metadata>& metadata, std::ostream &os)
{
    if (metadata.empty())
        return;
    
    // TODO:
}

// Serialize Resource Groups
void serialize(const ResourceGroup& group, std::ostream &os)
{
    os << "{ ";
    
    serialize("name", group.name, os);
    os << ", ";
    
    serialize("description", group.description, os);
    
    // TODO: resources
    
    os << " }";
}

// Serialize Resource Groups
void serialize(const Collection<ResourceGroup> resourceGroups, std::ostream &os)
{
    if (resourceGroups.empty())
        return;
    
    serialize("groups", os);
    os << ": [ ";
    
    for (auto it = resourceGroups.begin(); it != resourceGroups.end(); ++it) {
        if (it != resourceGroups.begin())
            os << ", ";
        
        serialize(*it, os);        
    }
    
    os << " ]";
}

void serialize(const Blueprint& blueprint, std::ostream &os)
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

void Serialize(const Blueprint& blueprint, std::ostream &os)
{
    serialize(blueprint, os);
}

