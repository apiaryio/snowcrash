//
//  test-SymbolTable.cc
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

TEST_CASE("Parse object resource symbol", "[symbol_table]")
{
    mdp::ByteBuffer source = \
    "# /resource\n"\
    "+ Super Model (text/plain)\n\n"\
    "          {...}\n";

    // Check we will get error parsing the same symbol again with the same symbol table
    Symbols symbols;
    buildSymbol("Super", symbols);

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ModelBodySectionType, report, resource, symbols);

    REQUIRE(report.error.code != Error::OK);
}
