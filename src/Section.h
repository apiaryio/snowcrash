//
//  Section.h
//  snowcrash
//
//  Created by Zdenek Nemec on 11/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SECTION_H
#define SNOWCRASH_SECTION_H

#include <string>
#include "MarkdownNode.h"

namespace snowcrash {
    
    /**
     *  API Blueprint Sections Types.
     */
    enum SectionType {
        UndefinedSectionType,           /// < Undefined section
        BlueprintSectionType,           /// < Blueprint overview
        ResourceGroupSectionType,       /// < Resource group
        ResourceSectionType,            /// < Resource
        ResourceMethodSectionType,      /// < Resource & Action combined (abbrev)
        ActionSectionType,              /// < Action
        RequestSectionType,             /// < Request
        RequestBodySectionType,         /// < Request & Payload body combined (abbrev)
        ResponseSectionType,            /// < Response
        ResponseBodySectionType,        /// < Response & Body combined (abbrev)
        ObjectSectionType,              /// < Object (deprecated)
        ObjectBodySectionType,          /// < Object & Body combined (abbrev, deprecated)
        ModelSectionType,               /// < Model
        ModelBodySectionType,           /// < Model & Body combined (abbrev)
        BodySectionType,                /// < Payload Body
        DanglingBodySectionType,        /// < Dangling Body (unrecognised section considered to be Body)
        SchemaSectionType,              /// < Payload Schema
        DanglingSchemaSectionType,      /// < Dangling Schema (unrecognised section considered to be Schema)
        HeadersSectionType,             /// < Headers
        ForeignSectionType,             /// < Foreign, unexpected section
        ParametersSectionType,          /// < Parameters
        ParameterDefinitionSectionType, /// < One Parameter definition
        ParameterValuesSectionType      /// < Parameter value enumeration
    };
    
    /** \return Human readable name for given %SectionType */
    extern std::string SectionName(const SectionType& section);
    
    /** \return True if the node has recognized %SectionType */
    extern bool RecognizeSection(const mdp::MarkdownNodeIterator& node);
}

#endif
