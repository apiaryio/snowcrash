//
//  CMSONSourcemap.cc
//  snowcrash
//  C Implementation of MSONSourcemap.h for binding purposes
//
//  Created by Pavan Kumar Sunkara on 07/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "CMSONSourcemap.h"
#include "snowcrash.h"

/*----------------------------------------------------------------------*/

SC_API size_t sc_source_map_size(const sc_source_map_t* source_map)
{
    const mdp::BytesRangeSet* p = AS_CTYPE(mdp::BytesRangeSet, source_map);

    if(!p) {
        return 0;
    }

    return p->size();
}

SC_API size_t sc_source_map_length(const sc_source_map_t* source_map, size_t index)
{
    const mdp::BytesRangeSet* p = AS_CTYPE(mdp::BytesRangeSet, source_map);

    if(!p) {
        return 0;
    }

    return p->at(index).length;
}

SC_API size_t sc_source_map_location(const sc_source_map_t* source_map, size_t index)
{
    const mdp::BytesRangeSet* p = AS_CTYPE(mdp::BytesRangeSet, source_map);

    if(!p) {
        return 0;
    }

    return p->at(index).location;
}

/*----------------------------------------------------------------------*/

SC_API const sc_source_map_t* sc_mson_sm_named_type_name(const sc_mson_sm_named_type_t* handle)
{
    const snowcrash::SourceMap<mson::NamedType>* p = AS_CTYPE(snowcrash::SourceMap<mson::NamedType>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_source_map_t, &p->name.sourceMap);
}

SC_API const sc_source_map_t* sc_mson_sm_named_type_type_definition(const sc_mson_sm_named_type_t* handle)
{
    const snowcrash::SourceMap<mson::NamedType>* p = AS_CTYPE(snowcrash::SourceMap<mson::NamedType>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_source_map_t, &p->typeDefinition.sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_sm_type_section_collection_t* sc_mson_sm_type_section_collection_handle_named_type(const sc_mson_sm_named_type_t* handle)
{
    const snowcrash::SourceMap<mson::NamedType>* p = AS_CTYPE(snowcrash::SourceMap<mson::NamedType>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_sm_type_section_collection_t, &p->sections);
}

SC_API const sc_mson_sm_type_section_collection_t* sc_mson_sm_type_section_collection_handle_property_member(const sc_mson_sm_property_member_t* handle)
{
    const snowcrash::SourceMap<mson::PropertyMember>* p = AS_CTYPE(snowcrash::SourceMap<mson::PropertyMember>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_sm_type_section_collection_t, &p->sections);
}

SC_API const sc_mson_sm_type_section_collection_t* sc_mson_sm_type_section_collection_handle_value_member(const sc_mson_sm_value_member_t* handle)
{
    const snowcrash::SourceMap<mson::ValueMember>* p = AS_CTYPE(snowcrash::SourceMap<mson::ValueMember>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_sm_type_section_collection_t, &p->sections);
}

SC_API size_t sc_mson_sm_type_section_collection_size(const sc_mson_sm_type_section_collection_t* handle)
{
    const snowcrash::SourceMap<mson::TypeSections>* p = AS_CTYPE(snowcrash::SourceMap<mson::TypeSections>, handle);

    if (!p) {
        return 0;
    }

    return p->collection.size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_sm_type_section_t* sc_mson_sm_type_section_handle(const sc_mson_sm_type_section_collection_t* handle, size_t index)
{
    const snowcrash::SourceMap<mson::TypeSections>* p = AS_CTYPE(snowcrash::SourceMap<mson::TypeSections>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_sm_type_section_t, &p->collection.at(index));
}

SC_API const sc_source_map_t* sc_mson_sm_type_section_description(const sc_mson_sm_type_section_t* handle)
{
    const snowcrash::SourceMap<mson::TypeSection>* p = AS_CTYPE(snowcrash::SourceMap<mson::TypeSection>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_source_map_t, &p->description.sourceMap);
}

SC_API const sc_source_map_t* sc_mson_sm_type_section_value(const sc_mson_sm_type_section_t* handle)
{
    const snowcrash::SourceMap<mson::TypeSection>* p = AS_CTYPE(snowcrash::SourceMap<mson::TypeSection>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_source_map_t, &p->value.sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_sm_element_collection_t* sc_mson_sm_element_collection_handle_type_section(const sc_mson_sm_type_section_t* handle)
{
    const snowcrash::SourceMap<mson::TypeSection>* p = AS_CTYPE(snowcrash::SourceMap<mson::TypeSection>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_sm_element_collection_t, &p->elements());
}

SC_API const sc_mson_sm_element_collection_t* sc_mson_sm_element_collection_handle_element(const sc_mson_sm_element_t* handle)
{
    const snowcrash::SourceMap<mson::Element>* p = AS_CTYPE(snowcrash::SourceMap<mson::Element>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_sm_element_collection_t, &p->elements());
}

SC_API size_t sc_mson_sm_element_collection_size(const sc_mson_sm_element_collection_t* handle)
{
    const snowcrash::SourceMap<mson::Elements>* p = AS_CTYPE(snowcrash::SourceMap<mson::Elements>, handle);

    if (!p) {
        return 0;
    }

    return p->collection.size();
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_sm_element_t* sc_mson_sm_element_handle(const sc_mson_sm_element_collection_t* handle, size_t index)
{
    const snowcrash::SourceMap<mson::Elements>* p = AS_CTYPE(snowcrash::SourceMap<mson::Elements>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_sm_element_t, &p->collection.at(index));
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_sm_property_member_t* sc_mson_sm_property_member_handle(const sc_mson_sm_element_t* handle)
{
    const snowcrash::SourceMap<mson::Element>* p = AS_CTYPE(snowcrash::SourceMap<mson::Element>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_sm_property_member_t, &p->property);
}

SC_API const sc_source_map_t* sc_mson_sm_property_member_name(const sc_mson_sm_property_member_t* handle)
{
    const snowcrash::SourceMap<mson::PropertyMember>* p = AS_CTYPE(snowcrash::SourceMap<mson::PropertyMember>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_source_map_t, &p->name.sourceMap);
}

SC_API const sc_source_map_t* sc_mson_sm_property_member_description(const sc_mson_sm_property_member_t* handle)
{
    const snowcrash::SourceMap<mson::PropertyMember>* p = AS_CTYPE(snowcrash::SourceMap<mson::PropertyMember>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_source_map_t, &p->description.sourceMap);
}

SC_API const sc_source_map_t* sc_mson_sm_property_member_value_definition(const sc_mson_sm_property_member_t* handle)
{
    const snowcrash::SourceMap<mson::PropertyMember>* p = AS_CTYPE(snowcrash::SourceMap<mson::PropertyMember>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_source_map_t, &p->valueDefinition.sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_sm_value_member_t* sc_mson_sm_value_member_handle(const sc_mson_sm_element_t* handle)
{
    const snowcrash::SourceMap<mson::Element>* p = AS_CTYPE(snowcrash::SourceMap<mson::Element>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_sm_value_member_t, &p->value);
}

SC_API const sc_source_map_t* sc_mson_sm_value_member_description(const sc_mson_sm_value_member_t* handle)
{
    const snowcrash::SourceMap<mson::ValueMember>* p = AS_CTYPE(snowcrash::SourceMap<mson::ValueMember>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_source_map_t, &p->description.sourceMap);
}

SC_API const sc_source_map_t* sc_mson_sm_value_member_value_definition(const sc_mson_sm_value_member_t* handle)
{
    const snowcrash::SourceMap<mson::ValueMember>* p = AS_CTYPE(snowcrash::SourceMap<mson::ValueMember>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_source_map_t, &p->valueDefinition.sourceMap);
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_sm_one_of_collection_t* sc_mson_sm_one_of_handle(const sc_mson_sm_element_t* handle)
{
    const snowcrash::SourceMap<mson::Element>* p = AS_CTYPE(snowcrash::SourceMap<mson::Element>, handle);

    if (!p) {
        return NULL;
    }

    return AS_CTYPE(sc_mson_sm_one_of_collection_t, &p->oneOf());
}

/*----------------------------------------------------------------------*/

SC_API const sc_mson_sm_mixin_t* sc_mson_sm_mixin_handle(const sc_mson_sm_element_t* handle)
{
    const snowcrash::SourceMap<mson::Element>* p = AS_CTYPE(snowcrash::SourceMap<mson::Element>, handle);
    
    if (!p) {
        return NULL;
    }
    
    return AS_CTYPE(sc_mson_sm_mixin_t, &p->mixin);
}

/*----------------------------------------------------------------------*/
