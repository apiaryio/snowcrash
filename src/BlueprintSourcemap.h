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
#include "MarkdownParser.h"

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

    /** Source Map of an asset data */
    typedef mdp::BytesRangeSet AssetSM;

    /** Source Map of Metadata */
    typedef mdp::BytesRangeSet MetadataSM;

    /** Source Map of Header */
    typedef mdp::BytesRangeSet HeaderSM;

    /** Source Map of Metadata Collection */
    typedef Collection<MetadataSM>::type MetadataCollectionSM;

    /** Source Map of Headers */
    typedef Collection<HeaderSM>::type HeadersSM;

    /** Source Map of Collection of Parameter values */
    typedef Collection<ValueSM>::type ValuesSM;

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
        ValuesSM values;
    };

    /** Source Map of Collection of Parameters */
    typedef Collection<ParameterSM>::type ParametersSM;

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
        ParametersSM parameters;

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

    /** Source Map structure for Response */
    typedef PayloadSM ResponseSM;

    /** Source Map of Collection of Requests */
    typedef Collection<RequestSM>::type RequestsSM;

    /** Source Map of Collection of Responses */
    typedef Collection<ResponseSM>::type ResponsesSM;

    /**
     *  Source Map Structure for an HTTP transaction example.
     */
    struct TransactionExampleSM {

        /** Source Map of an example name */
        NameSM name;

        /** Source Map of Description */
        DescriptionSM description;

        /** Requests */
        RequestsSM requests;

        /** Responses */
        ResponsesSM responses;
    };

    /** Source Map of Collection of Transaction examples */
    typedef Collection<TransactionExampleSM>::type TransactionExamplesSM;

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
        ParametersSM parameters;

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
        TransactionExamplesSM examples;
    };

    /** Source Map of Collection of Actions */
    typedef Collection<ActionSM>::type ActionsSM;

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
        ParametersSM parameters;

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
        ActionsSM actions;
    };

    /** Source Map of Collection of Resources */
    typedef Collection<ResourceSM>::type ResourcesSM;

    /**
     *  Source Map Structure for Group of API Resources
     */
    struct ResourceGroupSM {

        /** Source Map of a Group Name */
        NameSM name;

        /** Source Map of Group description */
        DescriptionSM description;

        /** Resources */
        ResourcesSM resources;
    };

    /** Source Map of Collection of Resource groups */
    typedef Collection<ResourceGroupSM>::type ResourceGroupsSM;

    /**
     *  \brief API Blueprint Sourcemap AST
     *
     *  This is top-level (or root if you prefer) of API Blueprint Sourcemap abstract syntax tree.
     *  Start reading a parsed API here.
     */
    struct BlueprintSM {

        /** Source Map of API Blueprint metadata */
        MetadataCollectionSM metadata;

        /** Source Map of the API Name */
        NameSM name;

        /** Source Map of an API Overview description */
        DescriptionSM description;

        /** The set of API Resource Groups */
        ResourceGroupsSM resourceGroups;
    };
}

#endif
