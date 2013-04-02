//
//  test-snowcrash.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/2/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "snowcrash.h"

#define CATCH_CONFIG_MAIN // generate main() for us
#include "catch.hpp"

using namespace snowcrash;

TEST_CASE( "snowcrash/parse", "snowcrash parse test" )
{    
    snowcrash::parse("", [](const Result& r, const Blueprint& b){
    });
}
