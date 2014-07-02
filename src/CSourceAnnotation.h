//
//  CSourceAnnotation.h
//  snowcrash
//  C Implementation of SourceAnnotation.h for binding purposes
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SC_C_SOURCEANNOTATION_H
#define SC_C_SOURCEANNOTATION_H

#ifndef SC_API
#  ifdef _WIN32
#     if defined(CSNOWCRASH_BUILD_SHARED) /* build dll */
#         define SC_API __declspec(dllimport)
#     elif !defined(CSNOWCRASH_BUILD_STATIC) /* use dll */
#         define SC_API __declspec(dllexport)
#     else /* static library */
#         define SC_API
#     endif
#  else
#     if __GNUC__ >= 4
#         define SC_API /*__attribute__((visibility("default")))*/
#     else
#         define SC_API
#     endif
#  endif
#endif


#ifdef __cplusplus
extern "C" {
#endif

    /** Class Result wrapper */
    struct sc_result_s;
    typedef struct sc_result_s sc_result_t;

    /** Class warnings wrapper */
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

    //////////////////////////////////////////////////////////////////////////

    SC_API sc_result_t* sc_result_new();
    SC_API void sc_result_free(sc_result_t* result);

    //////////////////////////////////////////////////////////////////////////

    /** \returns location handler*/
    SC_API sc_location_t* sc_location_handler(sc_source_annotation_t* source);

    /** \returns location array size*/
    SC_API const int sc_location_size(sc_location_t* location);

    /** \returns location at `i` length*/
    SC_API const int sc_location_length(sc_location_t* location , int i);

    /** \returns location at `i` location*/
    SC_API const int sc_location_location(sc_location_t* location , int i);

    //////////////////////////////////////////////////////////////////////////

    /** \returns error handler*/
    SC_API sc_error_t* sc_error_handler(sc_result_t* result);

    /** \returns error message*/
    SC_API const char* cs_error_message(sc_error_t* error);

    /** \returns error message*/
    SC_API const int cs_error_code(sc_error_t* error);

    /** \returns error message*/
    SC_API const int cs_error_ok(sc_error_t* error);

    //////////////////////////////////////////////////////////////////////////

    /** \returns warnings handler*/
    SC_API sc_warnings_t* sc_warnings_handler(sc_result_t* result);

    /** \returns warnings array size*/
    SC_API const int sc_warnings_size(sc_warnings_t* warning);

    //////////////////////////////////////////////////////////////////////////

    /** \returns warning at `i` handle*/
    SC_API sc_warning_t* sc_warning_handler(sc_warnings_t* warning, int i);

    /** \returns warning message*/
    SC_API const char* sc_warning_message(sc_warning_t* warning);

    /** \returns warning code*/
    SC_API const int sc_warning_code(sc_warning_t* warning);

    /** \returns warning ok*/
    SC_API const int sc_warning_ok(sc_warning_t* warning);

#ifdef __cplusplus
}
#endif

#endif
