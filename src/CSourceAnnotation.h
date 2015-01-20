//
//  CSourceAnnotation.h
//  snowcrash
//  C Implementation of SourceAnnotation.h for binding purposes
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_C_SOURCEANNOTATION_H
#define SNOWCRASH_C_SOURCEANNOTATION_H

#include "Platform.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

    /** Class Report wrapper */
    struct sc_report_s;
    typedef struct sc_report_s sc_report_t;

    /** Array warnings wrapper */
    struct sc_warnings_s;
    typedef struct sc_warnings_s sc_warnings_t;

    /** Class warning wrapper */
    struct sc_warning_s;
    typedef struct sc_warning_s sc_warning_t;

    /** Class error wrapper */
    struct sc_error_s;
    typedef struct sc_error_s sc_error_t;

    /** Class location wrapper */
    struct sc_location_s;
    typedef struct sc_location_s sc_location_t;

    /** Class SourceAnnotation wrapper */
    struct sc_source_annotation_s;
    typedef struct sc_source_annotation_s sc_source_annotation_t;

    /*----------------------------------------------------------------------*/

    /** \returns pointer to allocated Report*/
    SC_API sc_report_t* sc_report_new();

    /** \deallocate Report from pointer*/
    SC_API void sc_report_free(sc_report_t* report);

    /*----------------------------------------------------------------------*/

    /** \returns location handler*/
    SC_API const sc_location_t* sc_location_handler(const sc_source_annotation_t* source);

    /** \returns location array size*/
    SC_API size_t sc_location_size(const sc_location_t* location);

    /** \returns location at `index` length*/
    SC_API size_t sc_location_length(const sc_location_t* location, size_t index);

    /** \returns location at `index` location*/
    SC_API size_t sc_location_location(const sc_location_t* location, size_t index);

    /*----------------------------------------------------------------------*/

    /** \returns error handler*/
    SC_API const sc_error_t* sc_error_handler(const sc_report_t* report);

    /** \returns error message*/
    SC_API const char* sc_error_message(const sc_error_t* error);

    /** \returns error code*/
    SC_API int sc_error_code(const sc_error_t* error);

    /** \returns error OK*/
    SC_API int sc_error_ok(const sc_error_t* error);

    /*----------------------------------------------------------------------*/

    /** \returns warnings handler*/
    SC_API const sc_warnings_t* sc_warnings_handler(const sc_report_t* report);

    /** \returns warnings array size*/
    SC_API size_t sc_warnings_size(const sc_warnings_t* warning);

    /*----------------------------------------------------------------------*/

    /** \returns warning at `index` handle*/
    SC_API const sc_warning_t* sc_warning_handler(const sc_warnings_t* warning, size_t index);

    /** \returns warning message*/
    SC_API const char* sc_warning_message(const sc_warning_t* warning);

    /** \returns warning code*/
    SC_API int sc_warning_code(const sc_warning_t* warning);

    /** \returns warning OK*/
    SC_API int sc_warning_ok(const sc_warning_t* warning);

#ifdef __cplusplus
}
#endif

#endif
