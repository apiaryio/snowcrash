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
#include "Platform.h"
#include "MarkdownNode.h"
#include "MSONSourcemap.h"

/**
 *  API Blueprint Abstract Syntax Tree
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

    /** Parameter Type */
    typedef std::string Type;

    /** Parameter Value */
    typedef std::string Value;

    /** A generic key - value pair */
    typedef std::pair<std::string, std::string> KeyValuePair;

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

    /** Metadata Collection */
    typedef Collection<Metadata>::type MetadataCollection;

    /** Headers */
    typedef Collection<Header>::type Headers;

    /** Collection of Parameter Values */
    typedef Collection<Value>::type Values;

    /** Parameter Use flag */
    enum ParameterUse {
        UndefinedParameterUse,
        OptionalParameterUse,
        RequiredParameterUse
    };

    /** Parameter */
    struct Parameter {

        /** Parameter Name */
        Name name;

        /** Parameter Description */
        Description description;

        /** Type */
        Type type;

        /** Required flag */
        ParameterUse use;

        /** Default Value, applicable only when `required == false` */
        Value defaultValue;

        /** Example Value */
        Value exampleValue;

        /** Enumeration of possible values */
        Values values;
    };

    /** Source Map of Collection of Parameters */
    typedef Collection<Parameter>::type Parameters;

    /** Identifier(name) of Reference */
    typedef std::string Identifier;

    /** Reference */
    struct Reference {

        /** Reference Resolution State */
        enum State {
            StateUnresolved,    // Reference unresolved (undefined)
            StatePending,       // Reference resolution pending
            StateResolved       // Reference resolved successfully
        };

        /** Reference Type */
        enum ReferenceType {
            ModelReference  // Resource Model as a reference
        };

        /** Identifier */
        Identifier id;

        /** Type */
        ReferenceType type;

        struct ReferenceMetadata {

            /** Constructor */
            ReferenceMetadata(State state_ = StateUnresolved)
            : state(state_) {}

            /** Markdown AST reference source node (for source map) */
            mdp::MarkdownNodeIterator node;

            /** Reference resolution state */
            State state;
        };

        /** Metadata for the reference */
        ReferenceMetadata meta;
    };

    /**
     * Data Structure
     */
    struct DataStructure {

        /** As described in source */
        mson::NamedType source;

        /** As resolved by subsequent tooling */
        mson::NamedType resolved;
    };

    /**
     *  Attributes
     */
    typedef DataStructure Attributes;

    /**
     *  Payload
     */
    struct Payload {

        /** A Payload Name */
        Name name;

        /** Payload Description */
        Description description;

        /** Payload-specific Parameters */
        Parameters parameters;

        /** Payload-specific Headers */
        Headers headers;

        /** Payload-specific Attributes */
        Attributes attributes;

        /** Body */
        Asset body;

        /** Schema */
        Asset schema;

        /** Reference */
        Reference reference;
    };

    /** Resource Model */
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

    /** Collection of Requests */
    typedef Collection<Request>::type Requests;

    /** Collection of Responses */
    typedef Collection<Response>::type Responses;

    /**
     *  An HTTP transaction example.
     */
    struct TransactionExample {

        /** An example name */
        Name name;

        /** Description */
        Description description;

        /** Requests */
        Requests requests;

        /** Responses */
        Responses responses;
    };

    /** Collection of Transaction examples */
    typedef Collection<TransactionExample>::type TransactionExamples;

    /**
     *  Action
     */
    struct Action {

        /** HTTP method */
        HTTPMethod method;

        /** An Action name */
        Name name;

        /** Description */
        Description description;

        /** Action-specific Parameters */
        Parameters parameters;

        /**
         *  \brief Action-specific HTTP headers
         *
         *  DEPRECATION WARNING:
         *  --------------------
         *
         *  This AST node is build for deprecated API Blueprint syntax
         *  and as such it will be removed in a future version of
         *  Snow Crash.
         *
         *  Use respective payload's header collection instead.
         */
        DEPRECATED Headers headers;

        /** Transactions examples */
        TransactionExamples examples;
    };

    /** Collection of Actions */
    typedef Collection<Action>::type Actions;

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
        Parameters parameters;

        /**
         *  \brief Resource-specific HTTP Headers
         *
         *  DEPRECATION WARNING:
         *  --------------------
         *
         *  This AST node is build for deprecated API Blueprint syntax
         *  and as such it will be removed in a future version of
         *  Snow Crash.
         *
         *  Use respective payload's header collection instead.
         */
        DEPRECATED Headers headers;

        /** A set of Actions specified for this Resource */
        Actions actions;
    };

    /** Collection of Resources */
    typedef Collection<Resource>::type Resources;

    /**
     *  Group of API Resources
     */
    struct ResourceGroup {

        /** A Group Name */
        Name name;

        /** Group description */
        Description description;

        /** Resources */
        Resources resources;
    };

    /** Collection of Resource groups */
    typedef Collection<ResourceGroup>::type ResourceGroups;

    /**
     *  \brief API Blueprint AST
     *
     *  This is top-level (or root if you prefer) of API Blueprint abstract syntax tree.
     *  Start reading a parsed API here.
     */
    struct Blueprint {

        /** Metadata */
        MetadataCollection metadata;

        /** The API Name */
        Name name;

        /** An API Overview description */
        Description description;

        /** The set of API Resource Groups */
        ResourceGroups resourceGroups;
    };
}

#endif
