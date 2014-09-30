//
//  CSnowcrash.cc
//  snowcrash
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "csnowcrash.h"
#include "snowcrash.h"

int sc_c_parse(const char* source, sc_blueprint_parser_options option, sc_report_t** report, sc_blueprint_t** blueprint, sc_sm_blueprint_t** sm_blueprint)
{
    snowcrash::Report* t_report = ::new snowcrash::Report;
    snowcrash::Blueprint* t_blueprint = ::new snowcrash::Blueprint;
    snowcrash::SourceMap<snowcrash::Blueprint>* t_sm_blueprint = ::new snowcrash::SourceMap<snowcrash::Blueprint>;

    snowcrash::ParseResult<snowcrash::Blueprint>* t_parse_result = ::new snowcrash::ParseResult<snowcrash::Blueprint>(*t_report, *t_blueprint, *t_sm_blueprint);

    int ret = snowcrash::parse(source, option, *t_parse_result);

    *report = AS_TYPE(sc_report_t, t_report);
    *blueprint = AS_TYPE(sc_blueprint_t, t_blueprint);
    *sm_blueprint = AS_TYPE(sc_sm_blueprint_t, t_sm_blueprint);

    return ret;
}
