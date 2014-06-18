//
//  CSnowcrash.h
//  snowcrash
//  C Implementation of Snowcrash.h for binding purposes
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef C_SNOWCRASH_H
#define C_SNOWCRASH_H

#include "CSourceAnnotation.h"
#include "CBlueprint.h"

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC typedef char* C_SourceData;
EXTERNC typedef unsigned int C_BlueprintParserOptions;

/**
 *  \param i_source        A textual source data to be parsed.
 *  \param i_options       Parser options. Use 0 for no addtional options.
 *  \param i_result        Parsing result report.
 *  \param i_blueprint     Parsed blueprint AST.
 *  \return Error status code. Zero represents success, non-zero a failure.
 */
EXTERNC int C_parse(const C_SourceData& i_source, C_BlueprintParserOptions i_options, C_Result& i_result, C_Blueprint& i_blueprint);

#endif
