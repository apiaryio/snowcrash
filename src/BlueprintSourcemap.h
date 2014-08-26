//
//  BlueprintSourcemap.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 26/8/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_BLUEPRINT_SOURCEMAP_H
#define SNOWCRASH_BLUEPRINT_SOURCEMAP_H

#include "Blueprint.h"

/**
 *  API Blueprint Sourcemap Abstract Syntax Tree
 *  ---------------------------------------------
 *
 *  Data types in this documents define the API Blueprint Sourcemap AST.
 */

namespace snowcrash {

    /** Source Map of Name of a an API Blueprint entity. */
    typedef mdp::BytesRangeSet NameSM;

    /** Source Map of an API Blueprint entity Description. */
    typedef mdp::BytesRangeSet DescriptionSM;

    /** Source Map of URI template */
    typedef mdp::BytesRangeSet URITemplateSM;

    /** Source Map of HTTP Method */
    typedef mdp::BytesRangeSet HTTPMethodSM;

    /** Source Map of Parameter Type */
    typedef mdp::BytesRangeSet TypeSM;

    /** Source Map of Parameter Value */
    typedef mdp::BytesRangeSet ValueSM;

    /** Standard types for array of Source Maps */
    Collection<mdp::BytesRangeSet>::type SourceMaps;
    Collection<mdp::BytesRangeSet>::iterator SourceMapsIterator;

    /** Source Map of an asset data */
    typedef mdp::BytesRangeSet AssetSM;

    /** Source Map of Metadata */
    typedef SourceMaps MetadataSM;

    /** Source Map of Headers */
    typedef SourceMaps HeadersSM;

    /** Source Map of Parameter Use flag */
    typedef mdp::BytesRangeSet ParameterUseSM;

    /** Source Map Structure for Parameter */
    struct ParameterSM {

        /** Source Map of Parameter Name */
        NameSM name;

        /** Source Map of Parameter Description */
        DescriptionSM description;

        /** Source Map of Parameter Type */
        TypeSM type;

        /** Source Map of Required flag */
        ParameterUseSM use;

        /** Source Map of Default Value, applicable only when `required == false` */
        ValueSM defaultValue;

        /** Source Map of Example Value */
        ValueSM exampleValue;

        /** Enumeration of possible values */
        Collection<ValueSM>::type values;
    };

    /** Source Map of Name of a symbol */
    typedef mdp::BytesRangeSet SymbolNameSM;

    /**
     * Source Map Structure for Payload
     */
    struct PayloadSM {

        /** Source Map of a Payload Name */
        NameSM name;

        /** Source Map of Payload Description */
        DescriptionSM description;

        /** Payload-specific Parameters */
        Collection<Parameter>::type parameters;

        /** Payload-specific Headers */
        HeadersSM headers;

        /** Source Map of Body */
        AssetSM body;

        /** Source Map of Schema */
        AssetSM schema;

        /** Source Map of Symbol */
        SymbolNameSM symbol;
    };

    /** Source Map structure for Resource Model */
    typedef PayloadSM ResourceModelSM;

    /** Source Map structure for Request */
    typedef PayloadSM RequestSM;

    /**
     *  \brief Source Map structure for Response
     *
     *  A payload returned in a response to an action.
     *  Payload's name represents the HTTP status code.
     */
    typedef PayloadSM ResponseSM;

    /**
     *  Source Map Structure for an HTTP transaction example.
     */
    struct TransactionExampleSM {

        /** Source Map of an example name */
        NameSM name;

        /** Source Map of Description */
        DescriptionSM description;

        /** Requests */
        Collection<RequestSM>::type requests;

        /** Responses */
        Collection<ResponseSM>::type responses;
    };

    /**
     *  Source Map Structure for Action
     */
    struct ActionSM {

        /** Source Map of HTTP method */
        HTTPMethodSM method;

        /** Source Map of an Action name */
        NameSM name;

        /** Source Map of Description */
        DescriptionSM description;

        /** Action-specific Parameters */
        Collection<ParameterSM>::type parameters;

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
        DEPRECATED HeadersSM headers;

        /** Transactions examples */
        Collection<TransactionExampleSM>::type examples;
    };

    /**
     *  Source Map Structure for API Resource
     */
    struct ResourceSM {

        /** Source Map of URI template */
        URITemplateSM uriTemplate;

        /** Source Map of a Resource Name */
        NameSM name;

        /** Source Map of Description of the resource */
        DescriptionSM description;

        /** Model representing this Resource */
        ResourceModelSM model;

        /** Parameters */
        Collection<ParameterSM>::type parameters;

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
        DEPRECATED HeadersSM headers;

        /** A set of Actions specified for this Resource */
        Collection<ActionSM>::type actions;
    };

    /**
     *  Source Map Structure for Group of API Resources
     */
    struct ResourceGroupSM {

        /** Source Map of a Group Name */
        NameSM name;

        /** Source Map of Group description */
        DescriptionSM description;

        /** Resources */
        Collection<ResourceSM>::type resources;
    };

    /**
     *  \brief API Blueprint Sourcemap AST
     *
     *  This is top-level (or root if you prefer) of API Blueprint Sourcemap abstract syntax tree.
     *  Start reading a parsed API here.
     */
    struct BlueprintSM {

        /** Source Map of API Blueprint metadata */
        MetadataSM metadata;

        /** Source Map of the API Name */
        NameSM name;

        /** Source Map of an API Overview description */
        DescriptionSM description;

        /** The set of API Resource Groups */
        Collection<ResourceGroupSM>::type resourceGroups;
    };
}

#endif
