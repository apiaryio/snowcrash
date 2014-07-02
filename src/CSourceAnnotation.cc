//
//  CSourceAnnotation.h
//  snowcrash
//  C Implementation of SourceAnnotation.h for binding purposes
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "CSourceAnnotation.h"
#include "snowcrash.h"


using namespace snowcrash;

#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)

SC_API sc_result_t* sc_result_new()
{
    return AS_TYPE(sc_result_t, ::new snowcrash::Result);
}

SC_API void sc_result_free(sc_result_t* result)
{
    ::delete AS_TYPE(snowcrash::Result, result);
}

//////////////////////////////////////////////////////////////////////////

SC_API sc_location_t* sc_location_handler(sc_source_annotation_t* source)
{
    snowcrash::SourceAnnotation* b = AS_TYPE(snowcrash::SourceAnnotation, source);
    if(!b)
        return NULL;
    return AS_TYPE(sc_location_t, &b->location);
}

SC_API const int sc_location_size(sc_location_t* location)
{
    const snowcrash::SourceCharactersBlock* b = AS_CTYPE(snowcrash::SourceCharactersBlock, location);
    if(!b)
        return 0;
    return b->size();
}

SC_API const int sc_location_length(sc_location_t* location , int i)
{
    const snowcrash::SourceCharactersBlock* b = AS_CTYPE(snowcrash::SourceCharactersBlock, location);
    if(!b)
        return 0;
    return b->at(i).length;
}

SC_API const int sc_location_location(sc_location_t* location , int i)
{
    const snowcrash::SourceCharactersBlock* b = AS_CTYPE(snowcrash::SourceCharactersBlock, location);
    if(!b)
        return 0;
    return b->at(i).location;
}

//////////////////////////////////////////////////////////////////////////

SC_API sc_error_t* sc_error_handler(sc_result_t* result)
{
    snowcrash::Result* t_res = AS_TYPE(snowcrash::Result, result);
    if(!t_res)
        return NULL;
    return AS_TYPE(sc_error_t, &t_res->error);
}

SC_API const char* cs_error_message(sc_error_t* error)
{
    snowcrash::Error* b = AS_TYPE(snowcrash::Error, error);
    if (!b)
        return "";
    return b->message.c_str();
}

SC_API const int cs_error_code(sc_error_t* error)
{
    snowcrash::Error* b = AS_TYPE(snowcrash::Error, error);
    if (!b)
        return 0;
    return b->code;
}

SC_API const int cs_error_ok(sc_error_t* error)
{
    snowcrash::Error* b = AS_TYPE(snowcrash::Error, error);
    if (!b)
        return 0;
    return b->OK;
}

//////////////////////////////////////////////////////////////////////////

SC_API sc_warnings_t* sc_warnings_handler(sc_result_t* result)
{
    snowcrash::Result* t_res = AS_TYPE(snowcrash::Result, result);
    if(!t_res)
        return NULL;
    return AS_TYPE(sc_warnings_t, &t_res->warnings);
}

SC_API const int sc_warnings_size(sc_warnings_t* warning)
{
    const snowcrash::Warnings* b = AS_CTYPE(snowcrash::Warnings, warning);
    if(!b)
        return 0;
    return b->size();
}

SC_API sc_warning_t* sc_warning_handler(sc_warnings_t* warning, int i)
{
    snowcrash::Warnings* b = AS_TYPE(snowcrash::Warnings, warning);
    if(!b)
        return NULL;
    return AS_TYPE(sc_warning_t, &b->at(i));
}

SC_API const char* sc_warning_message(sc_warning_t* warning)
{
    snowcrash::Warning* b = AS_TYPE(snowcrash::Warning, warning);
    if (!b)
        return "";
    return b->message.c_str();
}

SC_API const int sc_warning_code(sc_warning_t* warning)
{
    snowcrash::Warning* b = AS_TYPE(snowcrash::Warning, warning);
    if (!b)
        return 0;
    return b->code;
}

SC_API const int sc_warning_ok(sc_warning_t* warning)
{
    snowcrash::Warning* b = AS_TYPE(snowcrash::Warning, warning);
    if (!b)
        return 0;
    return b->OK;
}
