//
//  test-BlueprintUtility.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 03/03/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "BlueprintUtility.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Match action with same uri template and same http method", "[blueprint_utility]")
{
    Action action1, action2;

    action1.method = "GET";
    action2.method = "GET";

    REQUIRE(MatchAction()(action1, action2));
}

TEST_CASE("Match action with same uri template and different http method", "[blueprint_utility]")
{
    Action action1, action2;

    action1.uriTemplate = "/test";
    action1.method = "GET";

    action2.uriTemplate = "/test";
    action2.method = "POST";

    REQUIRE_FALSE(MatchAction()(action1, action2));
}

TEST_CASE("Match action with different uri template and same http method", "[blueprint_utility]")
{
    Action action1, action2;

    action1.method = "GET";

    action2.uriTemplate = "/test";
    action2.method = "GET";

    REQUIRE_FALSE(MatchAction()(action1, action2));
}
