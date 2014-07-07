//
//  CSourceAnnotation.cc
//  snowcrash
//  C Implementation of SourceAnnotation.h for binding purposes
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "CSourceAnnotation.h"
#include "snowcrash.h"


SC_API sc_result_t* sc_result_new()
{
    return AS_TYPE(sc_result_t, ::new snowcrash::Result);
}

SC_API void sc_result_free(sc_result_t* result)
{
    ::delete AS_TYPE(snowcrash::Result, result);
}

/*----------------------------------------------------------------------*/

SC_API const sc_location_t* sc_location_handler(const sc_source_annotation_t* source)
{
    const snowcrash::SourceAnnotation* p = AS_CTYPE(snowcrash::SourceAnnotation, source);
    if(!p)
        return NULL;
    return AS_CTYPE(sc_location_t, &p->location);
}

SC_API size_t sc_location_size(const sc_location_t* location)
{
    const snowcrash::SourceCharactersBlock* p = AS_CTYPE(snowcrash::SourceCharactersBlock, location);
    if(!p)
        return 0;
    return p->size();
}

SC_API size_t sc_location_length(const sc_location_t* location, size_t index)
{
    const snowcrash::SourceCharactersBlock* p = AS_CTYPE(snowcrash::SourceCharactersBlock, location);
    if(!p)
        return 0;
    return p->at(index).length;
}

SC_API size_t sc_location_location(const sc_location_t* location, size_t index)
{
    const snowcrash::SourceCharactersBlock* p = AS_CTYPE(snowcrash::SourceCharactersBlock, location);
    if(!p)
        return 0;
    return p->at(index).location;
}

/*----------------------------------------------------------------------*/

SC_API const sc_error_t* sc_error_handler(const sc_result_t* result)
{
    const snowcrash::Result* p = AS_CTYPE(snowcrash::Result, result);
    if(!p)
        return NULL;
    return AS_CTYPE(sc_error_t, &p->error);
}

SC_API const char* sc_error_message(const sc_error_t* error)
{
    const snowcrash::Error* p = AS_CTYPE(snowcrash::Error, error);
    if (!p)
        return "";
    return p->message.c_str();
}

SC_API int sc_error_code(const sc_error_t* error)
{
    const snowcrash::Error* p = AS_CTYPE(snowcrash::Error, error);
    if (!p)
        return 0;
    return p->code;
}

SC_API int sc_error_ok(const sc_error_t* error)
{
    const snowcrash::Error* p = AS_CTYPE(snowcrash::Error, error);
    if (!p)
        return 0;
    return p->OK;
}

/*----------------------------------------------------------------------*/

SC_API const sc_warnings_t* sc_warnings_handler(const sc_result_t* result)
{
    const snowcrash::Result* p = AS_CTYPE(snowcrash::Result, result);
    if(!p)
        return NULL;
    return AS_CTYPE(sc_warnings_t, &p->warnings);
}

SC_API size_t sc_warnings_size(const sc_warnings_t* warning)
{
    const snowcrash::Warnings* p = AS_CTYPE(snowcrash::Warnings, warning);
    if(!p)
        return 0;
    return p->size();
}

SC_API const sc_warning_t* sc_warning_handler(const sc_warnings_t* warning, size_t index)
{
    const snowcrash::Warnings* p = AS_CTYPE(snowcrash::Warnings, warning);
    if(!p)
        return NULL;
    return AS_CTYPE(sc_warning_t, &p->at(index));
}

SC_API const char* sc_warning_message(const sc_warning_t* warning)
{
    const snowcrash::Warning* p = AS_CTYPE(snowcrash::Warning, warning);
    if (!p)
        return "";
    return p->message.c_str();
}

SC_API int sc_warning_code(const sc_warning_t* warning)
{
    const snowcrash::Warning* p = AS_CTYPE(snowcrash::Warning, warning);
    if (!p)
        return 0;
    return p->code;
}

SC_API int sc_warning_ok(const sc_warning_t* warning)
{
    const snowcrash::Warning* p = AS_CTYPE(snowcrash::Warning, warning);
    if (!p)
        return 0;
    return p->OK;
}
