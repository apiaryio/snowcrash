//
//  test-SymbolTable.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 6/9/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "ResourceParser.h"
#include "ResourceGroupParser.h"
#include "Fixture.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("symbol/resource-object", "Parse object resource symbol")
{
    MarkdownBlock::Stack markdown = CanonicalResourceFixture();
    Resource resource;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ResourceParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, resource);
    
    REQUIRE(result.first.error.code == Error::OK);
    
    REQUIRE(parser.symbolTable.resourceModels.size() == 1);
    ResourceModelSymbolTable::iterator it = parser.symbolTable.resourceModels.find("My Resource");
    REQUIRE(it != parser.symbolTable.resourceModels.end());
    REQUIRE(it->first == "My Resource");
    REQUIRE(it->second.body == "X.O.");
    
    // Check we will get error parsing the same symbol again with the same symbol table
    BlueprintParserCore parser2(0, SourceDataFixture, Blueprint());
    parser2.symbolTable = parser.symbolTable;
    Resource resource2;
    ParseSectionResult result2 = ResourceParser::Parse(markdown.begin(), markdown.end(), rootSection, parser2, resource2);
    REQUIRE(result2.first.error.code != Error::OK);
}
