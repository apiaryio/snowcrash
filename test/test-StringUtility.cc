//
//  test-StringUtility.cc
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

TEST_CASE("Templates for compare equality", "[utility]")
{
    SECTION("Testing equality on same types")
    {
        REQUIRE(IsEqual()(1,1));
        REQUIRE_FALSE(IsEqual()(1,2));

        REQUIRE(IsEqual()('a','a'));
        REQUIRE_FALSE(IsEqual()('a','A'));
    }

    SECTION("Testing case insensitive equality on same types")
    {
        REQUIRE(IsIEqual()('a','a'));
        REQUIRE(IsIEqual()('a','A'));
        REQUIRE(IsIEqual()('A','a'));
        REQUIRE(IsIEqual()('A','A'));

        REQUIRE_FALSE(IsIEqual()('a','b'));
    }

    SECTION("Both version should not throw while testing diferent types")
    {
        REQUIRE_NOTHROW(IsEqual()('a',1)); // allow compare diferent types - not throw
        REQUIRE_NOTHROW(IsIEqual()('a',1)); // allow compare diferent types - not throw
    }
}

TEST_CASE("Container comparation", "[utility]")
{
    REQUIRE(MatchContainers(std::string("abc"), std::string("abc"), IsEqual()));

    REQUIRE_FALSE(MatchContainers(std::string("ABC"), std::string("abc"), IsEqual()));

    REQUIRE(MatchContainers(std::string("abc"), std::string("abc"), IsIEqual()));
    REQUIRE(MatchContainers(std::string("abc"), std::string("ABC"), IsIEqual()));

    REQUIRE_FALSE(MatchContainers(std::string("def"), std::string("ABC"), IsIEqual()));
}

TEST_CASE("Comapare string", "[utility]")
{
    REQUIRE(Equal<std::string>()(std::string("abc"), std::string("abc")));
    REQUIRE_FALSE(Equal<std::string>()(std::string("abcd"), std::string("abc")));

    REQUIRE(IEqual<std::string>()(std::string("abc"), std::string("ABC")));
    REQUIRE(IEqual<std::string>()(std::string("ABC"), std::string("ABC")));
    REQUIRE_FALSE(IEqual<std::string>()(std::string("abcd"), std::string("abc")));
}

TEST_CASE("Remove markdown link")
{
    REQUIRE(StripMarkdownLink("[Google][]") == "Google");
    REQUIRE(StripMarkdownLink("[Google] [google]") == "Google");
    REQUIRE(StripMarkdownLink("[ Google](https://google.com)") == "Google");
}

TEST_CASE("Retrieve escaped string")
{
    std::string subject;

    subject = "a```b```cd";
    REQUIRE(RetrieveEscaped(subject, 1) == "```b```");
    REQUIRE(subject == "cd");

    subject = "*rel (custom)*";
    REQUIRE(RetrieveEscaped(subject) == "*rel (custom)*");
    REQUIRE(subject == "");

    subject = "site_admin";
    REQUIRE(RetrieveEscaped(subject, 4) == "");
    REQUIRE(subject == "site_admin");
}
