//
//  test-HeaderParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/22/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "HeadersParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

const mdp::ByteBuffer HeadersFixture = \
"+ Headers\n"\
"\n"\
"        Content-Type: application/json\n"\
"        X-My-Header: Hello World!\n";

TEST_CASE("recognize headers signature", "[headers]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(HeadersFixture, markdownAST);
    
    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<Headers>::sectionType(markdownAST.children().begin()) == HeadersSectionType);
}

TEST_CASE("parse headers fixture", "[headers]")
{
    Headers headers;
    Report report;
    SectionParserHelper<Headers, HeadersParser>::parse(HeadersFixture, HeadersSectionType, report, headers);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(headers.size() == 2);
    REQUIRE(headers[0].first == "Content-Type");
    REQUIRE(headers[0].second == "application/json");
    REQUIRE(headers[1].first == "X-My-Header");
    REQUIRE(headers[1].second == "Hello World!");
}

//
//TEST_CASE("hparser/parse", "Parse canonical header section")
//{
//    MarkdownBlock::Stack markdown = CanonicalHeaderFixture();
//    
//    CHECK(markdown.size() == 6);
//    
//    HeaderCollection headers;
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());    
//    ParseSectionResult result = HeadersParser::Parse(markdown.begin(),
//                                                     markdown.end(),
//                                                     rootSection,
//                                                     parser,
//                                                     headers);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//    CHECK(result.first.warnings.empty());
//    
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
//
//    REQUIRE(headers.size() == 2);
//    REQUIRE(headers[0].first == "Content-Type");
//    REQUIRE(headers[0].second == "application/json");
//    REQUIRE(headers[1].first == "X-My-Header");
//    REQUIRE(headers[1].second == "Hello World");
//}
//
//TEST_CASE("hparser/parse-malformed", "Parse malformed header section")
//{
//    MarkdownBlock::Stack markdown = CanonicalHeaderFixture();
//    
//    CHECK(markdown.size() == 6);
//    
//    markdown[3].content = "Content-Type: application/json\nX-My-Header:\n";
//    
//    HeaderCollection headers;
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    ParseSectionResult result = HeadersParser::Parse(markdown.begin(),
//                                                     markdown.end(),
//                                                     rootSection,
//                                                     parser,
//                                                     headers);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.size() == 1); // malformed header
//    
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
//    
//    REQUIRE(headers.size() == 1);
//    REQUIRE(headers[0].first == "Content-Type");
//    REQUIRE(headers[0].second == "application/json");
//}
//
//TEST_CASE("hparser/parse-multiple-blocks", "Parse header section composed of multiple blocks")
//{
//    // Blueprint in question:
//    //R"(
//    //+ Headers
//    //    Content-Type   : text/plain
//    //
//    //   B : 100
//    //
//    //		X-My-Header: 42
//    //)";
//    
//    MarkdownBlock::Stack markdown;
//    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
//    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
//    
//    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Headers\n  Content-Type   : text/plain\n", 0, MakeSourceDataBlock(0, 1)));
//    markdown.push_back(MarkdownBlock(ParagraphBlockType, "B : 100\n", 0, MakeSourceDataBlock(1, 1)));
//    markdown.push_back(MarkdownBlock(CodeBlockType, "X-My-Header: 42\n", 0, MakeSourceDataBlock(2, 1)));
//    
//    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
//    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
//    
//    HeaderCollection headers;
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    ParseSectionResult result = HeadersParser::Parse(markdown.begin(),
//                                                     markdown.end(),
//                                                     rootSection,
//                                                     parser,
//                                                     headers);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.size() == 3); // 2x not code block + 1x malformed (source map to "1")
//    
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 7);
//    
//    REQUIRE(headers.size() == 2);
//    REQUIRE(headers[0].first == "Content-Type");
//    REQUIRE(headers[0].second == "text/plain");
//    REQUIRE(headers[1].first == "X-My-Header");
//    REQUIRE(headers[1].second == "42");
//    
//}
//
//TEST_CASE("hparser/warnings", "Check warnings during parsing")
//{
//    MarkdownBlock::Stack markdown = CanonicalHeaderFixture();
//    
//    CHECK(markdown.size() == 6);
//    
//    HeaderCollection headers;
//    BlueprintParserCore parser(0, SourceDataFixture, Blueprint());
//    BlueprintSection rootSection(std::make_pair(markdown.begin(), markdown.end()));
//    ParseSectionResult result = HeadersParser::Parse(markdown.begin(),
//                                                     markdown.end(),
//                                                     rootSection,
//                                                     parser,
//                                                     headers);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.empty());
//    
//    const MarkdownBlock::Stack &blocks = markdown;
//    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
//    REQUIRE(headers.size() == 2);
//    
//    // Parse again with headers, check parser warnings
//    result = HeadersParser::Parse(markdown.begin(),
//                                  markdown.end(),
//                                  rootSection,
//                                  parser,
//                                  headers);
//    
//    REQUIRE(result.first.error.code == Error::OK);
//    REQUIRE(result.first.warnings.size() == 2);
//
//    REQUIRE(std::distance(blocks.begin(), result.second) == 6);
//    REQUIRE(headers.size() == 4);
//}
