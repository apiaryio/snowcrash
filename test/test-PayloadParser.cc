//
//  test-PayloadParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/7/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "PayloadParser.h"

using namespace snowcrash;

MarkdownBlock::Stack CanonicalPayloadFixture()
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

static SourceData SourceDataFixture = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

TEST_CASE("pldparser/classifier", "Payload block classifier")
{
    MarkdownBlock::Stack markdown = CanonicalPayloadFixture();
    
    CHECK(markdown.size() == 20);
    
    BlockIterator cur = markdown.begin();

    // ListBlockBeginType
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSection) == RequestSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSection) == RequestSection);

    ++cur; // ListItemBlockBeginType
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSection) == RequestSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSection) == UndefinedSection); 

    ++cur; // ParagraphBlockType
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSection) == RequestSection);

    std::advance(cur, 2); // ListBlockBeginType
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSection) == HeadersSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), HeadersSection) == HeadersSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), BodySection) == HeadersSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), SchemaSection) == HeadersSection); 

    ++cur; // ListItemBlockBeginType - Headers
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSection) == HeadersSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), HeadersSection) == HeadersSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), BodySection) == HeadersSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), SchemaSection) == HeadersSection);
    
    std::advance(cur, 4); // ListItemBlockBeginType - Body
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSection) == BodySection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), BodySection) == BodySection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), SchemaSection) == BodySection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), HeadersSection) == BodySection);
    
    std::advance(cur, 4); // ListItemBlockBeginType - Schema
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), RequestSection) == SchemaSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), SchemaSection) == SchemaSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), BodySection) == SchemaSection);
    REQUIRE(ClassifyBlock<Payload>(cur, markdown.end(), HeadersSection) == SchemaSection);
}

TEST_CASE("pldparser/parse", "Parse canonical payload")
{
    MarkdownBlock::Stack markdown = CanonicalPayloadFixture();    
    Payload payload;
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), SourceDataFixture, Blueprint(), payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 20);

    REQUIRE(payload.name == "Hello World");
    REQUIRE(payload.description == "1");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.size() == 1);
    REQUIRE(payload.headers[0].first == "X-Header");
    REQUIRE(payload.headers[0].second == "42");
    REQUIRE(payload.body == "Code");
    REQUIRE(payload.schema == "Code 2");
}

TEST_CASE("pldparser/parse-incomplete", "Parse incomplete payload")
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
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());

    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 4);
    
    REQUIRE(payload.name == "A");
    REQUIRE(payload.description == "  B\n");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body.empty());
    REQUIRE(payload.schema.empty());
}

TEST_CASE("pldparser/parse-list-description", "Parse description with list")
{
    // Blueprint in question:
    //R"(
    //+ Request
    //  + B
    //  + Body
    //");
    
    SourceData source = "01234";
    MarkdownBlock::Stack markdown;

    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request", 0, MakeSourceDataBlock(0, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "B", 0, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Body", 0, MakeSourceDataBlock(2, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
    
    Payload payload;
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // warn skipping body list
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 10);
    
    REQUIRE(payload.name.empty());
    REQUIRE(payload.description == "3");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body.empty());
    REQUIRE(payload.schema.empty());
}

TEST_CASE("pldparser/parse-one", "Parse just one payload in a list with multiple payloads")
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
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(payload.name == "A");
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body.empty());
    REQUIRE(payload.schema.empty());
}

TEST_CASE("pldparser/parse-one-foreign", "Parse just one payload in a list with multiple items")
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
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(payload.name == "A");
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body.empty());
    REQUIRE(payload.schema.empty());
}

TEST_CASE("pldparser/parse-payload-foreign-listitem", "Parse payload with foreign list item")
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
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // ignoring unrecognized item
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 14);
    REQUIRE(payload.name.empty());
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Foo");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("pldparser/parse-payload-foreign-block", "Parse payload with foreign block")
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
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // ignoring unrecognized item
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 12);
    REQUIRE(payload.name.empty());
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Foo");
    REQUIRE(payload.schema.empty());
}
