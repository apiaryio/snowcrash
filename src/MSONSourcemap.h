//
//  MSONSourcemap.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/23/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONSOURCEMAP_H
#define SNOWCRASH_MSONSOURCEMAP_H

#include "MSON.h"
#include "MarkdownParser.h"

/**
 * MSON Sourcemap Abstract Syntax Tree
 * -----------------------------------
 *
 * Data types in this document define the MSON Sourcemap AST
 */

namespace snowcrash {

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
}

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

    /** Source Map of Collection of Type Names */
    SOURCE_MAP_COLLECTION(mson::TypeName, mson::TypeNames)

    /** Forward Declaration for Source Map of Element */
    template<>
    struct SourceMap<mson::Element>;

    /** Source Map of Collection of Elements */
    SOURCE_MAP_COLLECTION(mson::Element, mson::Elements)

    /** Source Map structure for Type Section */
    template<>
    struct SourceMap<mson::TypeSection> : public SourceMapBase {

        /** EITHER Source Map of Block Description */
        SourceMap<mson::Markdown> description;

        /** OR Source Map of Literal */
        SourceMap<mson::Literal> value;

        /** OR Source Map of Collection of elements */
        //TODO
    };

    /** Source Map of Collection of Type Sections */
    SOURCE_MAP_COLLECTION(mson::TypeSection, mson::TypeSections)

    /** Source Map structure for Named Type */
    template<>
    struct SourceMap<mson::NamedType> : public SourceMapBase {

        /** Source Map of Type Name */
        SourceMap<mson::TypeName> name;

        /** Source Map of Type Definition */
        SourceMap<mson::TypeDefinition> typeDefinition;

        /** Source Map of Type Sections */
        SourceMap<mson::TypeSections> sections;
    };

    /** Source Map structure for Value Member */
    template<>
    struct SourceMap<mson::ValueMember> : public SourceMapBase {

        /** Source Map of Description */
        SourceMap<mson::Markdown> description;

        /** Source Map of Value Definition */
        SourceMap<mson::ValueDefinition> valueDefinition;

        /** Source Map of Type Sections */
        SourceMap<mson::TypeSections> sections;
    };

    /** Source Map structure for Property Member */
    template<>
    struct SourceMap<mson::PropertyMember> : public SourceMap<mson::ValueMember> {

        /** Source Map for Property Name */
        SourceMap<mson::PropertyName> name;
    };

    /** Source Map structure for One Of */
    // `OneOf` is the same as `Elements`

    /** Source Map structure for Element */
    template<>
    struct SourceMap<mson::Element> : public SourceMapBase {

        /** EITHER Source Map of Property Member */
        SourceMap<mson::PropertyMember> property;

        /** OR Source Map of Value Member */
        SourceMap<mson::ValueMember> value;

        /** OR Source Map of Mixin */
        SourceMap<mson::Mixin> mixin;

        /** OR Source Map of One Of */
        //TODO

        /** OR Source Map of Collection of elements */
        //TODO
    };
}

#endif
