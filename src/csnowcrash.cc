//
//  CSnowcrash.c
//  snowcrash
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//
#include "csnowcrash.h"
#include "snowcrash.h"

#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)


sc_return_value* cs_c_parse(const char* source)
{
    snowcrash::Result* result = ::new snowcrash::Result;

    snowcrash::Blueprint* blueprint = ::new snowcrash::Blueprint;
    snowcrash::parse(source, 0, *result, *blueprint);

    sc_return_value* ret = new sc_return_value;

    ret->blueprint = AS_TYPE(sc_blueprint_t, blueprint);
    ret->result = AS_TYPE(sc_result_t, result);

    return ret;
}
