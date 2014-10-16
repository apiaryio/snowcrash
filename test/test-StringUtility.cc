//
//  test/test-StringUtils.cc
//  snowcrash
//
//  Created by Jiri Kratochvil on 10/15/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "StringUtility.h"
#include "BlueprintUtility.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("templates for compare equality", "[utility]")
{
    SECTION("testing equality on same types") {
        REQUIRE(IsEqual()(1,1));
        REQUIRE_FALSE(IsEqual()(1,2));

        REQUIRE(IsEqual()('a','a'));
        REQUIRE_FALSE(IsEqual()('a','A'));
    }

    SECTION("testing case insensitive equality on same types") {

        REQUIRE(IsIEqual()('a','a'));
        REQUIRE(IsIEqual()('a','A'));
        REQUIRE(IsIEqual()('A','a'));
        REQUIRE(IsIEqual()('A','A'));

        REQUIRE_FALSE(IsIEqual()('a','b'));
    }

    SECTION("both version should not throw while testing diferent types") {
        REQUIRE_NOTHROW(IsEqual()('a',1)); // allow compare diferent types - not throw
        REQUIRE_NOTHROW(IsIEqual()('a',1)); // allow compare diferent types - not throw
    }

}

TEST_CASE("container comparation", "[utility]"){

    REQUIRE(MatchContainers(std::string("abc"), std::string("abc"), IsEqual()));

    REQUIRE_FALSE(MatchContainers(std::string("ABC"), std::string("abc"), IsEqual()));

    REQUIRE(MatchContainers(std::string("abc"), std::string("abc"), IsIEqual()));
    REQUIRE(MatchContainers(std::string("abc"), std::string("ABC"), IsIEqual()));

    REQUIRE_FALSE(MatchContainers(std::string("def"), std::string("ABC"), IsIEqual()));
}

TEST_CASE("comapare string", "[utility]"){

    REQUIRE(Equal<std::string>()(std::string("abc"), std::string("abc")));
    REQUIRE_FALSE(Equal<std::string>()(std::string("abcd"), std::string("abc")));

    REQUIRE(IEqual<std::string>()(std::string("abc"), std::string("ABC")));
    REQUIRE(IEqual<std::string>()(std::string("ABC"), std::string("ABC")));
    REQUIRE_FALSE(IEqual<std::string>()(std::string("abcd"), std::string("abc")));

}

