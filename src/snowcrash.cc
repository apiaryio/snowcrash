//
//  snowcrash.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrash.h"

using namespace snowcrash;

int snowcrash::parse(const SourceData& source, BlueprintParserOptions options, Result& result, Blueprint& blueprint)
{
    Parser p;
    p.parse(source, options, result, blueprint);
    return result.error.code;
}
