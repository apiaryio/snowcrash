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
    
    // URI
    using URI = std::string;
    
    // URI template
    using URITemplate = std::string;
    
    // HTTP Method
    using HTTPMethod = std::string;
    
    // Default Container for collections
    template<typename T>
    using Collection = std::vector<T>;
    
    //
    // API Blueprint sections
    //
    
    // Asset data
    using Asset = std::string;

    // Metadata key-value pair, e.g. "HOST: http://acme.com"
    using Metadata = std::pair<std::string, std::string>;

    // Header key-value pair, e.g. "Content-Type: application/json"
    using Header = std::pair<std::string, std::string>;
    
    // Parameter
    struct Parameter {
        
        // Group Name
        Name name;
        
        // Group Description
        Description description;
        
        // TODO: type, optional, default value
    };
    
    // Payload
    struct Payload {
        
        // Group Name
        Name name;
        
        // Group Description
        Description description;
        
        // Parameters
        Collection<Parameter> parameters;
        
        // Headers
        Collection<Header> headers;
        
        // Body
        Asset body;
        
        // Schema
        Asset schema;
    };
    
    // Request
    using Request = Payload;
    
    // Response, a payload where name is HTTP status code
    using Response = Payload;
    
    // Method
    struct Method {
        
        // HTTP method
        HTTPMethod method;
        
        // Description
        Description description;
        
        // Parameters
        Collection<Parameter> parameters;
        
        // Headers
        Collection<Header> headers;

        // Requests
        Collection<Request> requests;
        
        // Responses
        Collection<Response> responses;
    };
    
    // Resource
    struct Resource {
        
        // URI template
        URITemplate uri;
        
        // Description
        Description description;
        
        // Parameters
        Collection<Parameter> parameters;
        
        // Headers
        Collection<Header> headers;
        
        // Methods
        Collection<Method> methods;
    };
    
    // Group of resources
    struct ResourceGroup {
        
        // Group Name
        Name name;
        
        // Group Description
        Description description;
        
        // Resources
        Collection<Resource> resources;
    };
    
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
