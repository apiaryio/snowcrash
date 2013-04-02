//
//  snowcrash.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/2/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "snowcrash.h"

using namespace snowcrash;

void snowcrash::parse(const SourceData& source, const Parser::ParseHandler& callback)
{
    Parser p;
    p.parse(source, callback);
}
