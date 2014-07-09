//
//  CSnowcrash.cc
//  snowcrash
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//
#include "csnowcrash.h"
#include "snowcrash.h"


int sc_c_parse(const char* source, sc_blueprint_parser_options option, sc_result_t** result, sc_blueprint_t** blueprint)
{
    snowcrash::Result* t_result = ::new snowcrash::Result;
    snowcrash::Blueprint* t_blueprint = ::new snowcrash::Blueprint;

    int ret = snowcrash::parse(source, option, *t_result, *t_blueprint);

    *blueprint = AS_TYPE(sc_blueprint_t, t_blueprint);
    *result = AS_TYPE(sc_result_t, t_result);

    return ret;
}
