//
//  Blueprint.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/3/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
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
    typedef std::string Name;

    // Section Description. Rendered HTML from Markdown
    typedef std::string Description;
    
    // URI
    typedef std::string URI;
    
    // URI template
    typedef std::string URITemplate;
    
    // HTTP Method
    typedef std::string HTTPMethod;
    
    // Key:Value pair
    typedef std::pair<std::string, std::string> KeyValuePair;
    
    // Default Container for collections
    // FIXME: C++11 template aliases
    template<typename T>
    struct Collection {
        typedef std::vector<T> type;
        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;
    };
    
    //
    // API Blueprint sections
    //
    
    // Asset data
    typedef std::string Asset;

    // Metadata key-value pair, e.g. "HOST: http://acme.com"
    typedef KeyValuePair Metadata;

    // Header key-value pair, e.g. "Content-Type: application/json"
    typedef KeyValuePair Header;
    
    // Parameter
    struct Parameter {
        
        // Parameter Name
        Name name;
        
        // Parameter Description
        Description description;
        
        // TODO: type, optional, default value
    };
    
    // Payload
    struct Payload {
        
        // Payload Name
        Name name;
        
        // Payload Description
        Description description;
        
        // Parameters
        Collection<Parameter>::type parameters;
        
        // Headers
        Collection<Header>::type headers;
        
        // Body
        Asset body;
        
        // Schema
        Asset schema;
    };
    
    // Resource Object
    typedef Payload ResourceObject;
    
    // Request
    typedef Payload Request;
    
    // Response, a payload where name is HTTP status code
    typedef Payload Response;
    
    // Method
    struct Method {
        
        // HTTP method
        HTTPMethod method;
        
        // Method name
        Name name;
        
        // Description
        Description description;
        
        // Parameters
        Collection<Parameter>::type parameters;
        
        // Headers
        Collection<Header>::type headers;

        // Requests
        Collection<Request>::type requests;
        
        // Responses
        Collection<Response>::type responses;
    };
    
    // Resource
    struct Resource {
        
        // URI template
        // TODO: remane uri -> uriTemplate
        URITemplate uri;
        
        // Resource Name
        Name name;
        
        // Description
        Description description;
        
        // Object represented by this resource
        ResourceObject object;
        
        // Parameters
        Collection<Parameter>::type parameters;
        
        // Headers
        Collection<Header>::type headers;
        
        // Methods
        Collection<Method>::type methods;
    };
    
    // Group of resources
    struct ResourceGroup {
        
        // Group Name
        Name name;
        
        // Group Description
        Description description;
        
        // Resources
        Collection<Resource>::type resources;
    };
    
    // API Blueprint
    struct Blueprint {
        
        // Metadata
        Collection<Metadata>::type metadata;
        
        // API Name
        Name name;

        // API Overview
        Description description;
        
        // Resource Groups
        Collection<ResourceGroup>::type resourceGroups;
    };
}

#endif
