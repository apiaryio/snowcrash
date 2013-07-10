//
//  test-MarkdownParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/15/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "catch.hpp"
#include "MarkdownParser.h"

using namespace snowcrash;

TEST_CASE("mdparser/init", "MD parser construction")
{
    MarkdownParser* parser;
    REQUIRE_NOTHROW(parser = ::new MarkdownParser);
    REQUIRE_NOTHROW(::delete parser);
}

TEST_CASE("mdparser/parse-params", "parse() method parameters.")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock::Stack markdown;
    
    parser.parse("", result, markdown);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
}

TEST_CASE("mdparser/parse-flat", "parsing flat Markdown into AST")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock::Stack markdown;
    
    const std::string source = \
"\n\
# header\n\
paragraph\n\
\n\
    code\n\
";

    parser.parse(source, result, markdown);
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(markdown.size() == 3);
    
    // Header block
    REQUIRE(markdown[0].type == HeaderBlockType);
    REQUIRE(markdown[0].content == "header");
    REQUIRE(markdown[0].data == 1);
    REQUIRE(markdown[0].sourceMap.size() == 1);
    REQUIRE(markdown[0].sourceMap.back().location == 1);
    REQUIRE(markdown[0].sourceMap.back().length == 9);
    std::string sourceData = MapSourceData(source, markdown[0].sourceMap);
    REQUIRE(sourceData == "# header\n");
    
    // Paragraph block
    REQUIRE(markdown[1].type == ParagraphBlockType);
    REQUIRE(markdown[1].content == "paragraph");
    REQUIRE(markdown[1].data == 0);
    sourceData = MapSourceData(source, markdown[1].sourceMap);
    REQUIRE(sourceData == "paragraph\n\n");
    
    // Code block
    REQUIRE(markdown[2].type == CodeBlockType);
    REQUIRE(markdown[2].content == "code\n");
    REQUIRE(markdown[2].data == 0);
    sourceData = MapSourceData(source, markdown[2].sourceMap);
    REQUIRE(sourceData == "    code\n");
}

TEST_CASE("mdparser/parse-html", "parsing Markdown with HTML into AST")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock::Stack markdown;
    
    const std::string source = \
"# header\n\
<p>some text</p>\n\
";
    
    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(markdown.size() == 2);
    
    // Header block
    REQUIRE(markdown[0].type == HeaderBlockType);
    REQUIRE(markdown[0].content == "header");
    REQUIRE(markdown[0].data == 1);
    
    // HTML block
    REQUIRE(markdown[1].type == HTMLBlockType);
    REQUIRE(markdown[1].content == "<p>some text</p>\n");
    REQUIRE(markdown[1].data == 0);
    std::string sourceData = MapSourceData(source, markdown[1].sourceMap);
    REQUIRE(sourceData == "<p>some text</p>\n");
}

TEST_CASE("mdparser/parse-hr", "parsing Markdown with horizontal rule into AST")
{
    Result result;
    MarkdownBlock::Stack markdown;
    MarkdownParser parser;
    
    const std::string source = \
"# header\n\
\n\
---\n\
";

    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(markdown.size() == 2);
    
    // Header block
    REQUIRE(markdown[0].type == HeaderBlockType);
    REQUIRE(markdown[0].content == "header");
    REQUIRE(markdown[0].data == 1);
    
    // HR block
    REQUIRE(markdown[1].type == HRuleBlockType);
    REQUIRE(markdown[1].content.empty());
    REQUIRE(markdown[1].data == 0);
    std::string sourceData = MapSourceData(source, markdown[1].sourceMap);
    REQUIRE(sourceData == "---\n");

}

TEST_CASE("mdparser/parse-blockquote", "parsing Markdown with blockquote into AST")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock::Stack markdown;
    
    const std::string source = \
"\n\
> ## header\n\
> blockquote-1\n\
>\n\
> > blockquote-2\n\
>\n\
> blockquote-3\n\
\n\
paragraph-1\n\
\n\
";
    
    parser.parse(source, result, markdown);
        
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(markdown.size() == 9);
    
    // First quote
    REQUIRE(markdown[0].type == QuoteBlockBeginType);
    REQUIRE(markdown[0].content.empty());
    REQUIRE(markdown[0].data == 0);
    REQUIRE(markdown[0].sourceMap.empty());
    
    // header
    REQUIRE(markdown[1].type == HeaderBlockType);
    REQUIRE(markdown[1].content == "header");
    REQUIRE(markdown[1].data == 2);
    std::string sourceData1 = MapSourceData(source, markdown[1].sourceMap);
    REQUIRE(sourceData1 == "## header\n");
    
    // blockquote-1
    REQUIRE(markdown[2].type == ParagraphBlockType);
    REQUIRE(markdown[2].content == "blockquote-1");
    REQUIRE(markdown[2].data == 0);
    std::string sourceData2 = MapSourceData(source, markdown[2].sourceMap);
    REQUIRE(sourceData2 == "blockquote-1\n\n");
    
    // Second quote    
    REQUIRE(markdown[3].type == QuoteBlockBeginType);
    REQUIRE(markdown[3].content.empty());
    REQUIRE(markdown[3].data == 0);
    REQUIRE(markdown[3].sourceMap.empty());
    
    // blockquote-2
    REQUIRE(markdown[4].type == ParagraphBlockType);
    REQUIRE(markdown[4].content == "blockquote-2");
    REQUIRE(markdown[4].data == 0);
    std::string sourceData3 = MapSourceData(source, markdown[4].sourceMap);
    REQUIRE(sourceData3 == "blockquote-2\n");

    // Second quote end
    REQUIRE(markdown[5].type == QuoteBlockEndType);
    REQUIRE(markdown[5].content.empty());
    REQUIRE(markdown[5].data == 0);
    std::string sourceData4 = MapSourceData(source, markdown[5].sourceMap);
    REQUIRE(sourceData4 == "> blockquote-2\n\n");
    
    // blockquote-3
    REQUIRE(markdown[6].type == ParagraphBlockType);
    REQUIRE(markdown[6].content == "blockquote-3");
    REQUIRE(markdown[6].data == 0);
    std::string sourceData5 = MapSourceData(source, markdown[6].sourceMap);
    REQUIRE(sourceData5 == "blockquote-3\n");
    
    // First quote end
    REQUIRE(markdown[7].type == QuoteBlockEndType);
    REQUIRE(markdown[7].content.empty());
    REQUIRE(markdown[7].data == 0);
    std::string sourceData6 = MapSourceData(source, markdown[7].sourceMap);
    REQUIRE(sourceData6 == \
"> ## header\n\
> blockquote-1\n\
>\n\
> > blockquote-2\n\
>\n\
> blockquote-3\n\
\n\
");
    
    // paragraph-1
    REQUIRE(markdown[8].type == ParagraphBlockType);
    REQUIRE(markdown[8].content == "paragraph-1");
    REQUIRE(markdown[8].data == 0);
    std::string sourceData7 = MapSourceData(source, markdown[8].sourceMap);
    REQUIRE(sourceData7 == "paragraph-1\n\n");
}

TEST_CASE("mdparser/parse-src-map", "parsing simple nested Markdown into AST and testing its source mapping")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock::Stack markdown;
    
    const std::string source = \
"\n\
+ list\n\
\n\
    paragraph\n\
    loremipsu\n\
    \n\
    + list2\n\
        content\n\
";
    
    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(markdown.size() == 10);
    
    REQUIRE(markdown[0].type == ListBlockBeginType);
    REQUIRE(markdown[0].sourceMap.empty());

    REQUIRE(markdown[1].type == ListItemBlockBeginType);
    REQUIRE(markdown[1].sourceMap.empty());
    
    REQUIRE(markdown[2].type == ParagraphBlockType);
    REQUIRE(markdown[2].sourceMap.size() == 1);
    REQUIRE(markdown[2].sourceMap.back().location == 3);
    REQUIRE(markdown[2].sourceMap.back().length == 6);
    
    REQUIRE(markdown[3].type == ParagraphBlockType);
    REQUIRE(markdown[3].sourceMap.size() == 2);
    REQUIRE(markdown[3].sourceMap[0].location == 13);
    REQUIRE(markdown[3].sourceMap[0].length == 10);
    REQUIRE(markdown[3].sourceMap[1].location == 27);
    REQUIRE(markdown[3].sourceMap[1].length == 10);
    
    REQUIRE(markdown[4].type == ListBlockBeginType);
    REQUIRE(markdown[4].sourceMap.empty());

    REQUIRE(markdown[5].type == ListItemBlockBeginType);
    REQUIRE(markdown[5].sourceMap.empty());
    
    REQUIRE(markdown[6].type == ListItemBlockEndType);
    REQUIRE(markdown[6].sourceMap.size() == 2);
    REQUIRE(markdown[6].sourceMap[0].location == 48);
    REQUIRE(markdown[6].sourceMap[0].length == 6);
    REQUIRE(markdown[6].sourceMap[1].location == 62);
    REQUIRE(markdown[6].sourceMap[1].length == 8);
    
    REQUIRE(markdown[7].type == ListBlockEndType);
    REQUIRE(markdown[7].sourceMap.size() == 2);
    REQUIRE(markdown[7].sourceMap[0].location == 46);
    REQUIRE(markdown[7].sourceMap[0].length == 8);
    REQUIRE(markdown[7].sourceMap[1].location == 58);
    REQUIRE(markdown[7].sourceMap[1].length == 12);

    REQUIRE(markdown[8].type == ListItemBlockEndType);
    REQUIRE(markdown[8].sourceMap.size() == 5);
    REQUIRE(markdown[8].sourceMap[0].location == 3);
    REQUIRE(markdown[8].sourceMap[0].length == 6);
    REQUIRE(markdown[8].sourceMap[1].location == 13);
    REQUIRE(markdown[8].sourceMap[1].length == 10);
    REQUIRE(markdown[8].sourceMap[2].location == 27);
    REQUIRE(markdown[8].sourceMap[2].length == 10);
    REQUIRE(markdown[8].sourceMap[3].location == 46);
    REQUIRE(markdown[8].sourceMap[3].length == 8);
    REQUIRE(markdown[8].sourceMap[4].location == 58);
    REQUIRE(markdown[8].sourceMap[4].length == 12);
    
    REQUIRE(markdown[9].type == ListBlockEndType);
    REQUIRE(markdown[9].sourceMap.size() == 1);
    REQUIRE(markdown[9].sourceMap.back().location == 1);
    REQUIRE(markdown[9].sourceMap.back().length == 69);
    
}

TEST_CASE("mdparser/parse-src-map-inline-nested", "parse nested inline list testing source maps")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock::Stack markdown;
    
    const std::string source = \
"# /1\n\
## GET\n\
+ Request\n\
    + Schema\n\
    + Body\n\
\n";
    
    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(markdown.size() == 12);
    
    REQUIRE(markdown[0].type == HeaderBlockType);
    REQUIRE(markdown[0].sourceMap.size() == 1);
    REQUIRE(markdown[0].sourceMap[0].location == 0);
    REQUIRE(markdown[0].sourceMap[0].length == 5);

    REQUIRE(markdown[1].type == HeaderBlockType);
    REQUIRE(markdown[1].sourceMap.size() == 1);
    REQUIRE(markdown[1].sourceMap[0].location == 5);
    REQUIRE(markdown[1].sourceMap[0].length == 7);
    
    REQUIRE(markdown[2].type == ListBlockBeginType);
    REQUIRE(markdown[2].sourceMap.empty());
    
    REQUIRE(markdown[3].type == ListItemBlockBeginType);
    REQUIRE(markdown[3].sourceMap.empty());

    REQUIRE(markdown[4].type == ListBlockBeginType);
    REQUIRE(markdown[4].sourceMap.empty());
    
    REQUIRE(markdown[5].type == ListItemBlockBeginType);
    REQUIRE(markdown[5].sourceMap.empty());
    
    // "+ Schema"
    REQUIRE(markdown[6].type == ListItemBlockEndType);
    REQUIRE(markdown[6].sourceMap.size() == 1);
    REQUIRE(markdown[6].sourceMap[0].location == 28);
    REQUIRE(markdown[6].sourceMap[0].length == 7);
    
    REQUIRE(markdown[7].type == ListItemBlockBeginType);
    REQUIRE(markdown[7].sourceMap.empty());

    // "+ Body"
    REQUIRE(markdown[8].type == ListItemBlockEndType);
    REQUIRE(markdown[8].sourceMap.size() == 1);
    REQUIRE(markdown[8].sourceMap[0].location == 41);
    REQUIRE(markdown[8].sourceMap[0].length == 5);
    
    REQUIRE(markdown[9].type == ListBlockEndType);
    REQUIRE(markdown[9].sourceMap.size() == 2);
    REQUIRE(markdown[9].sourceMap[0].location == 26);
    REQUIRE(markdown[9].sourceMap[0].length == 9);
    REQUIRE(markdown[9].sourceMap[1].location == 39);
    REQUIRE(markdown[9].sourceMap[1].length == 7);
    
    REQUIRE(markdown[10].type == ListItemBlockEndType);
    REQUIRE(markdown[10].sourceMap.size() == 3);
    REQUIRE(markdown[10].sourceMap[0].location == 14);
    REQUIRE(markdown[10].sourceMap[0].length == 8);
    REQUIRE(markdown[10].sourceMap[1].location == 26);
    REQUIRE(markdown[10].sourceMap[1].length == 9);
    REQUIRE(markdown[10].sourceMap[2].location == 39);
    REQUIRE(markdown[10].sourceMap[2].length == 7);
    
    REQUIRE(markdown[11].type == ListBlockEndType);
    REQUIRE(markdown[11].sourceMap.size() == 1);
    REQUIRE(markdown[11].sourceMap[0].location == 12);
    REQUIRE(markdown[11].sourceMap[0].length == 35);
}

TEST_CASE("mdparser/parse-nested", "parsing complex nested Markdown into AST")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock::Stack markdown;
    
    const std::string source = \
"\n\
paragraph-1\n\
\n\
+ listitem-1-paragraph-1\n\
\n\
    listitem-1-paragraph-2\n\
\n\
+ listitem-2-paragraph-1\n\
\n\
    listitem-2-paragraph-2\n\
\n\
paragraph-2\n\
\n\
+ listitem-3-paragraph-1\n\
\n\
    listitem-3-paragraph-2\n\
\n\
    + listitem-4-paragraph-1\n\
\n\
        listitem-4-paragraph-2\n\
\n\
            listitem-4-code\n\
\n\
";

    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(markdown.size() == 25);

    // paragraph-1
    REQUIRE(markdown[0].type == ParagraphBlockType);
    REQUIRE(markdown[0].content == "paragraph-1");
    REQUIRE(markdown[0].data == 0);
    
    // list1 (: & listitem-2)
    REQUIRE(markdown[1].type == ListBlockBeginType);
    REQUIRE(markdown[1].content.empty());
    REQUIRE(markdown[1].data == 0);
    
    // listitem-1
    REQUIRE(markdown[2].type == ListItemBlockBeginType);
    REQUIRE(markdown[2].content.empty());
    REQUIRE(markdown[2].data == 0);
    
    REQUIRE(markdown[3].type == ParagraphBlockType);
    REQUIRE(markdown[3].content == "listitem-1-paragraph-1");
    REQUIRE(markdown[3].data == 0);
    
    REQUIRE(markdown[4].type == ParagraphBlockType);
    REQUIRE(markdown[4].content == "listitem-1-paragraph-2");
    REQUIRE(markdown[4].data == 0);
    
    REQUIRE(markdown[5].type == ListItemBlockEndType);
    REQUIRE(markdown[5].content.empty());
    
    // listitem-2
    REQUIRE(markdown[6].type == ListItemBlockBeginType);
    REQUIRE(markdown[6].content.empty());
    
    REQUIRE(markdown[7].type == ParagraphBlockType);
    REQUIRE(markdown[7].content == "listitem-2-paragraph-1");
    REQUIRE(markdown[7].data == 0);
    
    REQUIRE(markdown[8].type == ParagraphBlockType);
    REQUIRE(markdown[8].content == "listitem-2-paragraph-2");
    REQUIRE(markdown[8].data == 0);
    
    REQUIRE(markdown[9].type == ListItemBlockEndType);
    REQUIRE(markdown[9].content.empty());
    
    // list1 end
    REQUIRE(markdown[10].type == ListBlockEndType);
    REQUIRE(markdown[10].content.empty());
    
    // paragraph-2
    REQUIRE(markdown[11].type == ParagraphBlockType);
    REQUIRE(markdown[11].content == "paragraph-2");
    REQUIRE(markdown[11].data == 0);
    
    // list2 (listitem-3)
    REQUIRE(markdown[12].type == ListBlockBeginType);
    REQUIRE(markdown[12].content.empty());
    REQUIRE(markdown[12].data == 0);
    
    // listitem-3
    REQUIRE(markdown[13].type == ListItemBlockBeginType);
    REQUIRE(markdown[13].content.empty());
    REQUIRE(markdown[13].data == 0);
    
    REQUIRE(markdown[14].type == ParagraphBlockType);
    REQUIRE(markdown[14].content == "listitem-3-paragraph-1");
    REQUIRE(markdown[14].data == 0);
    
    REQUIRE(markdown[15].type == ParagraphBlockType);
    REQUIRE(markdown[15].content == "listitem-3-paragraph-2");
    REQUIRE(markdown[15].data == 0);
    
    // list3 (listitem-4)
    REQUIRE(markdown[16].type == ListBlockBeginType);
    REQUIRE(markdown[16].content.empty());
    REQUIRE(markdown[16].data == 0);
    
    // listitem-4
    REQUIRE(markdown[17].type == ListItemBlockBeginType);
    REQUIRE(markdown[17].content.empty());
    REQUIRE(markdown[17].data == 0);
    
    REQUIRE(markdown[18].type == ParagraphBlockType);
    REQUIRE(markdown[18].content == "listitem-4-paragraph-1");
    REQUIRE(markdown[18].data == 0);
    
    REQUIRE(markdown[19].type == ParagraphBlockType);
    REQUIRE(markdown[19].content == "listitem-4-paragraph-2");
    REQUIRE(markdown[19].data == 0);
    
    REQUIRE(markdown[20].type == CodeBlockType);
    REQUIRE(markdown[20].content == "listitem-4-code\n");
    REQUIRE(markdown[20].data == 0);
    
    REQUIRE(markdown[21].type == ListItemBlockEndType);
    REQUIRE(markdown[21].content.empty());

    // list3 end
    REQUIRE(markdown[22].type == ListBlockEndType);
    REQUIRE(markdown[22].content.empty());
    
    // listitem-3 end
    REQUIRE(markdown[23].type == ListItemBlockEndType);
    REQUIRE(markdown[23].content.empty());

    // list2 end
    REQUIRE(markdown[24].type == ListBlockEndType);
    REQUIRE(markdown[24].content.empty());
}

TEST_CASE("mdparser/parse-fenced-code", "parsing fenced code block into AST")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock::Stack markdown;

    const std::string source = \
"\n\
```\n\
code\n\
```\n\
";
    
    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(markdown.size() == 1);
    
    REQUIRE(markdown[0].type == CodeBlockType);
    REQUIRE(markdown[0].content == "code\n");
    REQUIRE(markdown[0].data == 0);
}

TEST_CASE("mdparser/parse-inline-list", "parsing inplace-list")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock::Stack markdown;

    const std::string source = \
"\n\
+ list1\n\
";
    
    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(markdown.size() == 4);
    
    REQUIRE(markdown[0].type == ListBlockBeginType);
    REQUIRE(markdown[0].content.empty());
    REQUIRE(markdown[0].data == 0);
    
    REQUIRE(markdown[1].type == ListItemBlockBeginType);
    REQUIRE(markdown[1].content.empty());
    REQUIRE(markdown[1].data == 0);
    
    REQUIRE(markdown[2].type == ListItemBlockEndType);
    REQUIRE(markdown[2].content == "list1\n");
    REQUIRE(markdown[2].data == 0);

    REQUIRE(markdown[3].type == ListBlockEndType);
    REQUIRE(markdown[3].content.empty());
    REQUIRE(markdown[3].data == 0);
}

TEST_CASE("mdparser/parse-header-only", "parsing asserting header one liner")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock::Stack markdown;

    const std::string source = "# My API";
    
    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    REQUIRE(markdown.size() == 1);
    
    REQUIRE(markdown[0].type == HeaderBlockType);
    REQUIRE(markdown[0].content == "My API");
    REQUIRE(markdown[0].data == 1);
}

TEST_CASE("Missing nested list item", "[markdown][issue][#12]")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock::Stack markdown;
    
    const std::string source = \
"\n\
+ Parent\n\
\n\
    + Item1\n\
\n\
    Para1\n\
\n\
    + Item2\n\
";
    
    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    REQUIRE(result.warnings.empty());
    
    REQUIRE(markdown.size() == 14);

    REQUIRE(markdown[0].type == ListBlockBeginType);
    REQUIRE(markdown[0].content.empty());
    REQUIRE(markdown[1].type == ListItemBlockBeginType);
    REQUIRE(markdown[1].content.empty());
    
    REQUIRE(markdown[2].type == ParagraphBlockType);
    REQUIRE(markdown[2].content == "Parent");
    
    REQUIRE(markdown[3].type == ListBlockBeginType);
    REQUIRE(markdown[3].content.empty());
    REQUIRE(markdown[4].type == ListItemBlockBeginType);
    REQUIRE(markdown[4].content.empty());
    
    REQUIRE(markdown[5].type == ListItemBlockEndType);
    REQUIRE(markdown[5].content == "Item1\n");
    REQUIRE(markdown[6].type == ListBlockEndType);
    REQUIRE(markdown[6].content.empty());
    
    REQUIRE(markdown[7].type == ParagraphBlockType);
    REQUIRE(markdown[7].content == "Para1");
    
    REQUIRE(markdown[8].type == ListBlockBeginType);
    REQUIRE(markdown[8].content.empty());
    REQUIRE(markdown[9].type == ListItemBlockBeginType);
    REQUIRE(markdown[9].content.empty());

    REQUIRE(markdown[10].type == ListItemBlockEndType);
    REQUIRE(markdown[10].content == "Item2\n");
    REQUIRE(markdown[11].type == ListBlockEndType);
    REQUIRE(markdown[11].content.empty());
    
    REQUIRE(markdown[12].type == ListItemBlockEndType);
    REQUIRE(markdown[12].content.empty());
    REQUIRE(markdown[13].type == ListBlockEndType);
    REQUIRE(markdown[13].content.empty());
}
