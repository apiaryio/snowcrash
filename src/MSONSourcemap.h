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

/**
 * MSON Sourcemap Abstract Syntax Tree
 * -----------------------------------
 *
 * Data types in this document define the MSON Sourcemap AST
 */

namespace snowcrash {

    /** Source Map of Collection of Values */
    SOURCE_MAP_COLLECTION(mson::Value, mson::Values)

    /** Source Map of Collection of Type Names */
    SOURCE_MAP_COLLECTION(mson::TypeName, mson::TypeNames)

    /** Source Map structure for Value Definition */
    template<>
    struct SourceMap<mson::ValueDefinition> : public SourceMapBase {

        /** Source Map of Type Definition */
        SourceMap<mson::TypeDefinition> typeDefinition;
    };

    /** Source Map structure for Named Type */
    template<>
    struct SourceMap<mson::NamedType> {

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

    /** Source Map structure for Mixin */
    template<>
    struct SourceMap<mson::Mixin> : public SourceMapBase {

        /** Source Map for Type Definition */
        SourceMap<mson::TypeDefinition> typeDefinition;
    };
}

#endif
