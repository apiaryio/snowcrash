//
//  Blueprint.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/3/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_BLUEPRINT_H
#define SNOWCRASH_BLUEPRINT_H

#include <vector>
#include <string>
#include <utility>

namespace snowcrash {
    
    //
    // Generic types
    //
    
    // Name of section / element. Plain text
    using Name = std::string;

    // Section Description. Rendered HTML from Markdown
    using Description = std::string;
    
    // Default Container for collections
    template<typename T>
    using Collection = std::vector<T>;
    
    //
    // API Blueprint sections
    //
    class Resource {};
    class Request {};
    class Response {};
    class Payload {};
    class Asset {};
    class Parameter {};
    
    // Group of resources
    struct ResourceGroup {
        
        // Group Name
        Name name;
        
        // Group Description
        Description description;
        
        // Resources
        Collection<Resource> resources;
    };

    // Metadata key-value pair, e.g. "HOST: http://acme.com"
    using Metadata = std::pair<std::string, std::string>;
    
    // API Blueprint
    struct Blueprint {
        
        // Metadata
        Collection<Metadata> metadata;
        
        // API Name
        Name name;

        // API Overview
        Description description;
        
        // Resource Groups
        Collection<ResourceGroup> resourceGroups;
    };
}

#endif
