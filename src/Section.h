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
        UndefinedSectionType = 0,       /// < Undefined section
        BlueprintSectionType,           /// < Blueprint overview
        DataStructureGroupSectionType,  /// < Data Structure Group
        ResourceGroupSectionType,       /// < Resource group
        ResourceSectionType,            /// < Resource
        ActionSectionType,              /// < Action
        RelationSectionType,            /// < Link Relation
        RequestSectionType,             /// < Request
        RequestBodySectionType,         /// < Request & Payload body combined (abbrev)
        ResponseSectionType,            /// < Response
        ResponseBodySectionType,        /// < Response & Body combined (abbrev)
        AttributesSectionType,          /// < Attributes
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
        MSONParameterSectionType,       /// < One Parameter definition using the MSON syntax
        ValuesSectionType,              /// < Value enumeration
        ValueSectionType,               /// < One Value
        MSONMixinSectionType,           /// < MSON Mixin
        MSONNamedTypeSectionType,       /// < MSON Named Type
        MSONOneOfSectionType,           /// < MSON One Of
        MSONPropertyMemberSectionType,  /// < MSON Property Member Type
        MSONSampleDefaultSectionType,   /// < MSON Sample/Default Type Section
        MSONPropertyMembersSectionType, /// < MSON Property Members Type Section
        MSONValueMembersSectionType,    /// < MSON Value Members Type Section
        MSONValueMemberSectionType,     /// < MSON Value Member Type
        MSONSectionType                 /// < MSON Property Member or Value Member
    };

    /** \return Human readable name for given %SectionType */
    extern std::string SectionName(const SectionType& section);
}

#endif
