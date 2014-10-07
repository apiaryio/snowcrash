//
//  test-HeaderParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/22/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "HeadersParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer HeadersFixture = \
"+ Headers\n"\
"\n"\
"        Content-Type: application/json\n"\
"        X-My-Header: Hello World!\n";

const mdp::ByteBuffer HeadersSignatureContentFixture = \
"+ Headers\n"\
"        Content-Type: application/json\n"\
"        X-My-Header: Hello World!\n";

TEST_CASE("recognize headers signature", "[headers]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(HeadersFixture, markdownAST);
    
    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Headers>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == HeadersSectionType);
}

TEST_CASE("parse headers fixture", "[headers]")
{
    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(HeadersFixture, HeadersSectionType, headers);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.empty());

    REQUIRE(headers.node.size() == 2);
    REQUIRE(headers.node[0].first == "Content-Type");
    REQUIRE(headers.node[0].second == "application/json");
    REQUIRE(headers.node[1].first == "X-My-Header");
    REQUIRE(headers.node[1].second == "Hello World!");

    REQUIRE(headers.sourceMap.collection.size() == 2);
    REQUIRE(headers.sourceMap.collection[0].sourceMap.size() == 2);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[0].location == 15);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[0].length == 35);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[1].location == 54);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[1].length == 30);
    REQUIRE(headers.sourceMap.collection[1].sourceMap.size() == 2);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[0].location == 15);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[0].length == 35);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[1].location == 54);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[1].length == 30);
}

TEST_CASE("parse headers fixture with no empty line between signature and content", "[headers]")
{
    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(HeadersSignatureContentFixture, HeadersSectionType, headers);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.size() == 1); // content in signature

    REQUIRE(headers.node.size() == 2);
    REQUIRE(headers.node[0].first == "Content-Type");
    REQUIRE(headers.node[0].second == "application/json");
    REQUIRE(headers.node[1].first == "X-My-Header");
    REQUIRE(headers.node[1].second == "Hello World!");

    REQUIRE(headers.sourceMap.collection.size() == 2);
    REQUIRE(headers.sourceMap.collection[0].sourceMap.size() == 1);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[0].location == 0);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[0].length == 83);
    REQUIRE(headers.sourceMap.collection[1].sourceMap.size() == 1);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[0].location == 0);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[0].length == 83);
}

TEST_CASE("parse malformed headers fixture", "[headers]")
{
    mdp::ByteBuffer source = HeadersFixture;
    source += "        X-Custom-Header:\n";

    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.size() == 1); // malformed header

    REQUIRE(headers.node.size() == 2);
    REQUIRE(headers.node[0].first == "Content-Type");
    REQUIRE(headers.node[0].second == "application/json");
    REQUIRE(headers.node[1].first == "X-My-Header");
    REQUIRE(headers.node[1].second == "Hello World!");

    REQUIRE(headers.sourceMap.collection.size() == 2);
    REQUIRE(headers.sourceMap.collection[0].sourceMap.size() == 3);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[0].location == 15);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[0].length == 35);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[1].location == 54);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[1].length == 30);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[2].location == 88);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[2].length == 21);
    REQUIRE(headers.sourceMap.collection[1].sourceMap.size() == 3);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[0].location == 15);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[0].length == 35);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[1].location == 54);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[1].length == 30);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[2].location == 88);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[2].length == 21);
}

TEST_CASE("Parse header section composed of multiple blocks", "[headers]")
{
    // Blueprint in question:
    //R"(
    //+ Headers
    //    Content-Type   : text/plain
    //
    //   B : 100
    //
    //        X-My-Header: 42
    //)";

    mdp::ByteBuffer source = "+ Headers\n\n";
    source += "        Content-Type   : text/plain\n\n";
    source += "    B : 100\n\n";
    source += "        X-My-Header: 42\n";

    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.size() == 1); // not a code block

    REQUIRE(headers.node.size() == 3);
    REQUIRE(headers.node[0].first == "Content-Type");
    REQUIRE(headers.node[0].second == "text/plain");
    REQUIRE(headers.node[1].first == "B");
    REQUIRE(headers.node[1].second == "100");
    REQUIRE(headers.node[2].first == "X-My-Header");
    REQUIRE(headers.node[2].second == "42");

    REQUIRE(headers.sourceMap.collection.size() == 3);
    REQUIRE(headers.sourceMap.collection[0].sourceMap.size() == 1);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[0].location == 15);
    REQUIRE(headers.sourceMap.collection[0].sourceMap[0].length == 33);
    REQUIRE(headers.sourceMap.collection[1].sourceMap.size() == 1);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[0].location == 52);
    REQUIRE(headers.sourceMap.collection[1].sourceMap[0].length == 9);
    REQUIRE(headers.sourceMap.collection[2].sourceMap.size() == 1);
    REQUIRE(headers.sourceMap.collection[2].sourceMap[0].location == 65);
    REQUIRE(headers.sourceMap.collection[2].sourceMap[0].length == 20);
}

TEST_CASE("Parse header section with missing headers", "[headers]")
{
    mdp::ByteBuffer source = "+ Headers\n\n";

    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.size() == 1); // no headers

    REQUIRE(headers.node.size() == 0);
    REQUIRE(headers.sourceMap.collection.size() == 0);
}

TEST_CASE("Headers parses should return warning on multiple definition of same headers", "[headers]")
{
    const mdp::ByteBuffer source = \
    "+ Headers\n"\
    "\n"\
    "        Content-Type: application/json\n"\
    "        Content-Type: application/json\n";

    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.size() == 1); // no headers

    REQUIRE(headers.node.size() == 2);
}

TEST_CASE("Parse header section with more same headers Set-Cookie and Link - there should not be warning", "[headers]")
{
    const mdp::ByteBuffer source = \
    "+ Headers\n"\
    "\n"\
    "        Set-Cookie: abcd\n"\
    "        Set-Cookie: kockaprede\n";

    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.size() == 0); // no headers

    REQUIRE(headers.node.size() == 2);
}
