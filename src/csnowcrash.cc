//
//  CSnowcrash.cc
//  snowcrash
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "csnowcrash.h"
#include "snowcrash.h"

int sc_c_parse(const char* source, sc_blueprint_parser_options option, sc_report_t** report, sc_blueprint_t** blueprint)
{
    snowcrash::Report* t_report = ::new snowcrash::Report;
    snowcrash::Blueprint* t_blueprint = ::new snowcrash::Blueprint;

    int ret = snowcrash::parse(source, option, *t_report, *t_blueprint);

    *blueprint = AS_TYPE(sc_blueprint_t, t_blueprint);
    *report = AS_TYPE(sc_report_t, t_report);

    return ret;
}
