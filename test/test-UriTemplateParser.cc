//
//  test-UriParser.c
//  snowcrash
//
//  Created by Carl Griffiths on 24/02/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "catch.hpp"
#include "Fixture.h"
#include "UriTemplateParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Parse a valid uri into seperate parts", "[validuriparser][issue][#79]")
{
	
	const snowcrash::URI uri ="http://www.test.com/other/{id}";

	URITemplateParser parser;
	ParsedURITemplate result;

	parser.parse(uri,result);
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

	parser.parse(uri, result);
	REQUIRE(result.scheme == "http");
	REQUIRE(result.host == "www.test.com");
	REQUIRE(result.path == "/other/{id}[2]");
	REQUIRE(result.warnings.size() == 1);
}

TEST_CASE("Parse uri template for invalid format curly brackets (nested brackets)", "[invalidcurlybracketparsingnested][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{id{}}";
    
    URITemplateParser parser;
    ParsedURITemplate result;

    parser.parse(uri, result);
  
    REQUIRE(result.warnings.size() == 1);
}

TEST_CASE("Parse uri template for invalid format curly brackets (missing end bracket)", "[invalidcurlybracketparsingmissingendbracket][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{id";

    URITemplateParser parser;
    ParsedURITemplate result;

    parser.parse(uri, result);

    REQUIRE(result.warnings.size() == 1);
}



TEST_CASE("Parse uri template for supported level one variable expansion", "[supportedlevelonevariableexpansionexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{id}/{test}";

    URITemplateParser parser;
    ParsedURITemplate result;

    parser.parse(uri, result);

    REQUIRE(result.warnings.size() == 0);
    
}


TEST_CASE("Parse uri template for supported level two fragment expansion", "[supportedleveltwofragmentexpansionexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{id}{#var}";

    URITemplateParser parser;
    ParsedURITemplate result;

    parser.parse(uri, result);

    REQUIRE(result.warnings.size() == 0);

}

TEST_CASE("Parse uri template for supported level three form style query string expansion", "[supportedlevelthreeformstylequeryexpansionexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{?varone,vartwo}";

    URITemplateParser parser;
    ParsedURITemplate result;

    parser.parse(uri, result);

    REQUIRE(result.warnings.size() == 0);

}

TEST_CASE("Parse uri template for supported level three reserved expansion", "[supportedlevelthreeformstylequeryexpansionexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{+varone}";

    URITemplateParser parser;
    ParsedURITemplate result;

    parser.parse(uri, result);

    REQUIRE(result.warnings.size() == 0);

}

TEST_CASE("Parse uri template for unsupported level three label expansion", "[unsupportedlevelthreelabelexpansionexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{.varone}";

    URITemplateParser parser;
    ParsedURITemplate result;

    parser.parse(uri, result);

    REQUIRE(result.warnings.size() == 1);

}

TEST_CASE("Parse uri template for unsupported level three path segment expansion", "[unsupportedlevelthreepathsegmentexpansionexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{/varone}";

    URITemplateParser parser;
    ParsedURITemplate result;

    parser.parse(uri, result);

    REQUIRE(result.warnings.size() == 1);

}

TEST_CASE("Parse uri template for unsupported level three path style parameter expansion", "[unsupportedlevelthreepathstyleparameterexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{;varone,vartwo}";

    URITemplateParser parser;
    ParsedURITemplate result;

    parser.parse(uri, result);

    REQUIRE(result.warnings.size() == 1);

}

TEST_CASE("Parse uri template for unsupported level three form style query continuation expansion", "[unsupportedlevelthreeformstylequerycontinuationexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{id}?path=test{&varone,vartwo}";

    URITemplateParser parser;
    ParsedURITemplate result;

    parser.parse(uri, result);

    REQUIRE(result.warnings.size() == 1);

}

TEST_CASE("Parse uri template for invalid variable name, contains spaces", "[invalidvariablenamecontainingspaces][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{?varone, vartwo}";

    URITemplateParser parser;
    ParsedURITemplate result;

    parser.parse(uri, result);

    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].message == "URI template expression \"?varone, vartwo\" contains spaces.");

}

TEST_CASE("Parse uri template for invalid variable name, contains hyphens", "[invalidvariablenamecontainingshyphens][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{id}{?var-one,var-two}";

    URITemplateParser parser;
    ParsedURITemplate result;

    parser.parse(uri, result);

    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].message == "URI template expression \"?var-one,var-two\" contains hyphens.");

}

TEST_CASE("Parse uri template for invalid variable name, contains assignment", "[invalidvariablenamecontainingsassignment][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{id}{?varone=vartwo}";

    URITemplateParser parser;
    ParsedURITemplate result;

    parser.parse(uri, result);

    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].message == "URI template expression \"?varone=vartwo\" contains assignment.");

}