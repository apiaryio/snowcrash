//
//  test-Blueprint.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/3/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "catch.hpp"
#include "Blueprint.h"

using namespace snowcrash;

TEST_CASE("blueprint/blueprint-init", "Blueprint initializaton")
{
    Blueprint blueprint;
    REQUIRE(blueprint.name.length() == 0);
    REQUIRE(blueprint.description.length() == 0);
    REQUIRE(blueprint.metadata.size() == 0);
    REQUIRE(blueprint.content.elements().size() == 0);
}

TEST_CASE("blueprint/init", "Blueprint initializaton")
{
    Blueprint blueprint;
    REQUIRE(blueprint.name.length() == 0);
    REQUIRE(blueprint.description.length() == 0);
    REQUIRE(blueprint.metadata.size() == 0);
    REQUIRE(blueprint.content.elements().size() == 0);
}
