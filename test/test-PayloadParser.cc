//
//  test-PayloadParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/7/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "PayloadParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer RequestFixture = \
"+ Request Hello World (text/plain)\n\n"\
"  Description\n\n"\
"    + Headers\n\n"\
"            X-Header: 42\n\n"\
"    + Body\n\n"\
"            Code\n\n"\
"    + Schema\n\n"\
"            Code 2\n\n";

const mdp::ByteBuffer RequestBodyFixture = \
"+ Request A\n\n"\
"        Hello World!\n";

const mdp::ByteBuffer ResponseBodyFixture = \
"+ Response 200 (text/plain)\n\n"\
"        Hello World!\n";

const mdp::ByteBuffer SymbolFixture = \
"+ Request\n\n"\
"    [Symbol][]\n";

const mdp::ByteBuffer EmptyBodyFixture = \
"+ Response 200\n\n"\
"    + Body\n\n\n\n";

TEST_CASE("recognize request signature", "[payload]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(RequestFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Payload>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == RequestSectionType);
}

TEST_CASE("recognize abbreviated request signature", "[payload]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(RequestBodyFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Payload>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == RequestBodySectionType);
}

TEST_CASE("recognize abbreviated response signature", "[payload]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(ResponseBodyFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Payload>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ResponseBodySectionType);
}

TEST_CASE("recognize empty body response signature as non-abbreviated", "[payload]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(EmptyBodyFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    SectionType sectionType = SectionProcessor<Payload>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ResponseSectionType);
}

TEST_CASE("Parse request payload", "[payload]")
{
    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(RequestFixture, RequestSectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    CHECK(payload.report.warnings.empty());

    REQUIRE(payload.node.name == "Hello World");
    REQUIRE(payload.node.description == "Description\n");
    REQUIRE(payload.node.parameters.empty());
    REQUIRE(payload.node.headers.size() == 2);
    REQUIRE(payload.node.headers[0].first == "Content-Type");
    REQUIRE(payload.node.headers[0].second == "text/plain");
    REQUIRE(payload.node.headers[1].first == "X-Header");
    REQUIRE(payload.node.headers[1].second == "42");
    REQUIRE(payload.node.body == "Code\n");
    REQUIRE(payload.node.schema == "Code 2\n");
}

TEST_CASE("Parse abbreviated payload body", "[payload]")
{
    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(ResponseBodyFixture, ResponseBodySectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    CHECK(payload.report.warnings.empty());

    REQUIRE(payload.node.name == "200");
    REQUIRE(payload.node.description.empty());
    REQUIRE(payload.node.parameters.empty());
    REQUIRE(payload.node.headers.size() == 1);
    REQUIRE(payload.node.body == "Hello World!\n");
    REQUIRE(payload.node.schema.empty());
}

TEST_CASE("Parse abbreviated inline payload body", "[payload]")
{
    mdp::ByteBuffer source = RequestBodyFixture;
    source += "  B\n";

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestBodySectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    CHECK(payload.report.warnings.size() == 1); // preformatted code block

    REQUIRE(payload.node.name == "A");
    REQUIRE(payload.node.description.empty());
    REQUIRE(payload.node.parameters.empty());
    REQUIRE(payload.node.headers.empty());
    REQUIRE(payload.node.body == "Hello World!\nB\n");
    REQUIRE(payload.node.schema.empty());
}

TEST_CASE("Parse payload description with list", "[payload]")
{
    // Blueprint in question:
    //R"(
    //+ Request
    //
    //    + B
    //
    //    + Body
    //
    //            {}
    //");

    mdp::ByteBuffer source = \
    "+ Request\n\n"\
    "    + B\n\n"\
    "    + Body\n\n"\
    "            {}\n";

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    CHECK(payload.report.warnings.empty());

    REQUIRE(payload.node.name.empty());
    REQUIRE(payload.node.description == "+ B\n");
    REQUIRE(payload.node.parameters.empty());
    REQUIRE(payload.node.headers.empty());
    REQUIRE(payload.node.body == "{}\n");
    REQUIRE(payload.node.schema.empty());
}

TEST_CASE("Parse payload with foreign list item", "[payload]")
{
    // Blueprint in question:
    //R"(
    //+ Request
    //
    //    + Body
    //
    //            {}
    //
    //    + Bar
    //");

    mdp::ByteBuffer source = \
    "+ Request\n\n"\
    "    + Body\n\n"\
    "            {}\n\n"\
    "    + Bar\n";

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    CHECK(payload.report.warnings.size() == 1); // dangling block

    REQUIRE(payload.node.name.empty());
    REQUIRE(payload.node.description.empty());
    REQUIRE(payload.node.parameters.empty());
    REQUIRE(payload.node.headers.empty());
    REQUIRE(payload.node.body == "{}\n");
    REQUIRE(payload.node.schema.empty());
}

TEST_CASE("Parse payload with dangling body", "[payload]")
{
    // Blueprint in question:
    //R"(
    //+ Request
    //    + Body
    //
    //            Foo
    //
    //    Bar
    //");

    mdp::ByteBuffer source = "+ Request\n";
    source += "    + Body\n\n";
    source += "            Foo\n\n";
    source += "    Bar\n";

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    REQUIRE(payload.report.warnings.size() == 1); // dangling block
    REQUIRE(payload.report.warnings[0].code == IndentationWarning);

    REQUIRE(payload.node.name.empty());
    REQUIRE(payload.node.description.empty());
    REQUIRE(payload.node.parameters.empty());
    REQUIRE(payload.node.headers.empty());
    REQUIRE(payload.node.body == "Foo\nBar\n\n");
    REQUIRE(payload.node.schema.empty());
}

TEST_CASE("Parse inline payload with symbol reference", "[payload]")
{
    Symbols symbols;
    buildSymbol("Symbol", symbols);

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(SymbolFixture, RequestBodySectionType, report, payload, symbols);

    REQUIRE(payload.report.error.code == Error::OK);
    REQUIRE(payload.report.warnings.size() == 0);

    REQUIRE(payload.node.name.empty());
    REQUIRE(payload.node.description == "Foo");
    REQUIRE(payload.node.parameters.empty());
    REQUIRE(payload.node.headers.empty());
    REQUIRE(payload.node.body == "Bar");
    REQUIRE(payload.node.schema.empty());
}

TEST_CASE("Parse inline payload with symbol reference with extra indentation", "[payload]")
{
    mdp::ByteBuffer source = \
    "+ Request\n\n"\
    "        [Symbol][]\n";

    Symbols symbols;
    buildSymbol("Symbol", symbols);

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestBodySectionType, report, payload, symbols);

    REQUIRE(payload.report.error.code == Error::OK);
    REQUIRE(payload.report.warnings.size() == 0);

    REQUIRE(payload.node.name.empty());
    REQUIRE(payload.node.description.empty());
    REQUIRE(payload.node.parameters.empty());
    REQUIRE(payload.node.headers.empty());
    REQUIRE(payload.node.body == "[Symbol][]\n");
    REQUIRE(payload.node.schema.empty());
}

TEST_CASE("Parse inline payload with symbol reference with foreign content", "[payload]")
{
    mdp::ByteBuffer source = SymbolFixture;
    source += "\n    Foreign\n";

    Symbols symbols;
    buildSymbol("Symbol", symbols);

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestBodySectionType, report, payload, symbols);

    REQUIRE(payload.report.error.code == Error::OK);
    REQUIRE(payload.report.warnings.size() == 1); // ignoring foreign entry

    REQUIRE(payload.node.name.empty());
    REQUIRE(payload.node.description == "Foo");
    REQUIRE(payload.node.parameters.empty());
    REQUIRE(payload.node.headers.empty());
    REQUIRE(payload.node.body == "Bar");
    REQUIRE(payload.node.schema.empty());
}

TEST_CASE("Parse named model", "[payload]")
{
    // Blueprint in question:
    //R"(
    //+ Super Model (text/plain)
    //
    //        Hello World!
    //");

    mdp::ByteBuffer source = "+ Super Model (text/plain)\n\n";
    source += "        Hello World!\n";

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, ModelBodySectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    REQUIRE(payload.report.warnings.empty());

    REQUIRE(payload.node.name == "Super");
    REQUIRE(payload.node.description.empty());
    REQUIRE(payload.node.parameters.empty());
    REQUIRE(payload.node.headers.size() == 1);
    REQUIRE(payload.node.headers[0].first == "Content-Type");
    REQUIRE(payload.node.headers[0].second == "text/plain");
    REQUIRE(payload.node.body == "Hello World!\n");
    REQUIRE(payload.node.schema.empty());
}

TEST_CASE("Parse nameless model", "[payload]")
{
    // Blueprint in question:
    //R"(
    //+ Model (text/plain)
    //
    //        Hello World!
    //");

    mdp::ByteBuffer source = "+ Model (text/plain)\n\n";
    source += "        Hello World!\n";

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, ModelBodySectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    REQUIRE(payload.report.warnings.empty());

    REQUIRE(payload.node.name.empty());
    REQUIRE(payload.node.description.empty());
    REQUIRE(payload.node.parameters.empty());
    REQUIRE(payload.node.headers.size() == 1);
    REQUIRE(payload.node.headers[0].first == "Content-Type");
    REQUIRE(payload.node.headers[0].second == "text/plain");
    REQUIRE(payload.node.body == "Hello World!\n");
    REQUIRE(payload.node.schema.empty());
}

TEST_CASE("Warn on malformed payload signature", "[payload]")
{
    // Blueprint in question:
    //R"(
    //+ Request This is FUN[ (text/plain)
    //  Description
    //
    //   Line 2
    //
    //    + Body
    //
    //            Hello World!
    //");

    mdp::ByteBuffer source = "+ Request This is FUN[ (text/plain)\n";
    source += "  Description\n\n";
    source += "   Line 2\n\n";
    source += "    + Body\n\n";
    source += "            Hello World!\n";

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    REQUIRE(payload.report.warnings.size() == 1);
    REQUIRE(payload.report.warnings[0].code == FormattingWarning);

    REQUIRE(payload.node.name.empty());
    REQUIRE(payload.node.description == "Description\n\nLine 2\n");
    REQUIRE(payload.node.parameters.empty());
    REQUIRE(payload.node.headers.empty());
    REQUIRE(payload.node.body == "Hello World!\n");
    REQUIRE(payload.node.schema.empty());
}

TEST_CASE("Give a warning of empty message body for requests with certain headers", "[payload]")
{
    mdp::ByteBuffer source = \
    "+ Request\n"\
    "    + Headers \n"\
    "\n"\
    "            Content-Length: 100\n";

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    REQUIRE(payload.report.warnings.size() == 1);
    REQUIRE(payload.report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(payload.node.headers.size() == 1);
    REQUIRE(payload.node.headers[0].first == "Content-Length");
    REQUIRE(payload.node.headers[0].second == "100");
    REQUIRE(payload.node.body.empty());
}

TEST_CASE("Do not report empty message body for requests", "[payload]")
{
    mdp::ByteBuffer source = \
    "+ Request\n"\
    "    + Headers \n"\
    "\n"\
    "            Accept: application/json, application/javascript\n";

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    REQUIRE(payload.report.warnings.empty());

    REQUIRE(payload.node.headers.size() == 1);
    REQUIRE(payload.node.headers[0].first == "Accept");
    REQUIRE(payload.node.headers[0].second == "application/json, application/javascript");
    REQUIRE(payload.node.body.empty());
}

TEST_CASE("Give a warning when 100 response has a body", "[payload]")
{
    mdp::ByteBuffer source = \
    "+ Response 100\n\n"\
    "        {}\n";

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, ResponseBodySectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    REQUIRE(payload.report.warnings.size() == 1);
    REQUIRE(payload.report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(payload.node.body == "{}\n");
}

TEST_CASE("Empty body section should shouldn't be parsed as description", "[payload]")
{
    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(EmptyBodyFixture, ResponseSectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    REQUIRE(payload.report.warnings.empty());

    REQUIRE(payload.node.body == "");
}

TEST_CASE("Parameters section should be taken as a description node", "[payload]")
{
    mdp::ByteBuffer source = \
    "+ Response 200\n\n"\
    "    + Parameters\n\n"\
    "        + id (string)\n\n"\
    "    + Body\n\n"\
    "            {}\n";

    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, ResponseSectionType, payload);

    REQUIRE(payload.report.error.code == Error::OK);
    REQUIRE(payload.report.warnings.empty());

    REQUIRE(payload.node.description == "+ Parameters\n\n    + id (string)\n");
    REQUIRE(payload.node.body == "{}\n");
}

TEST_CASE("Report ignoring nested request objects", "[payload][#163][#189]")
{
    mdp::ByteBuffer source = \
    "+ Request\n"\
    "    + Headers\n"\
    "\n"\
    "            Authorization: Basic AAAAA\n"\
    "\n"\
    "    + Request (application/x-www-form-urlencoded)\n"\
    "\n"\
    "            Hello World\n";
    
    ParseResult<Payload> payload;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, payload);
    
    REQUIRE(payload.report.error.code == Error::OK);
    REQUIRE(payload.report.warnings.size() == 1);
    REQUIRE(payload.report.warnings[0].code == IgnoringWarning);
    
    REQUIRE(payload.node.headers.size() == 1);
    REQUIRE(payload.node.body.empty());
}
