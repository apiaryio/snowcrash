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

/**
 *  API Blueprint Abstract Syntaxt Tree
 *  -----------------------------------
 *
 *  Data types in this documents define the API Blueprint AST.
 */

namespace snowcrash {
    
    /** Name of a an API Blueprint entity. */
    typedef std::string Name;

    /** 
     *  \brief An API Blueprint entity Description.
     *
     *  Depending on parser setting the description might be 
     *  rendered HTML from Markdown or raw Markdown.
     */
    typedef std::string Description;
    
    /** URI */
    typedef std::string URI;
    
    /** URI template */
    typedef std::string URITemplate;
    
    /** HTTP Method */
    typedef std::string HTTPMethod;
    
    /** A generic key - value pair */
    typedef std::pair<std::string, std::string> KeyValuePair;
    
    /**
     * Default Container for collections.
     *
     *  FIXME: Use C++11 template aliases when migrating to C++11.
     */
    template<typename T>
    struct Collection {
        typedef std::vector<T> type;
        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;
    };
    
    /** An asset data */
    typedef std::string Asset;

    /** 
     *  \brief Metadata key-value pair,
     *
     *  E.g. "HOST: http://acme.com"
     */
    typedef KeyValuePair Metadata;

    /** 
     *  \brief  Header key-value pair.
     *
     *  E.g. "Content-Type: application/json"
     */
    typedef KeyValuePair Header;
    
    /** Parameter */
    struct Parameter {
        
        /** Parameter Name */
        Name name;
        
        /** Parameter Description */
        Description description;
        
        // TODO: type, optional, default value
    };
    
    /**
     *  Payload
     */
    struct Payload {
        
        /** A Payload Name */
        Name name;
        
        /** Payload Description */
        Description description;
        
        /** Payload-specific Parameters */
        Collection<Parameter>::type parameters;
        
        /** Payload-specific Headers */
        Collection<Header>::type headers;
        
        /** Body */
        Asset body;
        
        /** Schema */
        Asset schema;
    };
    
    /** Resource Object */
    typedef Payload ResourceModel;
    
    /** Request */
    typedef Payload Request;
    
    /** 
     *  \brief Response
     *
     *  A payload returned in a response to an action.
     *  Payload's name represents the HTTP status code.
     */
    typedef Payload Response;
    
    /**
     *  Method
     */
    struct Method {
        
        /** HTTP method */
        HTTPMethod method;
        
        /** A Method name */
        Name name;
        
        /** Description */
        Description description;
        
        /** Method-specfic Parameters */
        Collection<Parameter>::type parameters;
        
        /** Method-specific HTTP headers */
        Collection<Header>::type headers;

        /** Requests */
        Collection<Request>::type requests;
        
        /** Responses */
        Collection<Response>::type responses;
    };
    
    /**
     *  API Resource
     */
    struct Resource {
        
        /** URI template */
        URITemplate uriTemplate;
        
        /** A Resource Name */
        Name name;
        
        /** Description of the resource */
        Description description;
        
        /** Model representing this Resource */
        ResourceModel model;
        
        /** Parameters */
        Collection<Parameter>::type parameters;
        
        /** Resource-specific HTTP Headers */
        Collection<Header>::type headers;
        
        /** A set of HTTP Methods specified for this Resource */
        Collection<Method>::type methods;
    };
    
    /**
     *  Group of API Resources
     */
    struct ResourceGroup {
        
        /** A Group Name */
        Name name;
        
        /** Group description */
        Description description;
        
        /** Resources */
        Collection<Resource>::type resources;
    };
    
    /** 
     *  \brief API Blueprint AST
     *
     *  This is top-level (or root if you prefer) of API Blueprint abstract syntax tree.
     *  Start reading a parsed API here.
     */
    struct Blueprint {
        
        /** Metadata */
        Collection<Metadata>::type metadata;
        
        /** The API Name */
        Name name;

        /** An API Overview description */
        Description description;
        
        /** The set of API Resource Groups */
        Collection<ResourceGroup>::type resourceGroups;
    };
}

#endif
