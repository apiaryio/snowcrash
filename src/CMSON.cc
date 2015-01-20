//
//  CMSON.cc
//  snowcrash
//  C Implementation of MSON.h for binding purposes
//
//  Created by Pavan Kumar Sunkara on 07/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "CMSON.h"
#include "snowcrash.h"

/*----------------------------------------------------------------------*/

SC_API const sc_mson_type_name_collection_t* sc_mson_type_name_collection_handle(const sc_mson_type_specification_t* handle)
{
    const mson::TypeSpecification* p = AS_CTYPE(mson::TypeSpecification, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_type_name_collection_t, &p->nestedTypes);
}

SC_API size_t sc_mson_type_name_collection_size(const sc_mson_type_name_collection_t* handle)
{
    const mson::TypeNames* p = AS_CTYPE(mson::TypeNames, handle);

    if (!p) {
        return 0;
    }

    return p->size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_type_name_t* sc_mson_type_name_handle(const sc_mson_type_name_collection_t* handle, size_t index)
{
    const mson::TypeNames* p = AS_CTYPE(mson::TypeNames, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_type_name_t, &p->at(index));
}

SC_API const sc_mson_type_name_t* sc_mson_type_name_handle_named_type(const sc_mson_named_type_t* handle)
{
    const mson::NamedType* p = AS_CTYPE(mson::NamedType, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_type_name_t, &p->name);
}

SC_API const sc_mson_type_name_t* sc_mson_type_name_handle_type_specification(const sc_mson_type_specification_t* handle)
{
    const mson::TypeSpecification* p = AS_CTYPE(mson::TypeSpecification, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_type_name_t, &p->name);
}

SC_API sc_mson_base_type_name_t sc_mson_base_type_name(const sc_mson_type_name_t* handle)
{
    const mson::TypeName* p = AS_CTYPE(mson::TypeName, handle);

    if (!p) {
        return (sc_mson_base_type_name_t)0;
    }

    return (sc_mson_base_type_name_t)p->base;
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_symbol_t* sc_mson_symbol_handle(const sc_mson_type_name_t* handle)
{
    const mson::TypeName* p = AS_CTYPE(mson::TypeName, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_symbol_t, &p->symbol);
}

SC_API const char* sc_mson_symbol_literal(const sc_mson_symbol_t* handle)
{
    const mson::Symbol* p = AS_CTYPE(mson::Symbol, handle);

    if (!p) {
        return "";
    }

    return p->literal.c_str();
}

SC_API bool sc_mson_symbol_variable(const sc_mson_symbol_t* handle)
{
    const mson::Symbol* p = AS_CTYPE(mson::Symbol, handle);

    if (!p) {
        return false;
    }

    return p->variable;
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_type_definition_t* sc_mson_type_definition_handle_named_type(const sc_mson_named_type_t* handle)
{
    const mson::NamedType* p = AS_CTYPE(mson::NamedType, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_type_definition_t, &p->typeDefinition);
}

SC_API const sc_mson_type_definition_t* sc_mson_type_definition_handle_value_definition(const sc_mson_value_definition_t* handle)
{
    const mson::ValueDefinition* p = AS_CTYPE(mson::ValueDefinition, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_type_definition_t, &p->typeDefinition);
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_type_specification_t* sc_mson_type_specification_handle(const sc_mson_type_definition_t* handle)
{
    const mson::TypeDefinition* p = AS_CTYPE(mson::TypeDefinition, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_type_specification_t, &p->typeSpecification);
}

/*----------------------------------------------------------------------*/

SC_API sc_mson_type_attributes_t sc_mson_type_attributes(const sc_mson_type_specification_t* handle)
{
    const mson::TypeDefinition* p = AS_CTYPE(mson::TypeDefinition, handle);

    if (!p) {
        return 0;
    }

    return p->attributes;
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_type_section_collection_t* sc_mson_type_section_collection_handle_named_type(const sc_mson_named_type_t* handle)
{
    const mson::NamedType* p = AS_CTYPE(mson::NamedType, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_type_section_collection_t, &p->sections);
}

SC_API const sc_mson_type_section_collection_t* sc_mson_type_section_collection_handle_property_member(const sc_mson_property_member_t* handle)
{
    const mson::PropertyMember* p = AS_CTYPE(mson::PropertyMember, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_type_section_collection_t, &p->sections);
}

SC_API const sc_mson_type_section_collection_t* sc_mson_type_section_collection_handle_value_member(const sc_mson_value_member_t* handle)
{
    const mson::ValueMember* p = AS_CTYPE(mson::ValueMember, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_type_section_collection_t, &p->sections);
}

SC_API size_t sc_mson_type_section_collection_size(const sc_mson_type_section_collection_t* handle)
{
    const mson::TypeSections* p = AS_CTYPE(mson::TypeSections, handle);

    if (!p) {
        return 0;
    }

    return p->size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_type_section_t* sc_mson_type_section_handle(const sc_mson_type_section_collection_t* handle, size_t index)
{
    const mson::TypeSections* p = AS_CTYPE(mson::TypeSections, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_type_section_t, &p->at(index));
}

SC_API sc_mson_type_section_class_t sc_mson_type_section_class(const sc_mson_type_section_t* handle)
{
    const mson::TypeSection* p = AS_CTYPE(mson::TypeSection, handle);

    if (!p) {
        return (sc_mson_type_section_class_t)0;
    }

    return (sc_mson_type_section_class_t)p->klass;
}

SC_API const char* sc_mson_type_section_description(const sc_mson_type_section_t* handle)
{
    const mson::TypeSection* p = AS_CTYPE(mson::TypeSection, handle);

    if (!p) {
        return "";
    }

    return p->content.description.c_str();
}

SC_API const char* sc_mson_type_section_value(const sc_mson_type_section_t* handle)
{
    const mson::TypeSection* p = AS_CTYPE(mson::TypeSection, handle);

    if (!p) {
        return "";
    }

    return p->content.value.c_str();
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_element_collection_t* sc_mson_element_collection_handle_type_section(const sc_mson_type_section_t* handle)
{
    const mson::TypeSection* p = AS_CTYPE(mson::TypeSection, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_element_collection_t, &p->content.elements());
}

SC_API const sc_mson_element_collection_t* sc_mson_element_collection_handle_element(const sc_mson_element_t* handle)
{
    const mson::Element* p = AS_CTYPE(mson::Element, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_element_collection_t, &p->content.elements());
}

SC_API size_t sc_mson_element_collection_size(const sc_mson_element_collection_t* handle)
{
    const mson::Elements* p = AS_CTYPE(mson::Elements, handle);

    if (!p) {
        return 0;
    }

    return p->size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_element_t* sc_mson_element_handle(const sc_mson_element_collection_t* handle, size_t index)
{
    const mson::Elements* p = AS_CTYPE(mson::Elements, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_element_t, &p->at(index));
}

SC_API sc_mson_element_class_t sc_mson_element_class(const sc_mson_element_t* handle)
{
    const mson::Element* p = AS_CTYPE(mson::Element, handle);

    if (!p) {
        return (sc_mson_element_class_t)0;
    }

    return (sc_mson_element_class_t)p->klass;
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_property_member_t* sc_mson_property_member_handle(const sc_mson_element_t* handle)
{
    const mson::Element* p = AS_CTYPE(mson::Element, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_property_member_t, &p->content.property);
}

SC_API const char* sc_mson_property_member_description(const sc_mson_property_member_t* handle)
{
    const mson::PropertyMember* p = AS_CTYPE(mson::PropertyMember, handle);

    if (!p) {
        return "";
    }

    return p->description.c_str();
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_value_member_t* sc_mson_value_member_handle(const sc_mson_element_t* handle)
{
    const mson::Element* p = AS_CTYPE(mson::Element, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_value_member_t, &p->content.value);
}

SC_API const char* sc_mson_value_member_description(const sc_mson_value_member_t* handle)
{
    const mson::ValueMember* p = AS_CTYPE(mson::ValueMember, handle);

    if (!p) {
        return "";
    }

    return p->description.c_str();
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_value_definition_t* sc_mson_value_definition_handle_property_member(const sc_mson_property_member_t* handle)
{
    const mson::PropertyMember* p = AS_CTYPE(mson::PropertyMember, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_value_definition_t, &p->valueDefinition);
}

SC_API const sc_mson_value_definition_t* sc_mson_value_definition_handle_value_member(const sc_mson_value_member_t* handle)
{
    const mson::ValueMember* p = AS_CTYPE(mson::ValueMember, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_value_definition_t, &p->valueDefinition);
}

SC_API const sc_mson_value_definition_t* sc_mson_value_definition_handle_property_name(const sc_mson_property_name_t* handle)
{
    const mson::PropertyName* p = AS_CTYPE(mson::PropertyName, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_value_definition_t, &p->variable);
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_value_collection_t* sc_mson_value_collection_handle(const sc_mson_value_definition_t* handle)
{
    const mson::ValueDefinition* p = AS_CTYPE(mson::ValueDefinition, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_value_collection_t, &p->values);
}

SC_API size_t sc_mson_value_collection_size(const sc_mson_value_collection_t* handle)
{
    const mson::Values* p = AS_CTYPE(mson::Values, handle);

    if (!p) {
        return 0;
    }

    return p->size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_value_t* sc_mson_value_handle(const sc_mson_value_collection_t* handle, size_t index)
{
    const mson::Values* p = AS_CTYPE(mson::Values, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_value_t, &p->at(index));
}

SC_API const char* sc_mson_value_literal(const sc_mson_value_t* handle)
{
    const mson::Value* p = AS_CTYPE(mson::Value, handle);

    if (!p) {
        return "";
    }

    return p->literal.c_str();
}

SC_API bool sc_mson_value_variable(const sc_mson_value_t* handle)
{
    const mson::Value* p = AS_CTYPE(mson::Value, handle);

    if (!p) {
        return false;
    }

    return p->variable;
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_property_name_t* sc_mson_property_name_handle(const sc_mson_property_member_t* handle)
{
    const mson::PropertyMember* p = AS_CTYPE(mson::PropertyMember, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_property_name_t, &p->name);
}

SC_API const char* sc_mson_property_name_literal(const sc_mson_property_name_t* handle)
{
    const mson::PropertyName* p = AS_CTYPE(mson::PropertyName, handle);

    if (!p) {
        return "";
    }

    return p->literal.c_str();
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_one_of_collection_t* sc_mson_one_of_handle(const sc_mson_element_t* handle)
{
    const mson::Element* p = AS_CTYPE(mson::Element, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_one_of_collection_t, &p->content.oneOf());
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_mixin_t* sc_mson_mixin_handle(const sc_mson_element_t* handle)
{
    const mson::Element* p = AS_CTYPE(mson::Element, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_mixin_t, &p->content.mixin);
}

/*----------------------------------------------------------------------*/