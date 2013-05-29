//
//  snowcrash.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/2/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "snowcrash.h"

using namespace snowcrash;

void snowcrash::parse(const SourceData& source, BlueprintParserOptions options, Result& result, Blueprint& blueprint)
{
    Parser p;
    p.parse(source, options, result, blueprint);
}
