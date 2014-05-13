//
//  test-ActionParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/6/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "ActionParser.h"
#include "ResourceParser.h"
#include "ResourceGroupParser.h"
#include "Fixture.h"
#include "Parser.h"

using namespace snowcrash;
using namespace snowcrashtest;

MarkdownBlock::Stack snowcrashtest::CanonicalActionFixture()
{
    // Blueprint in question:
    //R"(
    //# My Method [GET]
    //Method Description
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
    
    MarkdownBlock::Stack listBlock = CanonicalPayloadFixture();
    
    // inject response into payload list
    MarkdownBlock::Stack responseList;
    responseList.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    responseList.push_back(MarkdownBlock(ParagraphBlockType, "Response 200 (text/plain)", 0, MakeSourceDataBlock(4, 1)));
    responseList.push_back(MarkdownBlock(CodeBlockType, "OK.", 0, MakeSourceDataBlock(5, 1)));
    responseList.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    
    MarkdownBlock::Stack::iterator cur = listBlock.end();
    --cur;
    listBlock.insert(cur, responseList.begin(), responseList.end());
    
    // inject complete list into final markdown
    markdown.insert(markdown.end(), listBlock.begin(), listBlock.end());
    
    return markdown;
}

TEST_CASE("Method block classifier", "[action][classifier][blocks]")
{
    MarkdownBlock::Stack markdown = CanonicalActionFixture();
    
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
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), UndefinedSectionType) == ActionSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), ActionSectionType) == UndefinedSectionType);
    
    ++cur; // ParagraphBlockType - "ActionSectionType"
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), ActionSectionType) == ActionSectionType);

    ++cur; // ListBlockBeginType
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), UndefinedSectionType) == RequestSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), ActionSectionType) == RequestSectionType);
    
    ++cur; // ListItemBlockBeginType
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), UndefinedSectionType) == RequestSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), ActionSectionType) == RequestSectionType);
    
    std::advance(cur, 18); // Canonnical response
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), UndefinedSectionType) == ResponseSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), ActionSectionType) == ResponseSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), HeadersSectionType) == ResponseSectionType);
    
    std::advance(cur, 4); // Additional Response
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), UndefinedSectionType) == ResponseSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), ActionSectionType) == ResponseSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), ResponseSectionType) == ResponseSectionType);
    
    std::advance(cur, 3); // Foreign
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), UndefinedSectionType) == ForeignSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), ActionSectionType) == ForeignSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), HeadersSectionType) == ForeignSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), ResponseSectionType) == ForeignSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), RequestSectionType) == ForeignSectionType);
    
    // Nameless method
    markdown[0].content = "GET";
    cur = markdown.begin();
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), UndefinedSectionType) == ActionSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), ActionSectionType) == UndefinedSectionType);

    // Keyword "group"
    markdown[0].content = "Group A";
    cur = markdown.begin();
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), ActionSectionType) == UndefinedSectionType);
}

TEST_CASE("Method block classifier implicit termination", "[action][blocks]")
{
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "PUT", 1, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "A", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/1", 1, MakeSourceDataBlock(2, 1)));
    
    BlockIterator cur = markdown.begin();
    std::advance(cur, 2);
    
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), ActionSectionType) == UndefinedSectionType);
    REQUIRE(ClassifyBlock<Action>(cur, markdown.end(), UndefinedSectionType) == UndefinedSectionType);    
}

TEST_CASE("Parse method", "[action][blocks]")
{
    MarkdownBlock::Stack markdown = CanonicalActionFixture();   
    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(),
                                                    markdown.end(),
                                                    rootSection,
                                                    parser,
                                                    action);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());

    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 26);
    
    REQUIRE(action.name == "My Method");
    REQUIRE(action.method == "GET");
    REQUIRE(action.description == "1");

    REQUIRE(action.headers.size() == 0);
    REQUIRE(action.examples.front().requests.size() == 1);
    REQUIRE(action.examples.front().responses.size() == 1);
    
    REQUIRE(action.examples.front().responses[0].name == "200");
    REQUIRE(action.examples.front().responses[0].body == "OK.");
    REQUIRE(action.examples.front().responses[0].headers.size() == 1);
    REQUIRE(action.examples.front().responses[0].headers[0].first == "Content-Type");
    REQUIRE(action.examples.front().responses[0].headers[0].second == "text/plain");
}

TEST_CASE("Parse Action description with list", "[action][blocks]")
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
    
    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
    
    REQUIRE(result.first.error.code == Error::OK);
    
    REQUIRE(action.description == "123");
    REQUIRE(action.examples.front().responses.empty());
    REQUIRE(action.examples.front().requests.size() == 1);
}

TEST_CASE("Parse description with list followed by a request", "[action][blocks]")
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
    
    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1); // empty body asset
    REQUIRE(result.first.warnings[0].code == EmptyDefinitionWarning);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 10);
    
    REQUIRE(action.name.empty());    
    REQUIRE(action.method == "GET");
    REQUIRE(action.description == "23");
    REQUIRE(action.headers.empty());
    REQUIRE(action.parameters.empty());
    REQUIRE(action.examples.front().responses.empty());
    REQUIRE(action.examples.front().requests.size() == 1);
    REQUIRE(action.examples.front().requests.front().name.empty());
    REQUIRE(action.examples.front().requests.front().description.empty());
    REQUIRE(action.examples.front().requests.front().body.empty());
}

TEST_CASE("Parse method with response not matching regex", "[action][blocks]")
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

    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1); // preformatted asset
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 5);
    
    REQUIRE(action.name.empty());    
    REQUIRE(action.method == "GET");
    REQUIRE(action.description.empty());
    REQUIRE(action.headers.empty());
    REQUIRE(action.parameters.empty());
    REQUIRE(action.examples.front().requests.empty());
    REQUIRE(action.examples.front().responses.size() == 1);
    REQUIRE(action.examples.front().responses.front().name == "200");
    REQUIRE(action.examples.front().responses.front().description.empty());
    REQUIRE(action.examples.front().responses.front().body == "  B\n");
}

TEST_CASE("Parse method with multiple requests and responses", "[action][blocks]")
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
    
    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // warn responses with the same name
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 43);
    
    REQUIRE(action.name.empty());    
    REQUIRE(action.method == "PUT");
    REQUIRE(action.description.empty());
    REQUIRE(action.headers.empty());
    REQUIRE(action.parameters.empty());
    
    REQUIRE(action.examples.front().requests.size() == 2);
    
    REQUIRE(action.examples.front().requests[0].name == "A");
    REQUIRE(action.examples.front().requests[0].description == "2");
    REQUIRE(action.examples.front().requests[0].body == "C");
    REQUIRE(action.examples.front().requests[0].schema.empty());
    REQUIRE(action.examples.front().requests[0].parameters.empty());
    REQUIRE(action.examples.front().requests[0].headers.empty());
    
    REQUIRE(action.examples.front().requests[1].name == "D");
    REQUIRE(action.examples.front().requests[1].description == "9");
    REQUIRE(action.examples.front().requests[1].body == "F");
    REQUIRE(action.examples.front().requests[1].schema.empty());
    REQUIRE(action.examples.front().requests[1].parameters.empty());
    REQUIRE(action.examples.front().requests[1].headers.empty());
    
    REQUIRE(action.examples.front().responses.size() == 2);
    
    REQUIRE(action.examples.front().responses[0].name == "200");
    REQUIRE(action.examples.front().responses[0].description == "G");
    REQUIRE(action.examples.front().responses[0].body == "H");
    REQUIRE(action.examples.front().responses[0].schema.empty());
    REQUIRE(action.examples.front().responses[0].parameters.empty());
    REQUIRE(action.examples.front().responses[0].headers.empty());
    
    REQUIRE(action.examples.front().responses[1].name == "200");
    REQUIRE(action.examples.front().responses[1].description == "N");
    REQUIRE(action.examples.front().responses[1].body == "J");
    REQUIRE(action.examples.front().responses[1].schema.empty());
    REQUIRE(action.examples.front().responses[1].parameters.empty());
    REQUIRE(action.examples.front().responses[1].headers.empty());

}

TEST_CASE("Parse method with multiple incomplete requests", "[action][blocks]")
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

    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 2); // empty asset & preformatted asset
    REQUIRE(result.first.warnings[0].code == EmptyDefinitionWarning);
    REQUIRE(result.first.warnings[1].code == IndentationWarning);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 9);
    
    REQUIRE(action.name.empty());    
    REQUIRE(action.method == "HEAD");
    REQUIRE(action.description.empty());
    REQUIRE(action.headers.empty());
    REQUIRE(action.parameters.empty());
    
    REQUIRE(action.examples.front().requests.size() == 2);
    REQUIRE(action.examples.front().requests[0].name == "A");
    REQUIRE(action.examples.front().requests[0].body.empty());
    REQUIRE(action.examples.front().requests[0].schema.empty());
    REQUIRE(action.examples.front().requests[0].parameters.empty());
    REQUIRE(action.examples.front().requests[0].headers.empty());
    
    REQUIRE(action.examples.front().requests[1].name == "B");
    REQUIRE(action.examples.front().requests[1].description.empty());
    REQUIRE(action.examples.front().requests[1].body == "3");
    REQUIRE(action.examples.front().requests[1].schema.empty());
    REQUIRE(action.examples.front().requests[1].parameters.empty());
    REQUIRE(action.examples.front().requests[1].headers.empty());
}

TEST_CASE("Parse method with foreign item", "[action][foreign][blocks]")
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
    
    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1);
    REQUIRE(result.first.warnings[0].code == IgnoringWarning);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 15);
    
    REQUIRE(action.name.empty());    
    REQUIRE(action.method == "MKCOL");
    REQUIRE(action.description.empty());
    REQUIRE(action.headers.empty());
    REQUIRE(action.parameters.empty());
    
    REQUIRE(action.examples.front().requests.size() == 1);
    REQUIRE(action.examples.front().requests[0].name.empty());
    REQUIRE(action.examples.front().requests[0].body == "Foo");
    REQUIRE(action.examples.front().requests[0].schema.empty());
    REQUIRE(action.examples.front().requests[0].parameters.empty());
    REQUIRE(action.examples.front().requests[0].headers.empty());    
}

TEST_CASE("Parse method with inline payload", "[action][blocks]")
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
    
    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1); // empty asset
    REQUIRE(result.first.warnings[0].code == EmptyDefinitionWarning);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 9);
    
    REQUIRE(action.name.empty());
    REQUIRE(action.method == "POST");
    REQUIRE(action.description.empty());
    REQUIRE(action.examples.front().responses.empty());
    REQUIRE(action.examples.front().requests.size() == 1);
    REQUIRE(action.examples.front().requests[0].name.empty());
    REQUIRE(action.examples.front().requests[0].description.empty());
    REQUIRE(action.examples.front().requests[0].body.empty());
}

TEST_CASE("Parse method with a HR", "[action][blocks]")
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
    
    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(action.name.empty());    
    REQUIRE(action.method == "PATCH");
    REQUIRE(action.description == "12");
    REQUIRE(action.examples.empty());
}

TEST_CASE( "Parse incomplete method followed by another resource", "[action][blocks]")
{
    // Blueprint in question:
    //R"(
    //# /1
    //## GET
    //# /2
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 2, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "/2", 1, MakeSourceDataBlock(1, 1)));
    
    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 1);
    
    REQUIRE(action.name.empty());    
    REQUIRE(action.method == "GET");
    REQUIRE(action.description.empty());
}

TEST_CASE("Check warnings on overshadowing a header", "[action][blocks]")
{
    MarkdownBlock::Stack markdown = CanonicalActionFixture();
    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    action.headers.push_back(std::make_pair("X-Header", "24"));
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(),
                                                    markdown.end(),
                                                    rootSection,
                                                    parser,
                                                    action);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.size() == 1);
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 26);
}

TEST_CASE("Parse method without name", "[action][blocks]")
{
    // Blueprint in question:
    //R"(
    //# GET
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "GET", 2, MakeSourceDataBlock(0, 1)));
    
    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 1);
    
    REQUIRE(action.name.empty());
    REQUIRE(action.method == "GET");
    REQUIRE(action.description.empty());
}

TEST_CASE("Make sure method with object payload is not parsed", "[action][blocks]")
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
    
    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
    
    REQUIRE(result.first.error.code != Error::OK);
}

TEST_CASE("Make sure method followed by a group does not eat the group", "[action][blocks]")
{
    // Blueprint in question:
    //R"(
    //## POST
    //# Group Two
    //");
    
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(HeaderBlockType, "POST", 2, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(HeaderBlockType, "Group Two", 1, MakeSourceDataBlock(1, 1)));
    
    Action action;
    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
    ParseSectionResult result = ActionParser::Parse(markdown.begin(), markdown.end(), rootSection, parser, action);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 1);

    REQUIRE(action.method == "POST");
    REQUIRE(action.description.empty());

}

TEST_CASE("Parse action with parameters", "[action][parameters][source]")
{
    // Blueprint in question:
    //R"(
    //# GET /resrouce/{id}
    //+ Parameters
    //    + id (required, number, `42`) ... Resource Id
    //
    //+ Response 204
    //");
    const std::string blueprintSource = \
    "# GET /resrouce/{id}\n"\
    "+ Parameters\n"\
    "    + id (required, number, `42`) ... Resource Id\n"\
    "\n"\
    "+ Response 204\n"\
    "\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].name == "id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].description == "Resource Id");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].type == "number");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].defaultValue.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].exampleValue == "42");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].parameters[0].values.empty());
}

TEST_CASE("Do not report empty message body for requests", "[action][#20][source]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Request
    //    + Headers 
    //
    //            Accept: application/json, application/javascript
    //
    //+ Response 204
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Request\n"\
    "    + Headers \n"\
    "\n"\
    "            Accept: application/json, application/javascript\n\n"\
    "+ Response 204\n\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].requests.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].headers.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].headers[0].first == "Accept");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].headers[0].second == "application/json, application/javascript");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].body.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
}

TEST_CASE("Give a warning of empty message body for requests with certain headers", "[action][#77][source]")
{
    // Blueprint in question:
    //R"(
    //# POST /1
    //+ Request
    //    + Headers
    //
    //            Content-Length: 100
    //
    //+ Response 204
    //");
    const std::string blueprintSource = \
    "# POST /1\n"\
    "+ Request\n"\
    "    + Headers \n"\
    "\n"\
    "            Content-Length: 100\n\n"\
    "+ Response 204\n\n";

    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);

    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].requests.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].headers.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].headers[0].first == "Content-Length");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].headers[0].second == "100");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].body.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
}

TEST_CASE("Give a warning when 100 response has a body", "[action][empty][response][source]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ Response 100
    //
    //       {}
    //");
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ Response 100\n\n"\
    "        {}\n\n";

    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);

    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);

    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "{}\n");
}

TEST_CASE("Give a warning when 2xx CONNECT has a body", "[action][#77][source]")
{
    // Blueprint in question:
    //R"(
    //# CONNECT /1
    //+ Response 201
    //
    //       {}
    //");
    const std::string blueprintSource = \
    "# CONNECT /1\n"\
    "+ Response 201\n\n"\
    "        {}\n\n";

    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);

    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);

    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].method == "CONNECT");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "{}\n");
}

TEST_CASE("Give a warning when response to HEAD has a body", "[action][head][method][source]")
{
    // Blueprint in question:
    //R"(
    //# HEAD /1
    //+ Response 204
    //
    //        {}
    //");
    const std::string blueprintSource = \
    "# HEAD /1\n"\
    "+ Response 204\n\n"\
    "        {}\n\n";

    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);

    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 2);

    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].method == "HEAD");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "{}\n");
}

TEST_CASE("Missing 'LINK' HTTP request method", "[action][issue][#46][source]")
{
    // Blueprint in question:
    //R"(
    //# LINK /1
    //+ Response 204
    //");
    const std::string blueprintSource = \
    "# LINK /1\n"\
    "+ Response 204\n\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].parameters.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].requests.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
}

TEST_CASE("Automatic request response pairing", "[pairing][action][issue][#53][source]")
{
    // Blueprint in question:
    //R"(
    //# Resource [/]
    //## Do Someting [POST]
    //
    //+ request A
    //
    //        A
    //
    //+ response 200
    //
    //        200
    //
    //+ request B
    //
    //        B
    //
    //+ response 400
    //
    //        400
    //
    //+ response 500
    //
    //        500
    //
    //+ request C
    //    
    //        C
    //
    //+ request D
    //
    //        D
    //
    //+ response 200
    //
    //        200
    //
    //");
    const std::string blueprintSource = \
    "# Resource [/]\n"\
    "## Do Someting [POST]\n"\
    "\n"\
    "+ request A\n"\
    "    \n"\
    "        A\n"\
    "\n"\
    "+ response 200\n"\
    "\n"\
    "        200\n"\
    "\n"\
    "+ request B\n"\
    "\n"\
    "        B\n"\
    "\n"\
    "+ response 400\n"\
    "\n"\
    "        400\n"\
    "\n"\
    "+ response 500\n"\
    "\n"\
    "        500\n"\
    "\n"\
    "+ request C\n"\
    "    \n"\
    "        C\n"\
    "\n"\
    "+ request D\n"\
    "\n"\
    "        D\n"\
    "\n"\
    "+ response 200\n"\
    "\n"\
    "        200\n"\
    "\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 3);

    // Example 1
    TransactionExample& example1 = blueprint.resourceGroups[0].resources[0].actions[0].examples[0];
    REQUIRE(example1.requests.size() == 1);
    REQUIRE(example1.requests[0].name == "A");
    REQUIRE(example1.requests[0].body == "A\n");
    REQUIRE(example1.responses.size() == 1);
    REQUIRE(example1.responses[0].name == "200");
    REQUIRE(example1.responses[0].body == "200\n");
    
    // Example 2
    TransactionExample& example2 = blueprint.resourceGroups[0].resources[0].actions[0].examples[1];
    REQUIRE(example2.requests.size() == 1);
    REQUIRE(example2.requests[0].name == "B");
    REQUIRE(example2.requests[0].body == "B\n");
    REQUIRE(example2.responses.size() == 2);
    REQUIRE(example2.responses[0].name == "400");
    REQUIRE(example2.responses[0].body == "400\n");
    REQUIRE(example2.responses[1].name == "500");
    REQUIRE(example2.responses[1].body == "500\n");

    // Example 3
    TransactionExample& example3 = blueprint.resourceGroups[0].resources[0].actions[0].examples[2];
    REQUIRE(example3.requests.size() == 2);
    REQUIRE(example3.requests[0].name == "C");
    REQUIRE(example3.requests[0].body == "C\n");
    REQUIRE(example3.requests[1].name == "D");
    REQUIRE(example3.requests[1].body == "D\n");
    REQUIRE(example3.responses.size() == 1);
    REQUIRE(example3.responses[0].name == "200");
    REQUIRE(example3.responses[0].body == "200\n");
}


TEST_CASE("Warn when request is not followed by a response", "[pairing][action][issue][#53][source]")
{
    // Blueprint in question:
    //R"(
    //# GET /1
    //+ response 200
    //
    //        200
    //
    //+ request A
    //
    //        A
    //);
    const std::string blueprintSource = \
    "# GET /1\n"\
    "+ response 200 \n"\
    "\n"\
    "        200\n"\
    "\n"\
    "+ request A\n"\
    "\n"\
    "        A\n";
    
    Parser parser;
    Result result;
    Blueprint blueprint;
    parser.parse(blueprintSource, 0, result, blueprint);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0].code == EmptyDefinitionWarning);
    
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 2);
}

