//
//  test-MSONMixinParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONMixinParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Mixin block classifier", "[mson][mixin]")
{
    mdp::ByteBuffer source = \
    "- Include Person";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(markdownAST.children().front().type == mdp::ListItemMarkdownNodeType);
    REQUIRE(!markdownAST.children().front().children().empty());

    sectionType = SectionProcessor<mson::Mixin>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONMixinSectionType);

    markdownAST.children().front().children().front().text = "Include (Address, sample)";
    sectionType = SectionProcessor<mson::Mixin>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONMixinSectionType);
}
