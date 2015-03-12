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
#include "MSONSourcemap.h"
/**
 *  API Blueprint Sourcemap Abstract Syntax Tree
 *  ---------------------------------------------
 *
 *  Data types in this documents define the API Blueprint Sourcemap AST.
 */

namespace snowcrash {

    /** Source Map of Metadata Collection */
    SOURCE_MAP_COLLECTION(Metadata, MetadataCollection)

    /** Source Map of Headers */
    // 'Metadata' type is same as 'Header'

    /** Source Map of Collection of Parameter values */
    SOURCE_MAP_COLLECTION(Value, Values)

    /**
     * Source Map Structure for Parameter
     */
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

    /**
     * Source Map Structure for MSON Parameter
     */
    template<>
    struct SourceMap<MSONParameter> : public SourceMap<Parameter> {
    };

    /** Source Map of Collection of Parameters */
    SOURCE_MAP_COLLECTION(Parameter, Parameters)

    /**
     * Source Map Structure for DataStructure
     */
    template<>
    struct SourceMap<DataStructure> : public SourceMap<mson::NamedType> {
    };

    /** Source Map Structure for Attributes */
    // 'Attributes' is the same as 'DataStructure'

    /**
     * Source Map Structure for Payload
     */
    template<>
    struct SourceMap<Payload> : public SourceMapBase {

        /** Source Map of a Payload Name */
        SourceMap<Name> name;

        /** Source Map of Payload Description */
        SourceMap<Description> description;

        /** Source Map of Payload-specific Parameters */
        SourceMap<Parameters> parameters;

        /** Source Map of Payload-specific Headers */
        SourceMap<Headers> headers;

        /** Source Map of Payload-specific Attributes (THIS SHOULD NOT BE HERE - should be under content) */
        SourceMap<Attributes> attributes;

        /** Source Map of Body (THIS SHOULD NOT BE HERE - should be under content) */
        SourceMap<Asset> body;

        /** Source Map of Schema (THIS SHOULD NOT BE HERE - should be under content) */
        SourceMap<Asset> schema;

        /** Source Map of Model Reference */
        SourceMap<Reference> reference;
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

        /** Action-specific Attributes (THIS SHOULD NOT BE HERE - should be under content) */
        SourceMap<Attributes> attributes;

        /** Source Map of URI Template (THIS SHOULD NOT BE HERE - should be under element attributes) */
        SourceMap<URITemplate> uriTemplate;

        /** Source Map of Link Relation (THIS SHOULD NOT BE HERE - should be under element attributes) */
        SourceMap<Relation> relation;

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
    struct SourceMap<Resource> {

        /** Source Map of URI template */
        SourceMap<URITemplate> uriTemplate;

        /** Source Map of a Resource Name */
        SourceMap<Name> name;

        /** Source Map of Description of the resource */
        SourceMap<Description> description;

        /** Model representing this Resource */
        SourceMap<ResourceModel> model;

        /** Source Map of Resource-specific Attributes (THIS SHOULD NOT BE HERE - should be under content) */
        SourceMap<Attributes> attributes;

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

    /** Forward Declaration for Source Map of Element */
    template<>
    struct SourceMap<Element>;

    /** Source Map of Collection of Elements */
    SOURCE_MAP_COLLECTION(Element, Elements)

    /**
     * Source Map Structure for Element
     */
    template<>
    struct SourceMap<Element> : public SourceMapBase {

        /** Source Map Structure for Attributes of the Element */
        struct Attributes {

            /** Source Map of a Element Name */
            SourceMap<Name> name;
        };

        /** Source Map Structure for Content of the Element */
        struct Content {

            /** EITHER Source Map of Copy */
            SourceMap<std::string> copy;

            /** OR Source Map of Resource */
            SourceMap<Resource> resource;

            /** OR Source Map of Data Structure */
            SourceMap<DataStructure> dataStructure;

            /** OR Source Map of Collection of elements */
            SourceMap<Elements>& elements();
            const SourceMap<Elements>& elements() const;

            /** Constructor */
            Content();

            /** Copy constructor */
            Content(const SourceMap<Element>::Content& rhs);

            /** Assignment operator */
            SourceMap<Element>::Content& operator=(const SourceMap<Element>::Content& rhs);

            /** Destructor */
            ~Content();

        private:
            std::auto_ptr<SourceMap<Elements> > m_elements;
        };

        /** Class of the Element (to be used internally only) */
        Element::Class element;

        /** Source Map of Attributes of the Element */
        Attributes attributes;

        /** Source Map of Content of the Element */
        Content content;

        /** Type of the Category element (to be used internally only) */
        Element::Category category;

        /** Constructor */
        SourceMap(const Element::Class& element_ = Element::UndefinedElement);

        /** Copy constructor */
        SourceMap(const SourceMap<Element>& rhs);

        /** Assignment operator */
        SourceMap<Element>& operator=(const SourceMap<Element>& rhs);

        /** Destructor */
        ~SourceMap();
    };

    /**
     *  Source Map Structure for Group of API Resources (Category Element)
     */
    template<>
    struct SourceMap<ResourceGroup> : public SourceMap<Element> {
    };

    /**
     * Source Map Structure for Group of Data Structures (Category Element)
     */
    template<>
    struct SourceMap<DataStructureGroup> : public SourceMap<Element> {
    };

    /**
     *  \brief API Blueprint Sourcemap AST
     *
     *  This is top-level (or root if you prefer) of API Blueprint Sourcemap abstract syntax tree.
     *  Start reading a parsed API here.
     */
    template<>
    struct SourceMap<Blueprint> : public SourceMap<Element> {

        /** Source Map of the API Name */
        SourceMap<Name> name;

        /** Source Map of API Blueprint metadata */
        SourceMap<MetadataCollection> metadata;

        /** Source Map of an API Overview description */
        SourceMap<Description> description;
    };
}

#undef SOURCE_MAP_COLLECTION

#endif
