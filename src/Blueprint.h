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
#include "MSON.h"

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

    /** Metadata Collection */
    typedef Collection<Metadata>::type MetadataCollection;

    /** Headers */
    typedef Collection<Header>::type Headers;

    /** Collection of Parameter Values */
    typedef Collection<Value>::type Values;

    /** Role of an element */
    enum AssetRole {
        UndefinedAssetRole = 0,   // Unknown
        BodyExampleAssetRole,     // Body example for asset
        BodySchemaAssetRole       // Body schema for asset
    };

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

    /** Parameter with MSON-like syntax */
    struct MSONParameter : public Parameter {
    };

    /** Collection of Parameters */
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
    struct DataStructure : public mson::NamedType {

        /** Assignment operator for Named Type */
        DataStructure& operator=(const mson::NamedType& rhs);
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
     *  Link relation
     */
    struct Relation {

        /** String */
        std::string str;
    };

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

        /** Action-specific Attributes */
        Attributes attributes;

        /** URI Template */
        URITemplate uriTemplate;

        /** Link Relation */
        Relation relation;

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

        /** Resource-specific Attributes */
        Attributes attributes;

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

    /** Forward declaration of Element */
    struct Element;

    /** Collection of elements */
    typedef std::vector<Element> Elements;

    /** Element */
    struct Element {

        /** Class of an element */
        enum Class {
            UndefinedElement = 0, // Unknown
            CategoryElement,      // Group of other elements
            CopyElement,          // Human readable text
            AssetElement,         // Asset of API description
            ResourceElement,      // Resource
            DataStructureElement  // Data Structure
        };

        /** Attributes of an element */
        struct Attributes {

            /** Human readable name of the element */
            Name name;
        };

        /** Content of an element */
        struct Content {

            /** EITHER Copy */
            std::string copy;

            /** OR Resource */
            Resource resource;

            /** OR Data Structure */
            DataStructure dataStructure;

            /** OR Collection of elements */
            Elements& elements();
            const Elements& elements() const;

            /** Constructor */
            Content();

            /** Copy constructor */
            Content(const Element::Content& rhs);

            /** Assignment operator */
            Content& operator=(const Element::Content& rhs);

            /** Destructor */
            ~Content();

        private:
            std::unique_ptr<Elements> m_elements;
        };

        /** Type of Category element (parser internal flag) */
        enum Category {
            UndefinedCategory = 0,     // Unknown
            ResourceGroupCategory,     // Resource Group
            DataStructureGroupCategory // Data Structure Group
        };

        /** Type of the element */
        Element::Class element;

        /** Attributes of the element */
        Element::Attributes attributes;

        /** Content of the element */
        Element::Content content;

        /** Type of Category element (to be used internally only) */
        Element::Category category;

        /** Constructor */
        Element(const Element::Class& element_ = Element::UndefinedElement);

        /** Copy constructor */
        Element(const Element& rhs);

        /** Assignment operator */
        Element& operator=(const Element& rhs);

        /** Destructor */
        ~Element();
    };

    /**
     *  Group of API Resources (Category Element)
     */
    struct ResourceGroup : public Element {
    };

    /**
     * Group of Data Structures (Category Element)
     */
    struct DataStructureGroup : public Element {
    };

    /**
     *  \brief API Blueprint AST
     *
     *  This is top-level (or root if you prefer) of API Blueprint abstract syntax tree.
     *  Start reading a parsed API here.
     */
    struct Blueprint : public Element {

        /** Metadata */
        MetadataCollection metadata;

        /** The API Name */
        Name name;

        /** An API Overview description */
        Description description;
    };
}

#endif
