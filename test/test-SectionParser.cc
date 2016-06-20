//
//  test-SectionParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 6/20/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "SectionParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer HeaderSectionFixture = \
"# Signature\n"\
"\n"\
"Lorem Ipsum\n";

const mdp::ByteBuffer ListSectionFixture = \
"+ Signature\n"\
"\n"\
"    Lorem Ipsum\n";

TEST_CASE("Header adapter with header section", "[adapter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(HeaderSectionFixture, markdownAST);

    SectionParserData pd(0, HeaderSectionFixture, Blueprint());

    REQUIRE(!markdownAST.children().empty());

    REQUIRE_NOTHROW(HeaderSectionAdapter::startingNode(markdownAST.children().begin(), pd));
    MarkdownNodeIterator it = HeaderSectionAdapter::startingNode(markdownAST.children().begin(), pd);
    REQUIRE(it->text == "Signature");

    const MarkdownNodes& collection = HeaderSectionAdapter::startingNodeSiblings(it, markdownAST.children());
    REQUIRE(collection.size() == 2);

    REQUIRE(HeaderSectionAdapter::nextStartingNode(markdownAST.children().begin(), markdownAST.children(), it)->text == "Signature");
}

TEST_CASE("Header adapter with list section", "[adapter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(ListSectionFixture, markdownAST);

    SectionParserData pd(0, ListSectionFixture, Blueprint());

    REQUIRE(!markdownAST.children().empty());

    REQUIRE_THROWS_AS(HeaderSectionAdapter::startingNode(markdownAST.children().begin(), pd), Error);
}

TEST_CASE("List adapter with List section", "[adapter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(ListSectionFixture, markdownAST);

    SectionParserData pd(0, ListSectionFixture, Blueprint());

    REQUIRE(!markdownAST.children().empty());

    REQUIRE_NOTHROW(ListSectionAdapter::startingNode(markdownAST.children().begin(), pd));
    MarkdownNodeIterator it = ListSectionAdapter::startingNode(markdownAST.children().begin(), pd);
    REQUIRE(it->text == "Signature");

    MarkdownNodes collection = ListSectionAdapter::startingNodeSiblings(markdownAST.children().begin(), markdownAST.children());
    REQUIRE(collection.size() == 2);

    REQUIRE(ListSectionAdapter::nextStartingNode(markdownAST.children().begin(), markdownAST.children(), it) == markdownAST.children().end());
}

TEST_CASE("List adapter with Header section", "[adapter]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(HeaderSectionFixture, markdownAST);

    SectionParserData pd(0, HeaderSectionFixture, Blueprint());

    REQUIRE(!markdownAST.children().empty());

    REQUIRE_THROWS_AS(ListSectionAdapter::startingNode(markdownAST.children().begin(), pd), Error);
}
