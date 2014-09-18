//
//  CSnowcrash.h
//  snowcrash
//  C Implementation of Snowcrash.h for binding purposes
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SC_C_SNOWCRASH_H
#define SC_C_SNOWCRASH_H

#include "CSourceAnnotation.h"
#include "CBlueprint.h"

#ifdef __cplusplus
extern "C" {
#endif

    /**
     *  \This is C interface for snowcrash parser.
     *
     *  \param source        A textual source data to be parsed.
     *  \param options       Parser options. Use 0 for no addtional options.
     *  \param report        returns the pointer to report.
     *  \param blueprint     returns the pointer to blueprint AST.
     *
     *  \return Error status code. Zero represents success, non-zero a failure.
     *
     *  \this function will allocate `report` and `blueprint`, for deallocation `sc_blueprint_free` and `sc_report_free` should be called.
     */
    SC_API int sc_c_parse(const char* source, sc_blueprint_parser_options option, sc_report_t** report, sc_blueprint_t** blueprint);

#ifdef __cplusplus
}
#endif

#endif
