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

	URIParser parser;
	URIResult result;

	parser.parse(uri,result);
	REQUIRE(result.scheme == "http");
	REQUIRE(result.host == "www.test.com");
	REQUIRE(result.path == "/other/{id}");
	REQUIRE(result.isValid == true);
}

TEST_CASE("Parse an invalid uri into seperate parts", "[invaliduriparser][issue][#79]")
{

	const snowcrash::URI uri = "http://www.test.com/other/{id}[2]";

	URIParser parser;
	URIResult result;

	parser.parse(uri, result);
	REQUIRE(result.scheme == "http");
	REQUIRE(result.host == "www.test.com");
	REQUIRE(result.path == "/other/{id}[2]");
	REQUIRE(result.isValid == false);
}
