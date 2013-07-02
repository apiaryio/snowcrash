//
//  test-snowcrash.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#define CATCH_CONFIG_MAIN // generate main() for us
#include "catch.hpp"

#include "snowcrash.h"
#include "Fixture.h"

const snowcrash::SourceData snowcrashtest::SourceDataFixture = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

using namespace snowcrash;

TEST_CASE("snowcrash/parse", "snowcrash parse test")
{
    // TODO:
    Result result;
    Blueprint blueprint;
    snowcrash::parse("", 0, result, blueprint);
}
