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
    SectionParserHelper<Headers, HeadersParser>::parse(HeadersFixture, HeadersSectionType, headers, ExportSourcemapOption);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.empty());

    REQUIRE(headers.node.size() == 2);
    REQUIRE(headers.node[0].first == "Content-Type");
    REQUIRE(headers.node[0].second == "application/json");
    REQUIRE(headers.node[1].first == "X-My-Header");
    REQUIRE(headers.node[1].second == "Hello World!");

    REQUIRE(headers.sourceMap.collection.size() == 2);
    SourceMapHelper::check(headers.sourceMap.collection[0].sourceMap, 15, 35, 54, 30);
    SourceMapHelper::check(headers.sourceMap.collection[1].sourceMap, 15, 35, 54, 30);
}

TEST_CASE("parse headers fixture with no empty line between signature and content", "[headers]")
{
    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(HeadersSignatureContentFixture, HeadersSectionType, headers, ExportSourcemapOption);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.size() == 1); // content in signature

    REQUIRE(headers.node.size() == 2);
    REQUIRE(headers.node[0].first == "Content-Type");
    REQUIRE(headers.node[0].second == "application/json");
    REQUIRE(headers.node[1].first == "X-My-Header");
    REQUIRE(headers.node[1].second == "Hello World!");

    REQUIRE(headers.sourceMap.collection.size() == 2);
    SourceMapHelper::check(headers.sourceMap.collection[0].sourceMap, 0, 83);
    SourceMapHelper::check(headers.sourceMap.collection[1].sourceMap, 0, 83);
}

TEST_CASE("parse malformed headers fixture", "[headers]")
{
    mdp::ByteBuffer source = HeadersFixture;
    source += "        X-Custom-Header:\n";

    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers, ExportSourcemapOption);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.size() == 1); // malformed header

    REQUIRE(headers.node.size() == 3);
    REQUIRE(headers.node[0].first == "Content-Type");
    REQUIRE(headers.node[0].second == "application/json");
    REQUIRE(headers.node[1].first == "X-My-Header");
    REQUIRE(headers.node[1].second == "Hello World!");
    REQUIRE(headers.node[2].first == "X-Custom-Header");
    REQUIRE(headers.node[2].second == "");

    REQUIRE(headers.sourceMap.collection.size() == 3);

    REQUIRE(headers.sourceMap.collection[0].sourceMap.size() == 3);
    SourceMapHelper::check(headers.sourceMap.collection[0].sourceMap, 15, 35, 1);
    SourceMapHelper::check(headers.sourceMap.collection[0].sourceMap, 54, 30, 2);
    SourceMapHelper::check(headers.sourceMap.collection[0].sourceMap, 88, 21, 3);

    REQUIRE(headers.sourceMap.collection[1].sourceMap.size() == 3);
    SourceMapHelper::check(headers.sourceMap.collection[0].sourceMap, 15, 35, 1);
    SourceMapHelper::check(headers.sourceMap.collection[0].sourceMap, 54, 30, 2);
    SourceMapHelper::check(headers.sourceMap.collection[0].sourceMap, 88, 21, 3);

    REQUIRE(headers.sourceMap.collection[2].sourceMap.size() == 3);
    SourceMapHelper::check(headers.sourceMap.collection[0].sourceMap, 15, 35, 1);
    SourceMapHelper::check(headers.sourceMap.collection[0].sourceMap, 54, 30, 2);
    SourceMapHelper::check(headers.sourceMap.collection[0].sourceMap, 88, 21, 3);
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
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers, ExportSourcemapOption);

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
    SourceMapHelper::check(headers.sourceMap.collection[0].sourceMap, 15, 33);
    SourceMapHelper::check(headers.sourceMap.collection[1].sourceMap, 52, 9);
    SourceMapHelper::check(headers.sourceMap.collection[2].sourceMap, 65, 20);
}

TEST_CASE("Parse header section with missing headers", "[headers]")
{
    mdp::ByteBuffer source = "+ Headers\n\n";

    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers, ExportSourcemapOption);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.size() == 1); // no headers

    REQUIRE(headers.node.size() == 0);
    REQUIRE(headers.sourceMap.collection.size() == 0);
}

TEST_CASE("Headers parses should return warning on multiple definition of same headers", "[headers][issue][75]")
{
    const mdp::ByteBuffer source = \
    "+ Headers\n"\
    "\n"\
    "        Content-Type: application/json\n"\
    "        Content-Type: application/json\n";

    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.size() == 1); // one warning due to multiple declaration same header

    REQUIRE(headers.node.size() == 2);
}

TEST_CASE("Parse header section with more same headers Set-Cookie and Link - there should not be warning", "[headers][issue][75]")
{
    const mdp::ByteBuffer source = \
    "+ Headers\n"\
    "\n"\
    "        Set-Cookie: abcd\n"\
    "        Set-Cookie: kockaprede\n";

    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.empty()); // no warning - multiple definition, but from allowed set

    REQUIRE(headers.node.size() == 2);
    REQUIRE(headers.node[0].first == "Set-Cookie");
    REQUIRE(headers.node[0].second == "abcd");
    REQUIRE(headers.node[1].first == "Set-Cookie");
    REQUIRE(headers.node[1].second == "kockaprede");
}

TEST_CASE("Headers Filed Name should be case insensitive", "[headers][issue][230]")
{
    const mdp::ByteBuffer source = \
    "+ Headers\n"\
    "\n"\
    "        Content-Type: application/json\n"\
    "        content-type: application/json\n";

    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.size() == 1); // one warning - multiple definitions w/ different case sensitivity

    REQUIRE(headers.node.size() == 2);
    REQUIRE(headers.node[0].first == "Content-Type");
    REQUIRE(headers.node[0].second == "application/json");
    REQUIRE(headers.node[1].first == "content-type");
    REQUIRE(headers.node[1].second == "application/json");
}

TEST_CASE("Aditional test for Header name insenstivity combined with allowed multiple headers", "[headers][issue][230]")
{
    const mdp::ByteBuffer source = \
    "+ Headers\n"\
    "\n"\
    "        Set-cookie: abcd\n"\
    "        Set-Cookie: kockaprede\n";

    ParseResult<Headers> headers;
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

    REQUIRE(headers.report.error.code == Error::OK);
    REQUIRE(headers.report.warnings.empty()); // no warning - multiple definition, but from allowed set

    REQUIRE(headers.node.size() == 2);
    REQUIRE(headers.node[0].first == "Set-cookie");
    REQUIRE(headers.node[0].second == "abcd");
    REQUIRE(headers.node[1].first == "Set-Cookie");
    REQUIRE(headers.node[1].second == "kockaprede");
}

TEST_CASE("Missing or wrong placed colon in header definition", "[headers][issue][158]")
{


    SECTION("No colon") {
        const mdp::ByteBuffer source = \
        "+ Headers\n\n"\
        "        Set-Cookie chocolate cookie\n";

        ParseResult<Headers> headers;
        SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

        REQUIRE(headers.report.error.code == Error::OK); // no error

        REQUIRE(headers.report.warnings.size() == 1);    // warning - header is not defined correctly
        REQUIRE(headers.report.warnings[0].message == "missing colon after header name 'Set-Cookie'");

        REQUIRE(headers.node[0].first == "Set-Cookie");
        REQUIRE(headers.node[0].second == "chocolate cookie");
    }

    SECTION("Missing colon - but there is another in value") {
        const mdp::ByteBuffer source = \
        "+ Headers\n\n"\
        "        Last-Modified Sat, 02 Aug 2014 23:10:05 GMT\n";

        ParseResult<Headers> headers;
        SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

        REQUIRE(headers.report.error.code == Error::OK); // no error

        REQUIRE(headers.report.warnings.size() == 1);    // warning - header is not defined correctly
        REQUIRE(headers.report.warnings[0].message == "missing colon after header name 'Last-Modified'");

        REQUIRE(headers.node[0].first == "Last-Modified");
        REQUIRE(headers.node[0].second == "Sat, 02 Aug 2014 23:10:05 GMT");
    }

    SECTION("More colons") {
        const mdp::ByteBuffer source = \
        "+ Headers\n\n"\
        "        Set-Cookie :: chocolate cookie\n";

        ParseResult<Headers> headers;
        SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

        REQUIRE(headers.report.error.code == Error::OK); // no error
        REQUIRE(headers.report.warnings.empty());

        REQUIRE(headers.node[0].first == "Set-Cookie");
        REQUIRE(headers.node[0].second == ": chocolate cookie");
    }
}

TEST_CASE("Allow parse nonvalid headers, provide apropriate warning", "[headers][issue][158]") {

    SECTION("Strange but valid token") {
        const mdp::ByteBuffer source = \
        "+ Headers\n\n"\
        "        # : chocolate cookie\n";

        ParseResult<Headers> headers;
        SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

        REQUIRE(headers.report.error.code == Error::OK); // no error
        REQUIRE(headers.report.warnings.empty());

        REQUIRE(headers.node[0].first == "#");
        REQUIRE(headers.node[0].second == "chocolate cookie");
    }

    SECTION("Header has no value - just name") {
        const mdp::ByteBuffer source = \
        "+ Headers\n\n"\
        "        Header:\n";

        ParseResult<Headers> headers;
        SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

        REQUIRE(headers.report.error.code == Error::OK); // no error

        REQUIRE(headers.report.warnings.size() == 1);    // warning - header name is not defined correctly
        REQUIRE(headers.report.warnings[0].message == "HTTP header 'Header' has no value");

        REQUIRE(headers.node.size() == 1);
        REQUIRE(headers.node[0].first == "Header");
        REQUIRE(headers.node[0].second == "");
    }

    SECTION("Header has no value and there is no colon presented ") {
        const mdp::ByteBuffer source = \
        "+ Headers\n\n"\
        "        Header\n";

        ParseResult<Headers> headers;
        SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

        REQUIRE(headers.report.error.code == Error::OK); // no error

        REQUIRE(headers.report.warnings.size() == 2);
        REQUIRE(headers.report.warnings[0].message == "missing colon after header name 'Header'");
        REQUIRE(headers.report.warnings[1].message == "HTTP header 'Header' has no value");

        REQUIRE(headers.node.size() == 1);
        REQUIRE(headers.node[0].first == "Header");
        REQUIRE(headers.node[0].second == "");
    }

}

TEST_CASE("Skip completely invalid headers", "[headers][drafter-issue][382]") {
    
    SECTION("Invalid characters in header name") {
        const mdp::ByteBuffer source = \
        "+ Headers\n\n"\
        "        <Header> : Invalid\n"\
        "        Header : chocolate cookie\n";

        ParseResult<Headers> headers;
        SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

        REQUIRE(headers.report.error.code == Error::OK); // no error

        REQUIRE(headers.report.warnings.size() == 1);    // warning - header name is not defined correctly
        REQUIRE(headers.report.warnings[0].message == "HTTP header name '<Header>' contains illegal character '<' (0x3c) skipping the header");

        REQUIRE(headers.node.size() == 1);
        REQUIRE(headers.node[0].first == "Header");
        REQUIRE(headers.node[0].second == "chocolate cookie");
    }

    SECTION("Invalid characters in the only header name") {
        const mdp::ByteBuffer source = \
        "+ Headers\n\n"\
        "        <Header> : Invalid\n";

        ParseResult<Headers> headers;
        SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

	REQUIRE(headers.report.error.code == Error::OK); // no error

	REQUIRE(headers.report.warnings.size() == 2);    // warning - header name is not defined correctly
	REQUIRE(headers.report.warnings[0].message == "HTTP header name '<Header>' contains illegal character '<' (0x3c) skipping the header");
	REQUIRE(headers.report.warnings[1].message == "no valid headers specified");

    }

    SECTION("Invalid header") {
        const mdp::ByteBuffer source = \
        "+ Headers\n\n"\
        "        :Header: :\n";

        ParseResult<Headers> headers;
        SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, headers);

	REQUIRE(headers.report.error.code == Error::OK); // no error

	REQUIRE(headers.report.warnings.size() == 2);    // warning - header name is not defined correctly
	REQUIRE(headers.report.warnings[0].message == "unable to parse HTTP header, expected '<header name> : <header value>', one header per line");
	REQUIRE(headers.report.warnings[1].message == "no valid headers specified");
    }
}

