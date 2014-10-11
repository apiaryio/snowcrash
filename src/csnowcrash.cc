//
//  CSnowcrash.cc
//  snowcrash
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "csnowcrash.h"
#include "snowcrash.h"

using namespace snowcrash;

int sc_c_parse(const char* source, sc_blueprint_parser_options option, sc_report_t** report, sc_blueprint_t** blueprint, sc_sm_blueprint_t** sm_blueprint)
{
    Report* resultReport = ::new snowcrash::Report;
    Blueprint* resultblueprint = ::new snowcrash::Blueprint;
    SourceMap<snowcrash::Blueprint>* resultSourceMap = ::new SourceMap<snowcrash::Blueprint>;

    ParseResultRef<Blueprint> result(*resultReport, *resultblueprint, *resultSourceMap);

    int ret = snowcrash::parse(source, option, result);

    *report = AS_TYPE(sc_report_t, resultReport);
    *blueprint = AS_TYPE(sc_blueprint_t, resultblueprint);
    *sm_blueprint = AS_TYPE(sc_sm_blueprint_t, resultSourceMap);

    return ret;
}
