//
//  CSnowcrash.cc
//  snowcrash
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "csnowcrash.h"
#include "snowcrash.h"

int sc_c_parse(const char* source, int option, sc_report_t** result, sc_blueprint_t** blueprint)
{
    snowcrash::Report* t_result = ::new snowcrash::Report;
    snowcrash::Blueprint* t_blueprint = ::new snowcrash::Blueprint;

    mdp::ByteBuffer str(source);

    int ret = snowcrash::parse(str, option, *t_result, *t_blueprint);

    *blueprint = AS_TYPE(sc_blueprint_t, t_blueprint);
    *result = AS_TYPE(sc_report_t, t_result);

    return ret;
}
