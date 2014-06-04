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
    REQUIRE(SectionProcessor<Headers>::sectionType(markdownAST.children().begin()) == HeadersSectionType);
}

TEST_CASE("parse headers fixture", "[headers]")
{
    Headers headers;
    Report report;
    SectionParserHelper<Headers, HeadersParser>::parse(HeadersFixture, HeadersSectionType, report, headers);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(headers.size() == 2);
    REQUIRE(headers[0].first == "Content-Type");
    REQUIRE(headers[0].second == "application/json");
    REQUIRE(headers[1].first == "X-My-Header");
    REQUIRE(headers[1].second == "Hello World!");
}

TEST_CASE("parse headers fixture with no empty line between signature and content", "[headers]")
{
    Headers headers;
    Report report;
    SectionParserHelper<Headers, HeadersParser>::parse(HeadersSignatureContentFixture, HeadersSectionType, report, headers);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // content in signature

    REQUIRE(headers.size() == 2);
    REQUIRE(headers[0].first == "Content-Type");
    REQUIRE(headers[0].second == "application/json");
    REQUIRE(headers[1].first == "X-My-Header");
    REQUIRE(headers[1].second == "Hello World!");
}

TEST_CASE("parse malformed headers fixture", "[headers]")
{
    mdp::ByteBuffer source = HeadersFixture;
    source += "        X-Custom-Header:\n";

    Headers headers;
    Report report;
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, report, headers);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // malformed header

    REQUIRE(headers.size() == 2);
    REQUIRE(headers[0].first == "Content-Type");
    REQUIRE(headers[0].second == "application/json");
    REQUIRE(headers[1].first == "X-My-Header");
    REQUIRE(headers[1].second == "Hello World!");
}

TEST_CASE("hparser/parse-multiple-blocks", "Parse header section composed of multiple blocks")
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

    Headers headers;
    Report report;
    SectionParserHelper<Headers, HeadersParser>::parse(source, HeadersSectionType, report, headers);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // not a code block

    REQUIRE(headers.size() == 3);
    REQUIRE(headers[0].first == "Content-Type");
    REQUIRE(headers[0].second == "text/plain");
    REQUIRE(headers[1].first == "B");
    REQUIRE(headers[1].second == "100");
    REQUIRE(headers[2].first == "X-My-Header");
    REQUIRE(headers[2].second == "42");
}
