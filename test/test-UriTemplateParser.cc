//
//  test-UriParser.c
//  snowcrash
//
//  Created by Carl Griffiths on 24/02/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "catch.hpp"
#include "UriTemplateParser.h"

using namespace snowcrash;

TEST_CASE("Parse a valid uri into seperate parts", "[validuriparser][issue][#79]")
{

    const snowcrash::URI uri ="http://www.test.com/other/{id}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);
    REQUIRE(result.scheme == "http");
    REQUIRE(result.host == "www.test.com");
    REQUIRE(result.path == "/other/{id}");
    REQUIRE(result.report.warnings.size() == 0);
}

TEST_CASE("Parse an invalid uri into seperate parts", "[invaliduriparser][issue][#79]")
{

    const snowcrash::URI uri = "http://www.test.com/other/{id}[2]";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);
    REQUIRE(result.scheme == "http");
    REQUIRE(result.host == "www.test.com");
    REQUIRE(result.path == "/other/{id}[2]");
    REQUIRE(result.report.warnings.size() == 1);
}

TEST_CASE("Parse uri template for invalid format curly brackets (nested brackets)", "[invalidcurlybracketparsingnested][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{id{}}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 1);
}

TEST_CASE("Parse uri template for invalid format curly brackets (missing end bracket)", "[invalidcurlybracketparsingmissingendbracket][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{id";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 1);
}

TEST_CASE("Parse uri template for supported level one variable expansion", "[supportedlevelonevariableexpansionexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{id}/{test}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 0);

}

TEST_CASE("Parse uri template for supported variables with % encoding and explode modifier", "[supportedpercentencodedvariablesandexplodemodifier][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{id}/{?test%20one,test%20two*}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 0);

}

TEST_CASE("Parse uri template for invalid % encoding", "[invalidpercentencoding][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{id}/{?test%20one,test%2Ztwo}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 1);

}

TEST_CASE("Parse uri template for invalid expansion", "[invalidexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{@id}/{|test}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 2);

}


TEST_CASE("Parse uri template for supported level two fragment expansion", "[supportedleveltwofragmentexpansionexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{id}{#var}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 0);

}

TEST_CASE("Parse uri template for supported level three form style query string expansion", "[supportedlevelthreeformstylequeryexpansionexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{?varone,vartwo}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 0);

}

TEST_CASE("Parse uri template for supported level three reserved expansion", "[supportedlevelthreeformstylequeryexpansionexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{+varone}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 0);

}

TEST_CASE("Parse uri template for unsupported level three label expansion", "[unsupportedlevelthreelabelexpansionexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{.varone}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 1);

}

TEST_CASE("Parse uri template for unsupported level three path segment expansion", "[unsupportedlevelthreepathsegmentexpansionexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{/varone}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 1);

}

TEST_CASE("Parse uri template for unsupported level three path style parameter expansion", "[unsupportedlevelthreepathstyleparameterexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{;varone,vartwo}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 1);

}

TEST_CASE("Parse uri template for supported level three form style query continuation expansion", "[supportedlevelthreeformstylequerycontinuationexpression][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{id}?path=test{&varone,vartwo}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 0);

}

TEST_CASE("Parse uri template for invalid variable name, contains spaces", "[invalidvariablenamecontainingspaces][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/path{?varone, vartwo}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 1);
    REQUIRE(result.report.warnings[0].message == "URI template expression \"?varone, vartwo\" contains spaces. Allowed characters for expressions are A-Z a-z 0-9 _ and percent encoded characters");

}

TEST_CASE("Parse uri template for invalid variable name, contains hyphens", "[invalidvariablenamecontainingshyphens][issue][#78]")
{
    const snowcrash::URI uri = "http://www.test.com/{id}{?var-one,var-two}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 1);
    REQUIRE(result.report.warnings[0].message == "URI template expression \"?var-one,var-two\" contains hyphens. Allowed characters for expressions are A-Z a-z 0-9 _ and percent encoded characters");

}

TEST_CASE("Parse uri template for invalid variable name, contains assignment", "[invalidvariablenamecontainingsassignment][issue][#78]")
{
    const snowcrash::URITemplate uri = "http://www.test.com/{id}{?varone=vartwo}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 1);
    REQUIRE(result.report.warnings[0].message == "URI template expression \"?varone=vartwo\" contains assignment. Allowed characters for expressions are A-Z a-z 0-9 _ and percent encoded characters");

}

TEST_CASE("Parse uri template for invalid variable name, invalid % encoded", "[invalidvariablenamecontainingsbadpctencoding][issue][#78]")
{
    const snowcrash::URITemplate uri = "http://www.test.com/{id}{?varone%2z}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 1);
    REQUIRE(result.report.warnings[0].message == "URI template expression \"?varone%2z\" contains invalid characters. Allowed characters for expressions are A-Z a-z 0-9 _ and percent encoded characters");

}

TEST_CASE("Parse uri template for variable name containing dot", "[validvariablenamecontainingdot][issue][#78]")
{
    const snowcrash::URITemplate uri = "http://www.test.com/{id}{?varone.data}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 0);
}

TEST_CASE("Parse uri template for invalid variable name containing multiple contiguous dots", "[invalidvariablenamecontiguousdots][issue][#78]")
{
    const snowcrash::URITemplate uri = "http://www.test.com/{id}{?varone..data}";

    URITemplateParser parser;
    ParsedURITemplate result;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(uri, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 1);
}

TEST_CASE("Parse uri template for consistent invalid character warning", "[invalidexpressionconsistentinvalidcharacterwarning][issue][#78]")
{
    const snowcrash::URITemplate urione = "http://www.test.com/{$a,b,c}";
    const snowcrash::URITemplate uritwo = "http://www.test.com/{@a,b,c}";

    URITemplateParser parser;
    ParsedURITemplate result;
    ParsedURITemplate result2;
    mdp::CharactersRangeSet sourceBlock;

    parser.parse(urione, sourceBlock, result);

    REQUIRE(result.report.warnings.size() == 1);
    REQUIRE(result.report.warnings[0].message == "URI template expression \"$a,b,c\" contains invalid characters. Allowed characters for expressions are A-Z a-z 0-9 _ and percent encoded characters");

    parser.parse(urione, sourceBlock, result2);

    REQUIRE(result2.report.warnings.size() == 1);
    REQUIRE(result2.report.warnings[0].message == "URI template expression \"$a,b,c\" contains invalid characters. Allowed characters for expressions are A-Z a-z 0-9 _ and percent encoded characters");

}
