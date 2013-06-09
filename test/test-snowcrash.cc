//
//  test-snowcrash.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrash.h"

#define CATCH_CONFIG_MAIN // generate main() for us
#include "catch.hpp"

#include "Fixture.h"

using namespace snowcrash;

const SourceData snowcrashtest::SourceDataFixture = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

TEST_CASE("snowcrash/parse", "snowcrash parse test")
{
    // TODO: :)
    Result result;
    Blueprint blueprint;
    snowcrash::parse("", 0, result, blueprint);
}
