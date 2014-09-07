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
    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(RequestFixture, RequestSectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    CHECK(report.warnings.empty());

    REQUIRE(payload.name == "Hello World");
    REQUIRE(payload.description == "Description\n");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.size() == 2);
    REQUIRE(payload.headers[0].first == "Content-Type");
    REQUIRE(payload.headers[0].second == "text/plain");
    REQUIRE(payload.headers[1].first == "X-Header");
    REQUIRE(payload.headers[1].second == "42");
    REQUIRE(payload.body == "Code\n");
    REQUIRE(payload.schema == "Code 2\n");
}

TEST_CASE("Parse abbreviated payload body", "[payload]")
{
    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(ResponseBodyFixture, ResponseBodySectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    CHECK(report.warnings.empty());

    REQUIRE(payload.name == "200");
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.size() == 1);
    REQUIRE(payload.body == "Hello World!\n");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Parse abbreviated inline payload body", "[payload]")
{
    mdp::ByteBuffer source = RequestBodyFixture;
    source += "  B\n";

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestBodySectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    CHECK(report.warnings.size() == 1); // preformatted code block

    REQUIRE(payload.name == "A");
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Hello World!\nB\n");
    REQUIRE(payload.schema.empty());
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

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    CHECK(report.warnings.empty());

    REQUIRE(payload.name.empty());
    REQUIRE(payload.description == "+ B\n");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "{}\n");
    REQUIRE(payload.schema.empty());
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

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    CHECK(report.warnings.size() == 1); // dangling block

    REQUIRE(payload.name.empty());
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "{}\n");
    REQUIRE(payload.schema.empty());
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

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // dangling block
    REQUIRE(report.warnings[0].code == IndentationWarning);

    REQUIRE(payload.name.empty());
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Foo\nBar\n\n");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Parse inline payload with symbol reference", "[payload]")
{
    ResourceModel model;
    Symbols symbols;

    model.description = "Foo";
    model.body = "Bar";
    symbols.push_back(ResourceModelSymbol("Symbol", model));

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(SymbolFixture, RequestBodySectionType, report, payload, symbols);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 0);

    REQUIRE(payload.name.empty());
    REQUIRE(payload.description == "Foo");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Bar");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Parse inline payload with symbol reference with extra indentation", "[payload]")
{
    ResourceModel model;
    Symbols symbols;

    model.description = "Foo";
    model.body = "Bar";
    symbols.push_back(ResourceModelSymbol("Symbol", model));

    mdp::ByteBuffer source = \
    "+ Request\n\n"\
    "        [Symbol][]\n";

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestBodySectionType, report, payload, symbols);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 0);

    REQUIRE(payload.name.empty());
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "[Symbol][]\n");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Parse inline payload with symbol reference with foreign content", "[payload]")
{
    mdp::ByteBuffer source = SymbolFixture;
    source += "\n    Foreign\n";

    ResourceModel model;
    Symbols symbols;

    model.description = "Foo";
    model.body = "Bar";
    symbols.push_back(ResourceModelSymbol("Symbol", model));

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestBodySectionType, report, payload, symbols);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // ignoring foreign entry

    REQUIRE(payload.name.empty());
    REQUIRE(payload.description == "Foo");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Bar");
    REQUIRE(payload.schema.empty());
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

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, ModelBodySectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(payload.name == "Super");
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.size() == 1);
    REQUIRE(payload.headers[0].first == "Content-Type");
    REQUIRE(payload.headers[0].second == "text/plain");
    REQUIRE(payload.body == "Hello World!\n");
    REQUIRE(payload.schema.empty());
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

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, ModelBodySectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(payload.name.empty());
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.size() == 1);
    REQUIRE(payload.headers[0].first == "Content-Type");
    REQUIRE(payload.headers[0].second == "text/plain");
    REQUIRE(payload.body == "Hello World!\n");
    REQUIRE(payload.schema.empty());
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

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == FormattingWarning);

    REQUIRE(payload.name.empty());
    REQUIRE(payload.description == "Description\n\nLine 2\n");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Hello World!\n");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Give a warning of empty message body for requests with certain headers", "[payload]")
{
    mdp::ByteBuffer source = \
    "+ Request\n"\
    "    + Headers \n"\
    "\n"\
    "            Content-Length: 100\n";

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(payload.headers.size() == 1);
    REQUIRE(payload.headers[0].first == "Content-Length");
    REQUIRE(payload.headers[0].second == "100");
    REQUIRE(payload.body.empty());
}

TEST_CASE("Do not report empty message body for requests", "[payload]")
{
    mdp::ByteBuffer source = \
    "+ Request\n"\
    "    + Headers \n"\
    "\n"\
    "            Accept: application/json, application/javascript\n";

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(payload.headers.size() == 1);
    REQUIRE(payload.headers[0].first == "Accept");
    REQUIRE(payload.headers[0].second == "application/json, application/javascript");
    REQUIRE(payload.body.empty());
}

TEST_CASE("Give a warning when 100 response has a body", "[payload]")
{
    mdp::ByteBuffer source = \
    "+ Response 100\n\n"\
    "        {}\n";

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, ResponseBodySectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(payload.body == "{}\n");
}

TEST_CASE("Empty body section should shouldn't be parsed as description", "[payload]")
{
    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(EmptyBodyFixture, ResponseSectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(payload.body == "");
}

TEST_CASE("Parameters section should be taken as a description node", "[payload]")
{
    mdp::ByteBuffer source = \
    "+ Response 200\n\n"\
    "    + Parameters\n\n"\
    "        + id (string)\n\n"\
    "    + Body\n\n"\
    "            {}\n";

    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, ResponseSectionType, report, payload);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(payload.description == "+ Parameters\n\n    + id (string)\n");
    REQUIRE(payload.body == "{}\n");
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
    
    Payload payload;
    Report report;
    SectionParserHelper<Payload, PayloadParser>::parse(source, RequestSectionType, report, payload);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IgnoringWarning);
    
    REQUIRE(payload.headers.size() == 1);
    REQUIRE(payload.body.empty());
}
