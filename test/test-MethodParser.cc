//
//  test-MethodParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/6/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "MethodParser.h"
#include "ResourceParser.h"
#include "ResourceGroupParser.h"
#include "Fixture.h"

using namespace snowcrash;
using namespace snowcrashtest;

MarkdownBlock::Stack snowcrashtest::CanonicalMethodFixture()
{
    // Blueprint in question:
    //R"(
    //# My Method [GET]
    //Method Description
    //
    //+ Headers
    //
    //        X-Method-Header: 0xdeadbeef
    //
    // <see CanonicalPayloadFixture()>
    //
    //+ Response 200 (text/plain)
    //
    //       OK.
    //
    //)";
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "My Method [GET]", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Method Description", 0, MakeSourceDataBlock(1, 1)));
    
    MarkdownBlock::Stack headerList;
    headerList.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    headerList.push_back(MarkdownBlock(ParagraphBlockType, "Headers", 0, MakeSourceDataBlock(1, 1)));
    headerList.push_back(MarkdownBlock(CodeBlockType, "X-Method-Header: 0xdeadbeef", 0, MakeSourceDataBlock(2, 1)));
    headerList.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    MarkdownBlock::Stack listBlock = CanonicalPayloadFixture();
    
    // inject headers into payload list
    MarkdownBlock::Stack::iterator cur = listBlock.begin();
    ++cur;
    listBlock.insert(cur, headerList.begin(), headerList.end());
    
    // inject response into payload list
    MarkdownBlock::Stack responseList;
    responseList.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    responseList.push_back(MarkdownBlock(ParagraphBlockType, "Response 200 (text/plain)", 0, MakeSourceDataBlock(4, 1)));
    responseList.push_back(MarkdownBlock(CodeBlockType, "OK.", 0, MakeSourceDataBlock(5, 1)));
    responseList.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    
    cur = listBlock.begin();
    ++cur;
    listBlock.insert(cur, responseList.begin(), responseList.end());
    
    // inject complete list into final markdown
    markdown.insert(markdown.end(), listBlock.begin(), listBlock.end());

    return markdown;
}

TEST_CASE("mparser/classifier", "Method block classifier")
{
    MarkdownBlock::Stack markdown = CanonicalMethodFixture();
    
    MarkdownBlock::Stack additionalListItems;
    additionalListItems.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    additionalListItems.push_back(MarkdownBlock(ParagraphBlockType, "Response 200", 0, MakeSourceDataBlock(0, 1)));
    additionalListItems.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(1, 1)));
    additionalListItems.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    additionalListItems.push_back(MarkdownBlock(ListItemBlockEndType, "Foreign", 0, MakeSourceDataBlock(2, 1)));
    
    MarkdownBlock::Stack::iterator cur = markdown.end();
    --cur;
    markdown.insert(cur, additionalListItems.begin(), additionalListItems.end());
    
    cur = markdown.begin();
    // HeaderBlockType - "GET"
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == MethodSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == UndefinedSection);
    
    ++cur; // ParagraphBlockType - "MethodSection"
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == MethodSection);

    ++cur; // ListBlockBeginType - "Response"
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == ResponseSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == ResponseSection);
    
    ++cur; // ListItemBlockBeginType - "Response"
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == ResponseSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == ResponseSection);
    
    std::advance(cur, 4); // ListItemBlockBeginType - "Headers"
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == HeadersSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == HeadersSection);
    
    std::advance(cur, 4); // Request
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == RequestSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == RequestSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), HeadersSection) == RequestSection);
    
    std::advance(cur, 18); // Response
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == ResponseSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == ResponseSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), ResponseSection) == ResponseSection);
    
    std::advance(cur, 3); // Foreign
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == ForeignSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == ForeignSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), HeadersSection) == ForeignSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), ResponseSection) == ForeignSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), RequestSection) == ForeignSection);
    
    // Nameless method
    markdown[0].content = "GET";
    cur = markdown.begin();
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == MethodSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == UndefinedSection);

    // Keyword "group"
    markdown[0].content = "Group A";
    cur = markdown.begin();
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
    REQUIRE(ClassifyBlock<Method>(cur, markdown.end(), MethodSection) == UndefinedSection);
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
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = MethodParser::Parse(markdown.begin(),
                                                    markdown.end(),
                                                    parser,
                                                    method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());

    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 30);
    
    REQUIRE(method.name == "My Method");
    REQUIRE(method.method == "GET");
    REQUIRE(method.description == "1");

    REQUIRE(method.headers.size() == 1);
    REQUIRE(method.headers[0].first == "X-Method-Header");
    REQUIRE(method.headers[0].second == "0xdeadbeef");
    REQUIRE(method.parameters.empty());
    REQUIRE(method.requests.size() == 1);
    REQUIRE(method.responses.size() == 1);
    
    REQUIRE(method.responses[0].name == "200");
    REQUIRE(method.responses[0].body == "OK.");
    REQUIRE(method.responses[0].headers.size() == 1);
    REQUIRE(method.responses[0].headers[0].first == "Content-Type");
    REQUIRE(method.responses[0].headers[0].second == "text/plain");
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
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "A", 0, MakeSourceDataBlock(4, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "B", 0, MakeSourceDataBlock(3, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request", 0, MakeSourceDataBlock(2, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(1, 1)));
    
    Method method;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), parser, method);
    
    REQUIRE(result.first.error.code == Error::OK);
    
    REQUIRE(method.description == "123");
    REQUIRE(method.responses.empty());
    REQUIRE(method.requests.size() == 1);
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
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());    
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), parser, method);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1); // empty body asset
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 10);
    
    REQUIRE(method.name.empty());    
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
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), parser, method);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1); // preformatted asset
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 5);
    
    REQUIRE(method.name.empty());    
    REQUIRE(method.method == "GET");
    REQUIRE(method.description.empty());
    REQUIRE(method.headers.empty());
    REQUIRE(method.parameters.empty());
    REQUIRE(method.requests.empty());
    REQUIRE(method.responses.size() == 1);
    REQUIRE(method.responses.front().name == "200");
    REQUIRE(method.responses.front().description.empty());
    REQUIRE(method.responses.front().body == "  B\n");
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
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), parser, method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // warn responses with the same name
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 43);
    
    REQUIRE(method.name.empty());    
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
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), parser, method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 2); // empty asset & preformatted asset
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 9);
    
    REQUIRE(method.name.empty());    
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
    REQUIRE(method.requests[1].description.empty());
    REQUIRE(method.requests[1].body == "3");
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
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), parser, method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // ignoring unrecognized item
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 15);
    
    REQUIRE(method.name.empty());    
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
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), parser, method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // empty asset
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 9);
    
    REQUIRE(method.name.empty());
    REQUIRE(method.method == "POST");
    REQUIRE(method.description.empty());
    REQUIRE(method.responses.empty());
    REQUIRE(method.requests.size() == 1);
    REQUIRE(method.requests[0].name.empty());
    REQUIRE(method.requests[0].description.empty());
    REQUIRE(method.requests[0].body.empty());
}

TEST_CASE("mparser/parse-hr", "Parse method with a HR")
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
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), parser, method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(method.name.empty());    
    REQUIRE(method.method == "PATCH");
    REQUIRE(method.description == "12");
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
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), parser, method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 1);
    
    REQUIRE(method.name.empty());    
    REQUIRE(method.method == "GET");
    REQUIRE(method.description.empty());
}

TEST_CASE("mparser/header-warnings", "Check warnings on overshadowing a header")
{
    MarkdownBlock::Stack markdown = CanonicalMethodFixture();
    Method method;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    method.headers.push_back(std::make_pair("X-Header", "24"));
    ParseSectionResult result = MethodParser::Parse(markdown.begin(),
                                                    markdown.end(),
                                                    parser,
                                                    method);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 30);
}

TEST_CASE("mparser/parse-nameless-method", "Parse method without name")
{
    // Blueprint in question:
    //R"(
    //# GET
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 2, MakeSourceDataBlock(0, 1)));
    
    Method method;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), parser, method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 1);
    
    REQUIRE(method.name.empty());
    REQUIRE(method.method == "GET");
    REQUIRE(method.description.empty());
}

TEST_CASE("mparser/not-parse-object", "Make sure method with object payload is not parsed")
{
    // Blueprint in question:
    //R"(
    //# GET
    //+ My Object
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(0, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "My Object", 0, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    
    Method method;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), parser, method);
    
    REQUIRE(result.first.error.code != Error::OK);
}

TEST_CASE("mparser/adjacent-group", "Make sure method followed by a group does not eat the group")
{
    // Blueprint in question:
    //R"(
    //## POST
    //# Group Two
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "POST", 2, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Group Two", 1, MakeSourceDataBlock(1, 1)));
    
    Method method;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), parser, method);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 1);

    REQUIRE(method.method == "POST");
    REQUIRE(method.description.empty());

}
