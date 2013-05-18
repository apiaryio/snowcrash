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

using namespace snowcrash;

TEST_CASE("mparser/parse", "Parse method")
{
    SourceData source = "012345678";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "p1", 0, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Response 200", 0, MakeSourceDataBlock(2, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Code", 0, MakeSourceDataBlock(4, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(7, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(8, 1)));
    
    Method method;
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), method);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());

    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 13);
    
    REQUIRE(method.method == "GET");
    REQUIRE(method.description == "1");
    REQUIRE(method.requests.empty());
    REQUIRE(method.headers.empty());
    REQUIRE(method.parameters.empty());
    REQUIRE(method.responses.size() == 1);
    REQUIRE(method.responses.front().name == "200");
    REQUIRE(method.responses.front().description.empty());
    REQUIRE(method.responses.front().body == "Code");
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
    
    SourceData source = "01234";
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
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), method);
    
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
    
    SourceData source = "012345";
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
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), method);
    
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
    
    SourceData source = "012";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 1, MakeSourceDataBlock(0, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Response 200\n  B\n", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));

    Method method;
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), method);
    
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
    REQUIRE(method.responses.front().description == "B");
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
    
    
    SourceData source = "0123456789ABCDEFGHIJKLMNOPQRST"; 
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
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), method);
    
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
    
    SourceData source = "012345";
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
    ParseSectionResult result = MethodParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), method);
    
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
