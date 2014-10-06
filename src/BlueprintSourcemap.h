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

#define SOURCE_MAP_COLLECTION(T, TC) template<>\
struct SourceMap<TC> {\
    Collection<SourceMap<T> >::type collection;\
};\

namespace snowcrash {

    struct SourceMapBase {
        mdp::BytesRangeSet sourceMap;
    };

    template<typename T>
    struct SourceMap : public SourceMapBase {
    };

    /** Source Map of Metadata Collection */
    SOURCE_MAP_COLLECTION(Metadata, MetadataCollection)

    /** Source Map of Headers */
    // 'Metadata' type is same as 'Header'

    /** Source Map of Collection of Parameter values */
    SOURCE_MAP_COLLECTION(Value, Values)

    /** Source Map Structure for Parameter */
    template<>
    struct SourceMap<Parameter> : public SourceMapBase {

        /** Source Map of Parameter Name */
        SourceMap<Name> name;

        /** Source Map of Parameter Description */
        SourceMap<Description> description;

        /** Source Map of Parameter Type */
        SourceMap<Type> type;

        /** Source Map of Required flag */
        SourceMap<ParameterUse> use;

        /** Source Map of Default Value, applicable only when `required == false` */
        SourceMap<Value> defaultValue;

        /** Source Map of Example Value */
        SourceMap<Value> exampleValue;

        /** Enumeration of possible values */
        SourceMap<Values> values;
    };

    /** Source Map of Collection of Parameters */
    SOURCE_MAP_COLLECTION(Parameter, Parameters)

    /**
     * Source Map Structure for Payload
     */
    template<>
    struct SourceMap<Payload> : public SourceMapBase {

        /** Source Map of a Payload Name */
        SourceMap<Name> name;

        /** Source Map of Payload Description */
        SourceMap<Description> description;

        /** Payload-specific Parameters */
        SourceMap<Parameters> parameters;

        /** Payload-specific Headers */
        SourceMap<Headers> headers;

        /** Source Map of Body */
        SourceMap<Asset> body;

        /** Source Map of Schema */
        SourceMap<Asset> schema;

        /** Source Map of Symbol */
        SourceMap<Identifier> symbol;
    };

    /** Source Map of Collection of Requests */
    SOURCE_MAP_COLLECTION(Request, Requests)

    /** Source Map of Collection of Responses */
    // 'Response' type is same as 'Request'

    /**
     *  Source Map Structure for an HTTP transaction example.
     */
    template<>
    struct SourceMap<TransactionExample> : public SourceMapBase {

        /** Source Map of an example name */
        SourceMap<Name> name;

        /** Source Map of Description */
        SourceMap<Description> description;

        /** Requests */
        SourceMap<Requests> requests;

        /** Responses */
        SourceMap<Responses> responses;
    };

    /** Source Map of Collection of Transaction examples */
    SOURCE_MAP_COLLECTION(TransactionExample, TransactionExamples)

    /**
     *  Source Map Structure for Action
     */
    template<>
    struct SourceMap<Action> : public SourceMapBase {

        /** Source Map of HTTP method */
        SourceMap<HTTPMethod> method;

        /** Source Map of an Action name */
        SourceMap<Name> name;

        /** Source Map of Description */
        SourceMap<Description> description;

        /** Action-specific Parameters */
        SourceMap<Parameters> parameters;

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
        DEPRECATED SourceMap<Headers> headers;

        /** Transactions examples */
        SourceMap<TransactionExamples> examples;
    };

    /** Source Map of Collection of Actions */
    SOURCE_MAP_COLLECTION(Action, Actions)

    /**
     *  Source Map Structure for API Resource
     */
    template<>
    struct SourceMap<Resource> : public SourceMapBase {

        /** Source Map of URI template */
        SourceMap<URITemplate> uriTemplate;

        /** Source Map of a Resource Name */
        SourceMap<Name> name;

        /** Source Map of Description of the resource */
        SourceMap<Description> description;

        /** Model representing this Resource */
        SourceMap<ResourceModel> model;

        /** Parameters */
        SourceMap<Parameters> parameters;

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
        DEPRECATED SourceMap<Headers> headers;

        /** A set of Actions specified for this Resource */
        SourceMap<Actions> actions;
    };

    /** Source Map of Collection of Resources */
    SOURCE_MAP_COLLECTION(Resource, Resources)

    /**
     *  Source Map Structure for Group of API Resources
     */
    template<>
    struct SourceMap<ResourceGroup> : public SourceMapBase {

        /** Source Map of a Group Name */
        SourceMap<Name> name;

        /** Source Map of Group description */
        SourceMap<Description> description;

        /** Resources */
        SourceMap<Resources> resources;
    };

    /** Source Map of Collection of Resource groups */
    SOURCE_MAP_COLLECTION(ResourceGroup, ResourceGroups)

    /**
     *  \brief API Blueprint Sourcemap AST
     *
     *  This is top-level (or root if you prefer) of API Blueprint Sourcemap abstract syntax tree.
     *  Start reading a parsed API here.
     */
    template<>
    struct SourceMap<Blueprint> : public SourceMapBase {

        /** Source Map of API Blueprint metadata */
        SourceMap<MetadataCollection> metadata;

        /** Source Map of the API Name */
        SourceMap<Name> name;

        /** Source Map of an API Overview description */
        SourceMap<Description> description;

        /** The set of API Resource Groups */
        SourceMap<ResourceGroups> resourceGroups;
    };
}

#endif
