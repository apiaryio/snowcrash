//
//  test-RegexMatch.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "catch.hpp"
#include "RegexMatch.h"

using namespace snowcrash;

TEST_CASE("regexmatch/simple", "Simple regex test")
{
    REQUIRE(RegexMatch("The quick brown fox jumps over the lazy dog", "fox[[:space:]]") == true);
    REQUIRE(RegexMatch("The quick brown fox jumps over the lazy dog", "box") == false);
}

TEST_CASE("regexmatch/complex", "Complex regex test")
{
    REQUIRE(RegexMatch("The quick brown  fox jumps over the lazy dog", "[[:space:]]{2}fox[[:space:]]jumps+") == true);
    REQUIRE(RegexMatch("The quick brown  fox jumps over the lazy dog", "^The") == true);
    REQUIRE(RegexMatch("The quick brown fox jumps over the lazy dog", "[[:space:]]{2}fox[[:space:]]sjumps+") == false);
}

TEST_CASE("regexmatch/resource-header", "Match resource test")
{
    REQUIRE(RegexMatch("HEAD /resource/{id}", "^((GET|HEAD)[[:space:]]+)?/.*$") == true);
}

TEST_CASE("regexmatch/request-payload", "Match request payload test")
{
    REQUIRE(RegexMatch("Request My Id (application/json)", "^[Rr]equest([[:space:]]+([A-Za-z0-9_]|[[:space:]])*)?([[:space:]]\\([^\\)]*\\))?$") == true);
}
