//
//  test-UriParser.c
//  snowcrash
//
//  Created by Carl Griffiths on 24/02/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "catch.hpp"
#include "Fixture.h"
#include "UriParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Parse a valid uri into seperate parts", "[validuriparser][issue][#79]")
{
	
	const snowcrash::URI uri ="http://www.test.com/other/{id}";

	URITemplateParser parser;
	ParsedURITemplate result;
    SourceCharactersBlock source;

	parser.parse(uri,result,source);
	REQUIRE(result.scheme == "http");
	REQUIRE(result.host == "www.test.com");
	REQUIRE(result.path == "/other/{id}");
	REQUIRE(result.errors.size() == 0);
    REQUIRE(result.warnings.size() == 0);
}

TEST_CASE("Parse an invalid uri into seperate parts", "[invaliduriparser][issue][#79]")
{

	const snowcrash::URI uri = "http://www.test.com/other/{id}[2]";

	URITemplateParser parser;
	ParsedURITemplate result;
    SourceCharactersBlock source;

	parser.parse(uri, result,source);
	REQUIRE(result.scheme == "http");
	REQUIRE(result.host == "www.test.com");
	REQUIRE(result.path == "/other/{id}[2]");
	REQUIRE(result.warnings.size() == 1);
}

TEST_CASE("Parse uri template for invalid format curly brackets (nested brackets)", "[invalidcurlybracketparsingnested][issue][#61]")
{
    const snowcrash::URI uri = "http://www.test.com/{id{}";
    
    URITemplateParser parser;
    ParsedURITemplate result;
    SourceCharactersBlock source;

    parser.parse(uri, result,source);
  
    REQUIRE(result.warnings.size() == 1);
}

TEST_CASE("Parse uri template for invalid format curly brackets (missing end bracket)", "[invalidcurlybracketparsingmissingendbracket][issue][#61]")
{
    const snowcrash::URI uri = "http://www.test.com/{id";

    URITemplateParser parser;
    ParsedURITemplate result;
    SourceCharactersBlock source;

    parser.parse(uri, result, source);

    REQUIRE(result.warnings.size() == 1);
}

TEST_CASE("Parse uri template for invalid expression slash prefix path segments", "[invalidexpressionslashprefixpathsegment][issue][#61]")
{
    const snowcrash::URI uri = "http://www.test.com/{/id}{test}";

    URITemplateParser parser;
    ParsedURITemplate result;
    SourceCharactersBlock source;

    parser.parse(uri, result, source);

    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == (int)URIWarning);
}