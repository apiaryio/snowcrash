//
//  test-MethodParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/6/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "MethodParser.h"
#include "ResourceParser.h"
#include "Fixture.h"

using namespace snowcrash;
using namespace snowcrashtest;

MarkdownBlock::Stack snowcrashtest::CanonicalMethodFixture()
{
    // Blueprint in question:
    //R"(
    //# GET
    //Method Description
    //
    // <see CanonicalPayloadFixture()>
    //
    //)";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Method Description", 0, MakeSourceDataBlock(1, 1)));
    
    MarkdownBlock::Stack payload = CanonicalPayloadFixture();
    markdown.insert(markdown.end(), payload.begin(), payload.end());

    return markdown;
}

TEST_CASE("mparser/classifier", "Method block classifier")
{
    MarkdownBlock::Stack markdown;

    markdown.push_back(MarkdownBlock(HeaderBlockType, "PUT", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "A", 0, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request B", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "C", 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "D", 0, MakeSourceDataBlock(5, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(7, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(8, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request E", 0, MakeSourceDataBlock(9, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "F", 0, MakeSourceDataBlock(10, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(11, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Foreign", 0, MakeSourceDataBlock(12, 1)));    
    
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(13, 1)));
    
    BlockIterator cur = markdown.begin();
    // HeaderBlockType - "PUT"
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == MethodSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == UndefinedSection);
    
    ++cur; // ParagraphBlockType - "A"
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == MethodSection);
    
    ++cur; // ListBlockBeginType - "Request B"
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == RequestSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == RequestSection);
    
    ++cur; // ParagraphBlockType - "Request E"
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == RequestSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == RequestSection);
    
    cur = markdown.begin();
    std::advance(cur, 13);  // ListItemBlockBeginType - "Request E"
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == RequestSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == RequestSection);
    
    std::advance(cur, 4);  // ListItemBlockBeginType - "Foreign"
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == ForeignSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == ForeignSection);
}

TEST_CASE("mparser/classifier-implicit-termination", "Method block classifier implicit termination")
{
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "PUT", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "A", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/1", 1, MakeSourceDataBlock(2, 1)));
    
    BlockIterator cur = markdown.begin();
    std::advance(cur, 2);
    
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == UndefinedSection);    
}

TEST_CASE("mparser/parse", "Parse method")
{
    MarkdownBlock::Stack markdown = CanonicalMethodFixture();   
    Method method;
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), SourceDataFixture, Blueprint(), method);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());

    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 22);
    
    REQUIRE(method.method == "GET");
    REQUIRE(method.description == "1");
    REQUIRE(method.responses.empty());
    REQUIRE(method.headers.empty());
    REQUIRE(method.parameters.empty());
    REQUIRE(method.requests.size() == 1);
}

TEST_CASE("mparser/parse-list-description", "Parse description with list")
{
    // Blueprint in question:
    //R"(
    //# GET
    //+ A
    //+ B
    //+ Request
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(0, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "A", 0, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "B", 0, MakeSourceDataBlock(2, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request", 0, MakeSourceDataBlock(3, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
    
    Method method;
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), SourceDataFixture, Blueprint(), method);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1); // warn skipping Request list
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 9);
    
    REQUIRE(method.method == "GET");
    REQUIRE(method.description == "4");
    REQUIRE(method.requests.empty());
    REQUIRE(method.headers.empty());
    REQUIRE(method.parameters.empty());
    REQUIRE(method.responses.empty());
}

TEST_CASE("mparser/parse-list-description-request", "Parse description with list followed by a request")
{
    // Blueprint in question:
    //R"(
    //# GET
    //+ A
    //
    // B
    //+ Request
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(0, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "A", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "B", 0, MakeSourceDataBlock(3, 1)));

    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    
    
    Method method;
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), SourceDataFixture, Blueprint(), method);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 10);
    
    REQUIRE(method.method == "GET");
    REQUIRE(method.description == "23");
    REQUIRE(method.headers.empty());
    REQUIRE(method.parameters.empty());
    REQUIRE(method.responses.empty());
    REQUIRE(method.requests.size() == 1);
    REQUIRE(method.requests.front().name.empty());
    REQUIRE(method.requests.front().description.empty());
    REQUIRE(method.requests.front().body.empty());
}

TEST_CASE("mparser/response-regex-problem", "Parse method with response not matching regex")
{
    // Specific test case aimed at posix regex problem of multiline Response string matching
    // Blueprint in question:
    //R"(
    //# GET
    //+ Response 200
    //  X
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(0, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Response 200\n  B\n", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));

    Method method;
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), SourceDataFixture, Blueprint(), method);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 5);
    
    REQUIRE(method.method == "GET");
    REQUIRE(method.description.empty());
    REQUIRE(method.headers.empty());
    REQUIRE(method.parameters.empty());
    REQUIRE(method.requests.empty());
    REQUIRE(method.responses.size() == 1);
    REQUIRE(method.responses.front().name == "200");
    REQUIRE(method.responses.front().description == "  B\n");
    REQUIRE(method.responses.front().body.empty());
}

TEST_CASE("mparser/parse-multi-request-response", "Parse method with multiple requests and responses")
{
    
    // Blueprint in question:
    //R"(
    //# PUT
    //+ Request A
    //  B
    //  + Body
    //
    //            C
    //
    //+ Request D
    //  E
    //  + Body
    //
    //            F
    //
    //+ Response 200
    //  G
    //  + Body
    //
    //            H
    //
    //+ Response 200
    //  I
    //  + Body
    //
    //            J
    //");
    

    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "PUT", 1, MakeSourceDataBlock(0, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));

    // Request A
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request A", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "B", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "C", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(7, 1)));

    // Request D
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request D", 0, MakeSourceDataBlock(8, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "E", 0, MakeSourceDataBlock(9, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(10, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "F", 0, MakeSourceDataBlock(11, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(12, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(13, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(14, 1)));
    
    // Response 200 #1
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Response 200", 0, MakeSourceDataBlock(15, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "G", 0, MakeSourceDataBlock(16, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(17, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "H", 0, MakeSourceDataBlock(18, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(19, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(20, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(21, 1)));
    
    // Response 200 #2
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Response 200", 0, MakeSourceDataBlock(22, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "I", 0, MakeSourceDataBlock(23, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(24, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "J", 0, MakeSourceDataBlock(25, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(26, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(27, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(28, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(29, 1)));
    
    Method method;
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), SourceDataFixture, Blueprint(), method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // warn responses with the same name
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 43);
    
    REQUIRE(method.method == "PUT");
    REQUIRE(method.description.empty());
    REQUIRE(method.headers.empty());
    REQUIRE(method.parameters.empty());
    
    REQUIRE(method.requests.size() == 2);
    
    REQUIRE(method.requests[0].name == "A");
    REQUIRE(method.requests[0].description == "2");
    REQUIRE(method.requests[0].body == "C");
    REQUIRE(method.requests[0].schema.empty());
    REQUIRE(method.requests[0].parameters.empty());
    REQUIRE(method.requests[0].headers.empty());
    
    REQUIRE(method.requests[1].name == "D");
    REQUIRE(method.requests[1].description == "9");
    REQUIRE(method.requests[1].body == "F");
    REQUIRE(method.requests[1].schema.empty());
    REQUIRE(method.requests[1].parameters.empty());
    REQUIRE(method.requests[1].headers.empty());
    
    REQUIRE(method.responses.size() == 2);
    
    REQUIRE(method.responses[0].name == "200");
    REQUIRE(method.responses[0].description == "G");
    REQUIRE(method.responses[0].body == "H");
    REQUIRE(method.responses[0].schema.empty());
    REQUIRE(method.responses[0].parameters.empty());
    REQUIRE(method.responses[0].headers.empty());
    
    REQUIRE(method.responses[1].name == "200");
    REQUIRE(method.responses[1].description == "N");
    REQUIRE(method.responses[1].body == "J");
    REQUIRE(method.responses[1].schema.empty());
    REQUIRE(method.responses[1].parameters.empty());
    REQUIRE(method.responses[1].headers.empty());

}

TEST_CASE("mparser/parse-multi-request-incomplete", "Parse method with multiple incomplete requests")
{
    // Blueprint in question:
    //R"(
    //# /1
    //# HEAD
    //+ Request A
    //+ Request B
    //  C
    //");

    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "HEAD", 1, MakeSourceDataBlock(0, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    // Request A
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request A", 0, MakeSourceDataBlock(1, 1)));

    // Request B
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request B", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "C", 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));

    Method method;
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), SourceDataFixture, Blueprint(), method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 9);
    
    REQUIRE(method.method == "HEAD");
    REQUIRE(method.description.empty());
    REQUIRE(method.headers.empty());
    REQUIRE(method.parameters.empty());
    
    REQUIRE(method.requests.size() == 2);
    REQUIRE(method.requests[0].name == "A");
    REQUIRE(method.requests[0].body.empty());
    REQUIRE(method.requests[0].schema.empty());
    REQUIRE(method.requests[0].parameters.empty());
    REQUIRE(method.requests[0].headers.empty());
    
    REQUIRE(method.requests[1].name == "B");
    REQUIRE(method.requests[1].description == "3");
    REQUIRE(method.requests[1].body.empty());
    REQUIRE(method.requests[1].schema.empty());
    REQUIRE(method.requests[1].parameters.empty());
    REQUIRE(method.requests[1].headers.empty());
}

TEST_CASE("mparser/parse-foreign", "Parse method with foreign item")
{
    // Blueprint in question:
    //R"(
    //# MKCOL
    //+ Request
    //  + Body
    //
    //              Foo
    //
    //+ Bar
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "MKCOL", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Foo", 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Bar", 0, MakeSourceDataBlock(7, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(8, 1)));

    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(9, 1)));
    
    Method method;
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), SourceDataFixture, Blueprint(), method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // ignoring unrecognized item
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 15);
    
    REQUIRE(method.method == "MKCOL");
    REQUIRE(method.description.empty());
    REQUIRE(method.headers.empty());
    REQUIRE(method.parameters.empty());
    
    REQUIRE(method.requests.size() == 1);
    REQUIRE(method.requests[0].name.empty());
    REQUIRE(method.requests[0].body == "Foo");
    REQUIRE(method.requests[0].schema.empty());
    REQUIRE(method.requests[0].parameters.empty());
    REQUIRE(method.requests[0].headers.empty());    
}

TEST_CASE("mparser/parse-inline-method-payload", "Parse method with inline payload")
{
    // Blueprint in question:
    //R"(
    //# POST
    //+ request
    //  + body
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "POST", 1, MakeSourceDataBlock(0, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "body", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "request", 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
    
    Method method;
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), SourceDataFixture, Blueprint(), method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // empty asset
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 9);
        
    REQUIRE(method.method == "POST");
    REQUIRE(method.description.empty());
    REQUIRE(method.responses.empty());
    REQUIRE(method.requests.size() == 1);
    REQUIRE(method.requests[0].name.empty());
    REQUIRE(method.requests[0].description.empty());
    REQUIRE(method.requests[0].body.empty());
}

TEST_CASE("mparser/parse-terminator", "Parse method finalized by terminator")
{
    
    // Blueprint in question:
    //R"(
    //# /1
    //# PATCH
    //---
    //A

    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "PATCH", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HRuleBlockType, SourceData(), 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "A", 0, MakeSourceDataBlock(2, 1)));
    
    Method method;
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), SourceDataFixture, Blueprint(), method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 2);
    
    REQUIRE(method.method == "PATCH");
    REQUIRE(method.description.empty());
    REQUIRE(method.requests.empty());
    REQUIRE(method.responses.empty());
}

TEST_CASE("mparser/parse-implicit-termination", "Parse incomplete method followed by another resource")
{
    // Blueprint in question:
    //R"(
    //# /1
    //## GET
    //# /2
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 2, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/2", 1, MakeSourceDataBlock(1, 1)));
    
    Method method;
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), SourceDataFixture, Blueprint(), method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 1);
    
    REQUIRE(method.method == "GET");
    REQUIRE(method.description.empty());
}

