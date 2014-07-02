//
//  CSnowcrash.h
//  snowcrash
//  C Implementation of Snowcrash.h for binding purposes
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef CS_C_SNOWCRASH_H
#define CS_C_SNOWCRASH_H

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

#include "CSourceAnnotation.h"
#include "CBlueprint.h"

#ifdef __cplusplus
extern "C" {
#endif

    struct sc_return_value
    {
        sc_blueprint_t* blueprint;
        sc_result_s* result;
    };

    /** Parser */
    SC_API sc_return_value* cs_c_parse(const char* source);

#ifdef __cplusplus
}
#endif

#endif
