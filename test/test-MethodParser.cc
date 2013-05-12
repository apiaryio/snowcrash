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

//# /1
//# GET
//+ Response 200
//X
//block 5, content: '/1'
//block 5, content: 'GET'
//block 7
//block 9
//block 10, content: 'Response 200
//X
//'
//block 8

