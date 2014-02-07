//
//  test-PayloadParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/7/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "PayloadParser.h"
#include "Fixture.h"

using namespace snowcrash;
using namespace snowcrashtest;

MarkdownBlock::Stack snowcrashtest::CanonicalPayloadFixture()
{
    // Blueprint in question:
    //R"(
    //+ Request Hello World (text/plain)
    //
    //  Description
    //
    //    + Headers
    //
    //            X-Header: 42
    //
    //    + Body
    //
    //            Code
    //
    //    + Schema
    //
    //            Code 2
    //
    //)";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request Hello World (text/plain)", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Description", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));

    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Headers", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "X-Header: 42", 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Code", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Schema", 0, MakeSourceDataBlock(6, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Code 2", 0, MakeSourceDataBlock(7, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(8, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(9, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(10, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(11, 1)));
    
    return markdown;
}

TEST_CASE("Payload block classifier", "[payload][block]")
{
    MarkdownBlock::Stack markdown = CanonicalPayloadFixture();
    
    CHECK(markdown.size() == 20);
    
    BlockIterator cur = markdown.begin();

    // ListBlockBeginType
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSectionType) == RequestSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSectionType) == RequestSectionType);

    ++cur; // ListItemBlockBeginType
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSectionType) == RequestSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSectionType) == UndefinedSectionType); 

    ++cur; // ParagraphBlockType
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSectionType) == RequestSectionType);

    std::advance(cur, 2); // ListBlockBeginType
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSectionType) == HeadersSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), HeadersSectionType) == HeadersSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), BodySectionType) == HeadersSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), SchemaSectionType) == HeadersSectionType); 

    ++cur; // ListItemBlockBeginType - Headers
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSectionType) == HeadersSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), HeadersSectionType) == HeadersSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), BodySectionType) == HeadersSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), SchemaSectionType) == HeadersSectionType);
    
    std::advance(cur, 4); // ListItemBlockBeginType - Body
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSectionType) == BodySectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), BodySectionType) == BodySectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), SchemaSectionType) == BodySectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), HeadersSectionType) == BodySectionType);
    
    std::advance(cur, 4); // ListItemBlockBeginType - Schema
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSectionType) == SchemaSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), SchemaSectionType) == SchemaSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), BodySectionType) == SchemaSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), HeadersSectionType) == SchemaSectionType);
    
    // Test object payload
    markdown[2].content = "My Resource Object (application/json)";
    cur = markdown.begin();
    
    // ListBlockBeginType
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSectionType) == ObjectSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), ObjectSectionType) == ObjectSectionType);
    
    ++cur; // ListItemBlockBeginType
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSectionType) == ObjectSectionType);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), ObjectSectionType) == UndefinedSectionType);
    
}

TEST_CASE("Parse canonical payload", "[payload][block]")
{
    MarkdownBlock::Stack markdown = CanonicalPayloadFixture();    
    Payload payload;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 20);

    REQUIRE(payload.name == "Hello World");
    REQUIRE(payload.description == "1");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.size() == 2);
    REQUIRE(payload.headers[0].first == "Content-Type");
    REQUIRE(payload.headers[0].second == "text/plain");
    REQUIRE(payload.headers[1].first == "X-Header");
    REQUIRE(payload.headers[1].second == "42");
    REQUIRE(payload.body == "Code");
    REQUIRE(payload.schema == "Code 2");
}

TEST_CASE("Parse payload description with list", "[payload][block][#8]")
{
    // Blueprint in question:
    //R"(
    //+ Request
    //  + B
    //  + Body
    //");
    
    MarkdownBlock::Stack markdown;

    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request", 0, MakeSourceDataBlock(0, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "B", 0, MakeSourceDataBlock(4, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Body", 0, MakeSourceDataBlock(3, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(1, 1)));
    
    Payload payload;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    
    REQUIRE(payload.name.empty());
    REQUIRE(payload.description == "1234");
    REQUIRE(payload.body.empty());
}

TEST_CASE("Parse just one payload in a list with multiple payloads", "[payload][block]")
{
    // Blueprint in question:
    //R"(
    //+ Request A
    //+ Request B
    //)";
    
    SourceData source = "012";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request A", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request B", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    
    Payload payload;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(payload.name == "A");
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body.empty());
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Parse just one payload in a list with multiple items", "[payload][block]")
{
    // Blueprint in question:
    //R"(
    //+ Request A
    //+ Foo
    //)";
    
    SourceData source = "012";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request A", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock())); 
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Foo", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    
    Payload payload;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(payload.name == "A");
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body.empty());
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Parse payload with foreign list item", "[payload][foreign][block]")
{
    // Blueprint in question:
    //R"(
    //# /1
    //## MKCOL
    //+ Request
    //  + Body
    //
    //              Foo
    //
    //  + Bar
    //");
    
    SourceData source = "0123456789ABCDEFGHIJKLMNOPQRST";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Foo", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Bar", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(7, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(8, 1)));
    
    Payload payload;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1);
    REQUIRE(result.first.warnings[0].code == IgnoringWarning);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 14);
    REQUIRE(payload.name.empty());
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Foo");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Parse payload with foreign block", "[payload][foreign][block]")
{
    // Blueprint in question:
    //R"(
    //# /1
    //## MKCOL
    //+ Request
    //  + Body
    //
    //              Foo
    //
    //  Bar
    //");
    
    SourceData source = "0123456789ABCDEFGHIJKLMNOPQRST";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Foo", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Bar", 0, MakeSourceDataBlock(5, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(7, 1)));
    
    Payload payload;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1); // dangling body
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 12);
    REQUIRE(payload.name.empty());
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Foo5");
    REQUIRE(payload.schema.empty());
}


TEST_CASE("Parse abbreviated payload body", "[payload][block]")
{
    // Blueprint in question:
    //R"(
    //+ Response 200 (text/plain)
    //
    //          Hello World!
    //)";
    
    MarkdownBlock::Stack markdown;
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Response 200 (text/plain)", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Hello World", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    Payload payload;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
    
    REQUIRE(payload.name == "200");
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.size() == 1);
    REQUIRE(payload.body == "Hello World");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Parse abbreviated inline payload body", "[payload][block]")
{
    // Blueprint in question:
    //R"(
    //+ Request A
    //  B
    //)";
    
    SourceData source = "01";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request A\n  B\n", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(1, 1)));
    
    Payload payload;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // preformatted code block
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 4);
    
    REQUIRE(payload.name == "A");
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "  B\n");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Parse inline payload with symbol reference", "[payload][block]")
{
    // Blueprint in question:
    //R"(
    //+ Request
    //     [Symbol][]
    //");
    
    MarkdownBlock::Stack markdown;
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request\n  [Symbol][]", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(1, 1)));
    
    Payload payload;
    
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ResourceModel model;
    model.name = "Symbol";
    model.description = "Foo";
    model.body = "Bar";
    parser.symbolTable.resourceModels[model.name] = model;
    
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 4);
    
    REQUIRE(payload.name.empty());
    REQUIRE(payload.description == "Foo");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Bar");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Parse payload with symbol reference", "[payload][block]")
{
    // Blueprint in question:
    //R"(
    //+ Request
    //
    //     [Symbol][]
    //
    //     Foreign
    //");
    
    MarkdownBlock::Stack markdown;
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request A", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "  [Symbol][]  ", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "  Foreign ", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    Payload payload;
    
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ResourceModel model;
    model.name = "Symbol";
    model.description = "Foo";
    model.body = "Bar";
    parser.symbolTable.resourceModels[model.name] = model;
    
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1); // ignoring foreign entry
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 7);
    
    REQUIRE(payload.name == "A");
    REQUIRE(payload.description == "Foo");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Bar");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Missing 'expected pre-formatted code block' warning source map", "[payload][issue][#2][block]")
{
    // Blueprint in question:
    //R"(
    //# GET /res
    //
    //+ Response 200 (text/plain)
    //    + Body        
    //            something
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));

    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Body        \n    something\n", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(1, 1)));

    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Response 200 (text/plain)\n", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    Payload payload;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1); // ignoring unrecognized item
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 8);
    
    REQUIRE(result.first.warnings[0].location.size() == 1);
}

TEST_CASE("Test deprecated object payload", "[payload][object][block]")
{
    // Blueprint in question:
    //R"(
    //# /message
    //    + Message Object
    //    
    //    AAA
    //
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));

    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Message Object", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "AAA", 0, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    Payload payload;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1);
    REQUIRE(result.first.warnings[0].code == DeprecatedWarning);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
    REQUIRE(payload.name == "Message");
    REQUIRE(payload.body == "AAA");
}

TEST_CASE("Parse named model", "[payload][model][block]")
{
    // Blueprint in question:
    //R"(
    //+ Super Model (text/plain)
    //
    //        Hello World!
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Super Model (text/plain)", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Hello World!", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    Payload payload;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
    
    REQUIRE(payload.name == "Super");
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.size() == 1);
    REQUIRE(payload.headers[0].first == "Content-Type");
    REQUIRE(payload.headers[0].second == "text/plain");
    REQUIRE(payload.body == "Hello World!");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Parse nameless model", "[payload][model][block]")
{
    // Blueprint in question:
    //R"(
    //+ Model (text/plain)
    //
    //        Hello World!
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Model (text/plain)", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Hello World!", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    Payload payload;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
    
    REQUIRE(payload.name.empty());
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.size() == 1);
    REQUIRE(payload.headers[0].first == "Content-Type");
    REQUIRE(payload.headers[0].second == "text/plain");
    REQUIRE(payload.body == "Hello World!");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("Warn on malformed payload signature", "[payload][block][#20]")
{
    // Blueprint in question:
    //R"(
    //+ Request This is FUN[ (text/plain)
    //
    //        Hello World!
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request This is FUN[ (text/plain)", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Hello World!", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    Payload payload;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1);
    REQUIRE(result.first.warnings[0].code == FormattingWarning);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
    
    REQUIRE(payload.name.empty());
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Hello World!");
    REQUIRE(payload.schema.empty());
}

