//
//  test-ModelTable.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 6/9/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrash.h"
#include "snowcrashtest.h"
#include "ResourceParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Parse object resource model", "[model_table]")
{
    mdp::ByteBuffer source = \
    "# /resource\n"\
    "+ Super Model (text/plain)\n\n"\
    "          {...}\n";

    // Check we will get error parsing the same symbol again with the same symbol table
    Models models;
    ModelHelper::build("Super", models);

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ModelBodySectionType, resource, 0, models);

    REQUIRE(resource.report.error.code != Error::OK);
}
