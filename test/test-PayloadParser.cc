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
"          Hello World!\n";

const mdp::ByteBuffer ResponseBodyFixture = \
"+ Response 200 (text/plain)\n\n"\
"          Hello World!\n";

TEST_CASE("recognize request signature", "[payload]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(RequestFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<Payload>::sectionType(markdownAST.children().begin()) == RequestSectionType);
}

TEST_CASE("recognize abbreviated request signature", "[payload]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(RequestBodyFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<Payload>::sectionType(markdownAST.children().begin()) == RequestBodySectionType);
}

TEST_CASE("recognize abbreviated response signature", "[payload]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(ResponseBodyFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<Payload>::sectionType(markdownAST.children().begin()) == ResponseBodySectionType);
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
    REQUIRE(payload.body == "  Hello World!\n");
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
    REQUIRE(payload.body == "  Hello World!\nB\n");
    REQUIRE(payload.schema.empty());
}

//TEST_CASE("Parse payload description with list", "[payload][block][#8]")
//{
//    // Blueprint in question:
//    //R"(
//    //+ Request
//    //  + B
//    //  + Body
//    //");
//
//    Payload payload;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
//
//    REQUIRE(result.first.error.code == Error::OK);
//
//    REQUIRE(payload.name.empty());
//    REQUIRE(payload.description == "1234");
//    REQUIRE(payload.body.empty());
//}
//
//TEST_CASE("Parse just one payload in a list with multiple payloads", "[payload][block]")
//{
//    // Blueprint in question:
//    //R"(
//    //+ Request A
//    //+ Request B
//    //)";
//
//    Payload payload;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
//
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.empty());
//
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
//
//    REQUIRE(payload.name == "A");
//    REQUIRE(payload.description.empty());
//    REQUIRE(payload.parameters.empty());
//    REQUIRE(payload.headers.empty());
//    REQUIRE(payload.body.empty());
//    REQUIRE(payload.schema.empty());
//}
//
//TEST_CASE("Parse just one payload in a list with multiple items", "[payload][block]")
//{
//    // Blueprint in question:
//    //R"(
//    //+ Request A
//    //+ Foo
//    //)";
//
//    Payload payload;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
//
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.empty());
//
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
//
//    REQUIRE(payload.name == "A");
//    REQUIRE(payload.description.empty());
//    REQUIRE(payload.parameters.empty());
//    REQUIRE(payload.headers.empty());
//    REQUIRE(payload.body.empty());
//    REQUIRE(payload.schema.empty());
//}
//
//TEST_CASE("Parse payload with foreign list item", "[payload][foreign][block]")
//{
//    // Blueprint in question:
//    //R"(
//    //# /1
//    //## MKCOL
//    //+ Request
//    //  + Body
//    //
//    //              Foo
//    //
//    //  + Bar
//    //");
//
//    Payload payload;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
//
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.size() == 1);
//    REQUIRE(result.first.warnings[0].code == IgnoringWarning);
//
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 14);
//    REQUIRE(payload.name.empty());
//    REQUIRE(payload.description.empty());
//    REQUIRE(payload.parameters.empty());
//    REQUIRE(payload.headers.empty());
//    REQUIRE(payload.body == "Foo");
//    REQUIRE(payload.schema.empty());
//}

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
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == IndentationWarning);

    REQUIRE(payload.name.empty());
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Foo\n");
    REQUIRE(payload.schema.empty());
}

//TEST_CASE("Parse inline payload with symbol reference", "[payload][block]")
//{
//    // Blueprint in question:
//    //R"(
//    //+ Request
//    //     [Symbol][]
//    //");
//
//    Payload payload;
//
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    ResourceModel model;
//    model.name = "Symbol";
//    model.description = "Foo";
//    model.body = "Bar";
//    parser.symbolTable.resourceModels[model.name] = model;
//
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
//
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.empty());
//
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 4);
//
//    REQUIRE(payload.name.empty());
//    REQUIRE(payload.description == "Foo");
//    REQUIRE(payload.parameters.empty());
//    REQUIRE(payload.headers.empty());
//    REQUIRE(payload.body == "Bar");
//    REQUIRE(payload.schema.empty());
//}
//
//TEST_CASE("Parse payload with symbol reference", "[payload][block]")
//{
//    // Blueprint in question:
//    //R"(
//    //+ Request
//    //
//    //     [Symbol][]
//    //
//    //     Foreign
//    //");
//
//    Payload payload;
//
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    ResourceModel model;
//    model.name = "Symbol";
//    model.description = "Foo";
//    model.body = "Bar";
//    parser.symbolTable.resourceModels[model.name] = model;
//
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
//
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.size() == 1); // ignoring foreign entry
//
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 7);
//
//    REQUIRE(payload.name == "A");
//    REQUIRE(payload.description == "Foo");
//    REQUIRE(payload.parameters.empty());
//    REQUIRE(payload.headers.empty());
//    REQUIRE(payload.body == "Bar");
//    REQUIRE(payload.schema.empty());
//}
//
//TEST_CASE("Missing 'expected pre-formatted code block' warning source map", "[payload][issue][#2][block]")
//{
//    // Blueprint in question:
//    //R"(
//    //# GET /res
//    //
//    //+ Response 200 (text/plain)
//    //    + Body
//    //            something
//    //");
//
//    Payload payload;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
//
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.size() == 1); // ignoring unrecognized item
//
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 8);
//
//    REQUIRE(result.first.warnings[0].location.size() == 1);
//}
//
//TEST_CASE("Test deprecated object payload", "[payload][object][block]")
//{
//    // Blueprint in question:
//    //R"(
//    //# /message
//    //    + Message Object
//    //
//    //    AAA
//    //
//    //");
//
//    Payload payload;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
//
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.size() == 1);
//    REQUIRE(result.first.warnings[0].code == DeprecatedWarning);
//
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
//    REQUIRE(payload.name == "Message");
//    REQUIRE(payload.body == "AAA");
//}
//
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

