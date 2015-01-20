//
//  CMSON.h
//  snowcrash
//  C Implementation of MSON.h for binding purposes
//
//  Created by Pavan Kumar Sunkara on 07/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_C_MSON_H
#define SNOWCRASH_C_MSON_H

#include "Platform.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

    /** Class Named Type wrapper */
    struct sc_mson_named_type_s;
    typedef struct sc_mson_named_type_s sc_mson_named_type_t;

    /** Class Type Name wrapper */
    struct sc_mson_type_name_s;
    typedef struct sc_mson_type_name_s sc_mson_type_name_t;

    /** Array Type Name Collection wrapper */
    struct sc_mson_type_name_collection_s;
    typedef struct sc_mson_type_name_collection_s sc_mson_type_name_collection_t;

    /** Enum Base Type Name */
    typedef enum sc_mson_base_type_name_t {
        SC_MSON_UNDEFINED_TYPE_NAME,
        SC_MSON_BOOLEAN_TYPE_NAME,
        SC_MSON_STRING_TYPE_NAME,
        SC_MSON_NUMBER_TYPE_NAME,
        SC_MSON_ARRAY_TYPE_NAME,
        SC_MSON_ENUM_TYPE_NAME,
        SC_MSON_OBJECT_TYPE_NAME
    } sc_mson_base_type_name_t;

    /** Class Symbol wrapper */
    struct sc_mson_symbol_s;
    typedef struct sc_mson_symbol_s sc_mson_symbol_t;

    /** Class Type Definition wrapper */
    struct sc_mson_type_definition_s;
    typedef struct sc_mson_type_definition_s sc_mson_type_definition_t;

    /** Class Type Specification wrapper */
    struct sc_mson_type_specification_s;
    typedef struct sc_mson_type_specification_s sc_mson_type_specification_t;

    /** Type Attributes */
    typedef unsigned int sc_mson_type_attributes_t;

    /** Enum Type Attribute */
    typedef enum sc_mson_type_attribute_t {
        SC_MSON_REQUIRED_TYPE_ATTRIBUTE,
        SC_MSON_OPTIONAL_TYPE_ATTRIBUTE,
        SC_MSON_FIXED_TYPE_ATTRIBUTE,
        SC_MSON_SAMPLE_TYPE_ATTRIBUTE,
        SC_MSON_DEFAULT_TYPE_ATTRIBUTE
    } sc_mson_type_attribute_t;

    /** Class Type Section wrapper */
    struct sc_mson_type_section_s;
    typedef struct sc_mson_type_section_s sc_mson_type_section_t;

    /** Array Type Section Collection wrapper */
    struct sc_mson_type_section_collection_s;
    typedef struct sc_mson_type_section_collection_s sc_mson_type_section_collection_t;

    /** Enum Type Section Class */
    typedef enum sc_mson_type_section_class_t {
        SC_MSON_UNDEFINED_TYPE_SECTION_CLASS,
        SC_MSON_BLOCK_DESCRIPTION_TYPE_SECTION_CLASS,
        SC_MSON_MEMBER_TYPE_TYPE_SECTION_CLASS,
        SC_MSON_SAMPLE_TYPE_SECTION_CLASS,
        SC_MSON_DEFAULT_TYPE_SECTION_CLASS
    } sc_mson_type_section_class_t;

    /** Class Element wrapper */
    struct sc_mson_element_s;
    typedef struct sc_mson_element_s sc_mson_element_t;

    /** Array Element Collection wrapper */
    struct sc_mson_element_collection_s;
    typedef struct sc_mson_element_collection_s sc_mson_element_collection_t;

    /** Enum Element Class */
    typedef enum sc_mson_element_class_t {
        SC_MSON_UNDEFINED_ELEMENT_CLASS,
        SC_MSON_PROPERTY_ELEMENT_CLASS,
        SC_MSON_VALUE_ELEMENT_CLASS,
        SC_MSON_MIXIN_ELEMENT_CLASS,
        SC_MSON_ONE_OF_ELEMENT_CLASS,
        SC_MSON_GROUP_ELEMENT_CLASS
    } sc_mson_element_class_t;

    /** Class Property Member wrapper */
    struct sc_mson_property_member_s;
    typedef struct sc_mson_property_member_s sc_mson_property_member_t;

    /** Class Property Name wrapper */
    struct sc_mson_property_name_s;
    typedef struct sc_mson_property_name_s sc_mson_property_name_t;

    /** Class Value Definition wrapper */
    struct sc_mson_value_definition_s;
    typedef struct sc_mson_value_definition_s sc_mson_value_definition_t;

    /** Array Value Collection wrapper */
    struct sc_mson_value_collection_s;
    typedef struct sc_mson_value_collection_s sc_mson_value_collection_t;

    /** Class Value wrapper */
    struct sc_mson_value_s;
    typedef struct sc_mson_value_s sc_mson_value_t;

    /** Class Value Member wrapper */
    struct sc_mson_value_member_s;
    typedef struct sc_mson_value_member_s sc_mson_value_member_t;

    /** Class Mixin wrapper (alias of Type Definition) */
    typedef struct sc_mson_type_definition_s sc_mson_mixin_t;

    /** Class One Of wrapper (alias of Element Collection) */
    typedef struct sc_mson_element_collection_s sc_mson_one_of_collection_t;

    /*----------------------------------------------------------------------*/

    /** \returns Type Name Collection array handle */
    SC_API const sc_mson_type_name_collection_t* sc_mson_type_name_collection_handle(const sc_mson_type_specification_t* handle);

    /** \returns size of Type Name Collection array */
    SC_API size_t sc_mson_type_name_collection_size(const sc_mson_type_name_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Type Name at `index` handle */
    SC_API const sc_mson_type_name_t* sc_mson_type_name_handle(const sc_mson_type_name_collection_t* handle, size_t index);

    /** \returns Type Name handle from Named Type */
    SC_API const sc_mson_type_name_t* sc_mson_type_name_handle_named_type(const sc_mson_named_type_t* handle);

    /** \returns Type Name handle from Type Specification  */
    SC_API const sc_mson_type_name_t* sc_mson_type_name_handle_type_specification(const sc_mson_type_specification_t* handle);

    /** \returns Base Type Name */
    SC_API sc_mson_base_type_name_t sc_mson_base_type_name(const sc_mson_type_name_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Symbol handle */
    SC_API const sc_mson_symbol_t* sc_mson_symbol_handle(const sc_mson_type_name_t* handle);

    /** \returns Symbol literal */
    SC_API const char* sc_mson_symbol_literal(const sc_mson_symbol_t* handle);

    /** \returns Symbol variable */
    SC_API bool sc_mson_symbol_variable(const sc_mson_symbol_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Type Definition handle from Named Type */
    SC_API const sc_mson_type_definition_t* sc_mson_type_definition_handle_named_type(const sc_mson_named_type_t* handle);

    /** \returns Type Definition handle from Value Definition */
    SC_API const sc_mson_type_definition_t* sc_mson_type_definition_handle_value_definition(const sc_mson_value_definition_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Type Specification handle */
    SC_API const sc_mson_type_specification_t* sc_mson_type_specification_handle(const sc_mson_type_definition_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Type Attributes */
    SC_API sc_mson_type_attributes_t sc_mson_type_attributes(const sc_mson_type_specification_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Type Section Collection array handle from Named Type */
    SC_API const sc_mson_type_section_collection_t* sc_mson_type_section_collection_handle_named_type(const sc_mson_named_type_t* handle);

    /** \returns Type Section Collection array handle from Property Member */
    SC_API const sc_mson_type_section_collection_t* sc_mson_type_section_collection_handle_property_member(const sc_mson_property_member_t* handle);

    /** \returns Type Section Collection array handle from Value Member */
    SC_API const sc_mson_type_section_collection_t* sc_mson_type_section_collection_handle_value_member(const sc_mson_value_member_t* handle);

    /** \returns size of Type Section Collection array */
    SC_API size_t sc_mson_type_section_collection_size(const sc_mson_type_section_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Type Section at `index` handle */
    SC_API const sc_mson_type_section_t* sc_mson_type_section_handle(const sc_mson_type_section_collection_t* handle, size_t index);

    /** \returns Type Section class */
    SC_API sc_mson_type_section_class_t sc_mson_type_section_class(const sc_mson_type_section_t* handle);

    /** \returns Type Section description */
    SC_API const char* sc_mson_type_section_description(const sc_mson_type_section_t* handle);

    /** \returns Type Section value */
    SC_API const char* sc_mson_type_section_value(const sc_mson_type_section_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Element Collection array handle from Type Section */
    SC_API const sc_mson_element_collection_t* sc_mson_element_collection_handle_type_section(const sc_mson_type_section_t* handle);

    /** \returns Element Collection array handle from Element */
    SC_API const sc_mson_element_collection_t* sc_mson_element_collection_handle_element(const sc_mson_element_t* handle);

    /** \returns size of Element Collection array */
    SC_API size_t sc_mson_element_collection_size(const sc_mson_element_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Element at `index` handle */
    SC_API const sc_mson_element_t* sc_mson_element_handle(const sc_mson_element_collection_t* handle, size_t index);

    /** \returns Element class */
    SC_API sc_mson_element_class_t sc_mson_element_class(const sc_mson_element_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Property Member handle */
    SC_API const sc_mson_property_member_t* sc_mson_property_member_handle(const sc_mson_element_t* handle);

    /** \returns Property Member description */
    SC_API const char* sc_mson_property_member_description(const sc_mson_property_member_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Value Member handle */
    SC_API const sc_mson_value_member_t* sc_mson_value_member_handle(const sc_mson_element_t* handle);

    /** \returns Value Member description */
    SC_API const char* sc_mson_value_member_description(const sc_mson_value_member_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Value Definition handle from Property Member */
    SC_API const sc_mson_value_definition_t* sc_mson_value_definition_handle_property_member(const sc_mson_property_member_t* handle);

    /** \returns Value Definition handle from Value Member */
    SC_API const sc_mson_value_definition_t* sc_mson_value_definition_handle_value_member(const sc_mson_value_member_t* handle);

    /** \returns Value Definition handle from Property Name */
    SC_API const sc_mson_value_definition_t* sc_mson_value_definition_handle_property_name(const sc_mson_property_name_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Value Collection array handle */
    SC_API const sc_mson_value_collection_t* sc_mson_value_collection_handle(const sc_mson_value_definition_t* handle);

    /** \returns size of Value Collection array */
    SC_API size_t sc_mson_value_collection_size(const sc_mson_value_collection_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Value at `index` handle */
    SC_API const sc_mson_value_t* sc_mson_value_handle(const sc_mson_value_collection_t* handle, size_t index);

    /** \returns Value literal */
    SC_API const char* sc_mson_value_literal(const sc_mson_value_t* handle);

    /** \returns Value variable */
    SC_API bool sc_mson_value_variable(const sc_mson_value_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns Property Name handle */
    SC_API const sc_mson_property_name_t* sc_mson_property_name_handle(const sc_mson_property_member_t* handle);

    /** \returns Property Name literal */
    SC_API const char* sc_mson_property_name_literal(const sc_mson_property_name_t* handle);

    /*----------------------------------------------------------------------*/

    /** \returns One Of handle */
    SC_API const sc_mson_one_of_collection_t* sc_mson_one_of_handle(const sc_mson_element_t* handle);

    /*----------------------------------------------------------------------*/

    /** \retuns Mixin handle */
    SC_API const sc_mson_mixin_t* sc_mson_mixin_handle(const sc_mson_element_t* handle);

    /*----------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif
