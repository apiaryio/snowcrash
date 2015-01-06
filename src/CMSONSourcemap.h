//
//  CMSONSourcemap.h
//  snowcrash
//  C Implementation of MSONSourcemap.h for binding purposes
//
//  Created by Pavan Kumar Sunkara on 07/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_C_MSONSOURCEMAP_H
#define SNOWCRASH_C_MSONSOURCEMAP_H

#include "Platform.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

    /** Class source map wrapper */
    struct sc_source_map_s;
    typedef struct sc_source_map_s sc_source_map_t;

    /** Class Named Type source map wrapper */
    struct sc_mson_sm_named_type_s;
    typedef struct sc_mson_sm_named_type_s sc_mson_sm_named_type_t;

    /** Class Type Section source map wrapper */
    struct sc_mson_sm_type_section_s;
    typedef struct sc_mson_sm_type_section_s sc_mson_sm_type_section_t;

    /** Array Type Section Collection source map wrapper */
    struct sc_mson_sm_type_section_collection_s;
    typedef struct sc_mson_sm_type_section_collection_s sc_mson_sm_type_section_collection_t;

    /** Class Element source map wrapper */
    struct sc_mson_sm_element_s;
    typedef struct sc_mson_sm_element_s sc_mson_sm_element_t;

    /** Array Element Collection source map wrapper */
    struct sc_mson_sm_element_collection_s;
    typedef struct sc_mson_sm_element_collection_s sc_mson_sm_element_collection_t;

    /** Class Property Member source map wrapper */
    struct sc_mson_sm_property_member_s;
    typedef struct sc_mson_sm_property_member_s sc_mson_sm_property_member_t;

    /** Class Value Member source map wrapper */
    struct sc_mson_sm_value_member_s;
    typedef struct sc_mson_sm_value_member_s sc_mson_sm_value_member_t;

    /** Class Mixin source map wrapper (alias of source map) */
    typedef sc_source_map_t sc_mson_sm_mixin_t;

    /** Class One Of source map wrapper (alias of Element Collection source map) */
    typedef sc_mson_sm_element_collection_t sc_mson_sm_one_of_collection_t;

    /*----------------------------------------------------------------------*/

    /** \returns size of the source map */
    SC_API size_t sc_source_map_size(const sc_source_map_t* source_map);

    /** \returns Location of the source map item at index */
    SC_API size_t sc_source_map_length(const sc_source_map_t* source_map, size_t index);

    /** \returns Length of the source map item at index */
    SC_API size_t sc_source_map_location(const sc_source_map_t* source_map, size_t index);

    /*----------------------------------------------------------------------*/

    /** \returns Named Type name */
    SC_API const sc_source_map_t* sc_mson_sm_named_type_name(const sc_mson_sm_named_type_t* handle);

    /** \returns Named Type type definition */
    SC_API const sc_source_map_t* sc_mson_sm_named_type_type_definition(const sc_mson_sm_named_type_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Type Section Collection array handle from Named Type */
    SC_API const sc_mson_sm_type_section_collection_t* sc_mson_sm_type_section_collection_handle_named_type(const sc_mson_sm_named_type_t* handle);

    /** \returns Type Section Collection array handle from Property Member */
    SC_API const sc_mson_sm_type_section_collection_t* sc_mson_sm_type_section_collection_handle_property_member(const sc_mson_sm_property_member_t* handle);

    /** \returns Type Section Collection array handle from Value Member */
    SC_API const sc_mson_sm_type_section_collection_t* sc_mson_sm_type_section_collection_handle_value_member(const sc_mson_sm_value_member_t* handle);

    /** \returns size of Type Section Collection array */
    SC_API size_t sc_mson_sm_type_section_collection_size(const sc_mson_sm_type_section_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Type Section at `index` handle */
    SC_API const sc_mson_sm_type_section_t* sc_mson_sm_type_section_handle(const sc_mson_sm_type_section_collection_t* handle, size_t index);

    /** \returns Type Section description */
    SC_API const sc_source_map_t* sc_mson_sm_type_section_description(const sc_mson_sm_type_section_t* handle);

    /** \returns Type Section value */
    SC_API const sc_source_map_t* sc_mson_sm_type_section_value(const sc_mson_sm_type_section_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Element Collection array handle from Type Section */
    SC_API const sc_mson_sm_element_collection_t* sc_mson_sm_element_collection_handle_type_section(const sc_mson_sm_type_section_t* handle);

    /** \returns Element Collection array handle from Element */
    SC_API const sc_mson_sm_element_collection_t* sc_mson_sm_element_collection_handle_element(const sc_mson_sm_element_t* handle);

    /** \returns size of Element Collection array */
    SC_API size_t sc_mson_sm_element_collection_size(const sc_mson_sm_element_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Element at `index` handle */
    SC_API const sc_mson_sm_element_t* sc_mson_sm_element_handle(const sc_mson_sm_element_t* handle, size_t index);

    /*----------------------------------------------------------------------*/

    /** \returns Property Member handle */
    SC_API const sc_mson_sm_property_member_t* sc_mson_sm_property_member_handle(const sc_mson_sm_element_t* handle);

    /** \returns Property Member name */
    SC_API const sc_source_map_t* sc_mson_sm_property_member_name(const sc_mson_sm_property_member_t* handle);

    /** \returns Property Member description */
    SC_API const sc_source_map_t* sc_mson_sm_property_member_description(const sc_mson_sm_property_member_t* handle);

    /** \returns Property Member value definition */
    SC_API const sc_source_map_t* sc_mson_sm_property_member_value_definition(const sc_mson_sm_property_member_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Value Member handle */
    SC_API const sc_mson_sm_value_member_t* sc_mson_sm_value_member_handle(const sc_mson_sm_element_t* handle);

    /** \returns Value Member description */
    SC_API const sc_source_map_t* sc_mson_sm_value_member_description(const sc_mson_sm_value_member_t* handle);

    /** \returns Value Member value definition */
    SC_API const sc_source_map_t* sc_mson_sm_value_member_value_definition(const sc_mson_sm_value_member_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns One Of handle */
    SC_API const sc_mson_sm_one_of_collection_t* sc_mson_sm_one_of_handle(const sc_mson_sm_element_t* handle);

    /*----------------------------------------------------------------------*/

    /** \retuns Mixin handle */
    SC_API const sc_mson_sm_mixin_t* sc_mson_sm_mixin_handle(const sc_mson_sm_element_t* handle);

    /*----------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif
