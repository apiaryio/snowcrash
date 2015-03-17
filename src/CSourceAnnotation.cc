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


SC_API sc_report_t* sc_report_new()
{
    return AS_TYPE(sc_report_t, ::new snowcrash::Report);
}

SC_API void sc_report_free(sc_report_t* report)
{
    ::delete AS_TYPE(snowcrash::Report, report);
}

/*----------------------------------------------------------------------*/

SC_API const sc_location_t* sc_location_handler(const sc_source_annotation_t* source)
{
    const snowcrash::SourceAnnotation* p = AS_CTYPE(snowcrash::SourceAnnotation, source);
    if(!p)
        return NULL;

    return AS_CTYPE(sc_location_t, &p->location);
}

SC_API const sc_location_t* sc_resolution_location_handler(const sc_resolution_t* source)
{
    const snowcrash::ResolutionAnnotation* p = AS_CTYPE(snowcrash::ResolutionAnnotation, source);
    if (!p)
        return NULL;

    return AS_CTYPE(sc_location_t, &p->location);
}

SC_API size_t sc_location_size(const sc_location_t* location)
{
    const mdp::CharactersRangeSet* p = AS_CTYPE(mdp::CharactersRangeSet, location);
    if(!p)
        return 0;

    return p->size();
}

SC_API size_t sc_location_length(const sc_location_t* location, size_t index)
{
    const mdp::CharactersRangeSet* p = AS_CTYPE(mdp::CharactersRangeSet, location);
    if(!p)
        return 0;

    return p->at(index).length;
}

SC_API size_t sc_location_location(const sc_location_t* location, size_t index)
{
    const mdp::CharactersRangeSet* p = AS_CTYPE(mdp::CharactersRangeSet, location);
    if(!p)
        return 0;

    return p->at(index).location;
}

/*----------------------------------------------------------------------*/

SC_API const sc_error_t* sc_error_handler(const sc_report_t* report)
{
    const snowcrash::Report* p = AS_CTYPE(snowcrash::Report, report);
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

SC_API int sc_error_subCode(const sc_error_t* error)
{
    const snowcrash::Error* p = AS_CTYPE(snowcrash::Error, error);
    if (!p)
        return 0;

    return p->subCode;
}

SC_API int sc_error_ok(const sc_error_t* error)
{
    const snowcrash::Error* p = AS_CTYPE(snowcrash::Error, error);
    if (!p)
        return 0;

    return p->OK;
}

/*----------------------------------------------------------------------*/

SC_API const sc_warnings_t* sc_warnings_handler(const sc_report_t* report)
{
    const snowcrash::Report* p = AS_CTYPE(snowcrash::Report, report);
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

SC_API int sc_warning_subCode(const sc_warning_t* warning)
{
    const snowcrash::Warning* p = AS_CTYPE(snowcrash::Warning, warning);
    if (!p)
        return 0;

    return p->subCode;
}

SC_API int sc_warning_ok(const sc_warning_t* warning)
{
    const snowcrash::Warning* p = AS_CTYPE(snowcrash::Warning, warning);
    if (!p)
        return 0;

    return p->OK;
}
/*----------------------------------------------------------------------*/

/** \returns resolution at `index` message*/
SC_API const char* sc_resolution_message(const sc_resolutions_t* resolutions, size_t index)
{
    const snowcrash::Resolutions* p = AS_CTYPE(snowcrash::Resolutions, resolutions);
    if (!p)
        return NULL;

    return p->at(index).message.c_str();
}

/** \returns resolution at `index` resolvedSource*/
SC_API const char* sc_resolution_resolvedSource(const sc_resolutions_t* resolutions, size_t index)
{
    const snowcrash::Resolutions* p = AS_CTYPE(snowcrash::Resolutions, resolutions);
    if (!p)
        return NULL;

    return p->at(index).resolvedSource.c_str();
}