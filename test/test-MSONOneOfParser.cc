//
//  test-MSONOneOfParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONOneOfParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("OneOf block classifier", "[mson][one_of]")
{
    mdp::ByteBuffer source = \
    "- one Of";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(markdownAST.children().front().type == mdp::ListItemMarkdownNodeType);
    REQUIRE(!markdownAST.children().front().children().empty());

    sectionType = SectionProcessor<mson::OneOf>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONOneOfSectionType);

    markdownAST.children().front().children().front().text = "One of";
    sectionType = SectionProcessor<mson::OneOf>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONOneOfSectionType);
}
