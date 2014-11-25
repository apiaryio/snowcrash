//
//  test-AttributesParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/25/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "AttributesParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer AttributesFixture = \
"+ Attributes (array[[Coupon](#coupon)])";

TEST_CASE("Recognize explicit attributes signature", "[attributes]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(AttributesFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<Attributes>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == AttributesSectionType);
}
