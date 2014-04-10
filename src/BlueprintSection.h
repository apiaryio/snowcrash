//
//  BlueprintSection.h
//  snowcrash
//
//  Created by Zdenek Nemec on 11/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_BLUEPRINTSECTION_H
#define SNOWCRASH_BLUEPRINTSECTION_H

#include "MarkdownBlock.h"
#include "Blueprint.h"

namespace snowcrash {
    
    /**
     *  API Blueprint Sections Types. Block range classification.
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
    
    /** \returns human readable name for given %SectionType */
    FORCEINLINE std::string SectionName(const SectionType& section) {
        switch (section) {
                
            case ModelSectionType:
            case ModelBodySectionType:
                return "model";
                
            case ObjectSectionType:
            case ObjectBodySectionType:
                return "object";
                
            case RequestSectionType:
            case RequestBodySectionType:
                return "request";
                
            case ResponseSectionType:
            case ResponseBodySectionType:
                return "response";
                
            case BodySectionType:
            case DanglingBodySectionType:
                return "message-body";
                
            case SchemaSectionType:
            case DanglingSchemaSectionType:
                return "message-schema";
                
            case HeadersSectionType:
                return "headers";
                
            default:
                return "section";
        }
    }
    
    /** Markdown block iterator */
    typedef MarkdownBlock::Stack::const_iterator BlockIterator;
    
    /**
     *  \brief SectionType boundaries.
     *
     *  A continuous range of markdown blocks <first, second).
     */
    typedef std::pair<BlockIterator, BlockIterator> SectionBounds;
    
    
    /**
     *  \brief  API Blueprint Section Descriptor.
     */
    class BlueprintSection {
        
    public:
        BlueprintSection(const SectionBounds& _bounds)
        : type(UndefinedSectionType), bounds(_bounds), m_parent(NULL) {}
        
        BlueprintSection(const SectionType _type,
                         const SectionBounds& _bounds,
                         const BlueprintSection& _parent)
        : type(_type), bounds(_bounds), m_parent(&_parent) {}
        
        /** Type of section */
        const SectionType type;
        
        /**
         *  Maximum boundaries of the section including any subsection.
         */
        const SectionBounds bounds;
        
        /**
         *  \return True if section's parent is specified, false otherwise.
         */
        bool hasParent() const {
            return (m_parent != NULL);
        }
        
        /**
         *  \return Parent section, throws exception if no parent is defined.
         */
        const BlueprintSection& parent() const {
            if (!hasParent())
                throw "accessing parent of a root blueprint section";
            return *m_parent;
        }

    private:
        BlueprintSection();
        BlueprintSection(const BlueprintSection&);
        BlueprintSection& operator=(const BlueprintSection&);
        
        const BlueprintSection* m_parent;
    };
}

#endif
