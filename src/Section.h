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

namespace snowcrash {

    /**
     *  API Blueprint Sections Types.
     */
    enum SectionType {
        UndefinedSectionType,           /// < Undefined section
        BlueprintSectionType,           /// < Blueprint overview
        ResourceGroupSectionType,       /// < Resource group
        ResourceSectionType,            /// < Resource
        ActionSectionType,              /// < Action
        RequestSectionType,             /// < Request
        RequestBodySectionType,         /// < Request & Payload body combined (abbrev)
        ResponseSectionType,            /// < Response
        ResponseBodySectionType,        /// < Response & Body combined (abbrev)
        ModelSectionType,               /// < Model
        ModelBodySectionType,           /// < Model & Body combined (abbrev)
        BodySectionType,                /// < Payload Body
        DanglingBodySectionType,        /// < Dangling Body (unrecognised section considered to be Body)
        SchemaSectionType,              /// < Payload Schema
        DanglingSchemaSectionType,      /// < Dangling Schema (unrecognised section considered to be Schema)
        HeadersSectionType,             /// < Headers
        ForeignSectionType,             /// < Foreign, unexpected section
        ParametersSectionType,          /// < Parameters
        ParameterSectionType,           /// < One Parameter definition
        ValuesSectionType,              /// < Value enumeration
        ValueSectionType,               /// < One Value
        MSONMixinSectionType,           /// < MSON Mixin
        MSONNamedTypeSectionType,       /// < MSON Named Type
        MSONMemberTypeGroupSectionType, /// < MSON Member Type Group Type Section
        MSONOneOfSectionType,           /// < MSON One Of
        MSONPropertyMemberSectionType,  /// < MSON Property Member Type
        MSONTypeSectionSectionType,     /// < MSON Type Section
        MSONValueMemberSectionType      /// < MSON Value Member Type
    };

    /** \return Human readable name for given %SectionType */
    extern std::string SectionName(const SectionType& section);
}

#endif
