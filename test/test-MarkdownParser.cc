//
//  test-MarkdownParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/15/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
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
    MarkdownBlock markdown;
    
    parser.parse("", result, markdown);
    REQUIRE(result.error.code == Error::OK);
}

TEST_CASE("mdparser/parse-flat", "parsing flat Markdown into AST")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock markdown;
    
    const std::string source = \
"\n\
# header\n\
paragraph\n\
\n\
    code\n\
";

    parser.parse(source, result, markdown);

    REQUIRE(result.error.code == Error::OK);
    
    REQUIRE(markdown.type == UndefinedBlockType);
    REQUIRE(markdown.blocks.size() == 3);
    
    // Header block
    REQUIRE(markdown.blocks[0].type == HeaderBlockType);
    REQUIRE(markdown.blocks[0].content == "header");
    REQUIRE(markdown.blocks[0].data == 1);
    REQUIRE(markdown.blocks[0].blocks.size() == 0);
    REQUIRE(markdown.blocks[0].sourceMap.size() == 1);
    REQUIRE(markdown.blocks[0].sourceMap.back().location == 1);
    REQUIRE(markdown.blocks[0].sourceMap.back().length == 9);
    std::string sourceData = MapSourceData(source, markdown.blocks[0].sourceMap);
    REQUIRE(sourceData == "# header\n");
    
    // Paragraph block
    REQUIRE(markdown.blocks[1].type == ParagraphBlockType);
    REQUIRE(markdown.blocks[1].content == "paragraph");
    REQUIRE(markdown.blocks[1].data == 0);
    REQUIRE(markdown.blocks[1].blocks.size() == 0);
    sourceData = MapSourceData(source, markdown.blocks[1].sourceMap);
    REQUIRE(sourceData == "paragraph\n\n");
    
    // Code block
    REQUIRE(markdown.blocks[2].type == CodeBlockType);
    REQUIRE(markdown.blocks[2].content == "code\n");
    REQUIRE(markdown.blocks[2].data == 0);
    REQUIRE(markdown.blocks[2].blocks.size() == 0);
    sourceData = MapSourceData(source, markdown.blocks[2].sourceMap);
    REQUIRE(sourceData == "    code\n");

}

TEST_CASE("mdparser/parse-html", "parsing Markdown with HTML into AST")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock markdown;
    
    const std::string source = \
"# header\n\
<p>some text</p>\n\
";
    
    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    
    REQUIRE(markdown.type == UndefinedBlockType);
    REQUIRE(markdown.blocks.size() == 2);
    
    // Header block
    REQUIRE(markdown.blocks[0].type == HeaderBlockType);
    REQUIRE(markdown.blocks[0].content == "header");
    REQUIRE(markdown.blocks[0].data == 1);
    REQUIRE(markdown.blocks[0].blocks.size() == 0);
    
    // HTML block
    REQUIRE(markdown.blocks[1].type == HTMLBlockType);
    REQUIRE(markdown.blocks[1].content == "<p>some text</p>\n");
    REQUIRE(markdown.blocks[1].data == 0);
    REQUIRE(markdown.blocks[1].blocks.size() == 0);
    std::string sourceData = MapSourceData(source, markdown.blocks[1].sourceMap);
    REQUIRE(sourceData == "<p>some text</p>\n");
}

TEST_CASE("mdparser/parse-hr", "parsing Markdown with horizontal rule into AST")
{
    Result result;
    MarkdownBlock markdown;
    MarkdownParser parser;
    
    const std::string source = \
"# header\n\
\n\
---\n\
";

    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    
    REQUIRE(markdown.type == UndefinedBlockType);
    REQUIRE(markdown.blocks.size() == 2);
    
    // Header block
    REQUIRE(markdown.blocks[0].type == HeaderBlockType);
    REQUIRE(markdown.blocks[0].content == "header");
    REQUIRE(markdown.blocks[0].data == 1);
    REQUIRE(markdown.blocks[0].blocks.size() == 0);
    
    // HR block
    REQUIRE(markdown.blocks[1].type == HRuleBlockType);
    REQUIRE(markdown.blocks[1].content == "");
    REQUIRE(markdown.blocks[1].data == 0);
    REQUIRE(markdown.blocks[1].blocks.size() == 0);
    std::string sourceData = MapSourceData(source, markdown.blocks[1].sourceMap);
    REQUIRE(sourceData == "---\n");

}

TEST_CASE("mdparser/parse-blockquote", "parsing Markdown with blockquote into AST")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock markdown;
    
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
    
    REQUIRE(markdown.type == UndefinedBlockType);
    REQUIRE(markdown.blocks.size() == 2);
    
    // First quote
    REQUIRE(markdown.blocks[0].type == QuoteBlockType);
    REQUIRE(markdown.blocks[0].content == "");
    REQUIRE(markdown.blocks[0].data == 0);
    REQUIRE(markdown.blocks[0].blocks.size() == 4);
    std::string sourceData = MapSourceData(source, markdown.blocks[0].sourceMap);
    REQUIRE(sourceData == \
"> ## header\n\
> blockquote-1\n\
>\n\
> > blockquote-2\n\
>\n\
> blockquote-3\n\
\n\
");
        
    MarkdownBlock quote1 = markdown.blocks[0];

    // header
    REQUIRE(quote1.blocks[0].type == HeaderBlockType);
    REQUIRE(quote1.blocks[0].content == "header");
    REQUIRE(quote1.blocks[0].data == 2);
    REQUIRE(quote1.blocks[0].blocks.size() == 0);
    std::string sourceData1 = MapSourceData(source, quote1.blocks[0].sourceMap);
    REQUIRE(sourceData1 == "## header\n");
    
    // blockquote-1
    REQUIRE(quote1.blocks[1].type == ParagraphBlockType);
    REQUIRE(quote1.blocks[1].content == "blockquote-1");
    REQUIRE(quote1.blocks[1].data == 0);
    REQUIRE(quote1.blocks[1].blocks.size() == 0);
    std::string sourceData2 = MapSourceData(source, quote1.blocks[1].sourceMap);
    REQUIRE(sourceData2 == "blockquote-1\n\n");
    
    // Second quote
    REQUIRE(quote1.blocks[2].type == QuoteBlockType);
    REQUIRE(quote1.blocks[2].content == "");
    REQUIRE(quote1.blocks[2].data == 0);
    REQUIRE(quote1.blocks[2].blocks.size() == 1);
    std::string sourceData3 = MapSourceData(source, quote1.blocks[2].sourceMap);
    REQUIRE(sourceData3 == "> blockquote-2\n\n");
    
    // blockquote-2
    MarkdownBlock quote2 = quote1.blocks[2];
    REQUIRE(quote2.blocks[0].type == ParagraphBlockType);
    REQUIRE(quote2.blocks[0].content == "blockquote-2");
    REQUIRE(quote2.blocks[0].data == 0);
    REQUIRE(quote2.blocks[0].blocks.size() == 0);
    std::string sourceData4 = MapSourceData(source, quote2.blocks[0].sourceMap);
    REQUIRE(sourceData4 == "blockquote-2\n");
    
    // blockquote-3
    REQUIRE(quote1.blocks[3].type == ParagraphBlockType);
    REQUIRE(quote1.blocks[3].content == "blockquote-3");
    REQUIRE(quote1.blocks[3].data == 0);
    REQUIRE(quote1.blocks[3].blocks.size() == 0);
    std::string sourceData5 = MapSourceData(source, quote1.blocks[3].sourceMap);
    REQUIRE(sourceData5 == "blockquote-3\n");
    
    // paragraph-1
    REQUIRE(markdown.blocks[1].type == ParagraphBlockType);
    REQUIRE(markdown.blocks[1].content == "paragraph-1");
    REQUIRE(markdown.blocks[1].data == 0);
    REQUIRE(markdown.blocks[1].blocks.size() == 0);
    std::string sourceData6 = MapSourceData(source, markdown.blocks[1].sourceMap);
    REQUIRE(sourceData6 == "paragraph-1\n\n");
}

TEST_CASE("mdparser/parse-src-map", "parsing simple nested Markdown into AST and testing its source mapping")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock markdown;
    
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
    
    REQUIRE(markdown.type == UndefinedBlockType);
    REQUIRE(markdown.blocks.size() == 1);
    
    MarkdownBlock list = markdown.blocks[0];
    REQUIRE(list.type == ListBlockType);
    REQUIRE(list.blocks.size() == 1);
    REQUIRE(list.sourceMap.size() == 1);
    REQUIRE(list.sourceMap.back().location == 1);
    REQUIRE(list.sourceMap.back().length == 69);

    MarkdownBlock listItem = list.blocks[0];
    REQUIRE(listItem.type == ListItemBlockType);
    REQUIRE(listItem.blocks.size() == 3);
    REQUIRE(listItem.sourceMap.size() == 1);
    REQUIRE(listItem.sourceMap.back().location == 1);
    REQUIRE(listItem.sourceMap.back().length == 69);
    
    MarkdownBlock para1 = listItem.blocks[0];
    REQUIRE(para1.type == ParagraphBlockType);
    REQUIRE(para1.blocks.size() == 0);
    REQUIRE(para1.sourceMap.size() == 1);
    REQUIRE(para1.sourceMap.back().location == 3);
    REQUIRE(para1.sourceMap.back().length == 6);
    
    MarkdownBlock para2 = listItem.blocks[1];
    REQUIRE(para2.type == ParagraphBlockType);
    REQUIRE(para2.blocks.size() == 0);
    REQUIRE(para2.sourceMap.size() == 2);
    REQUIRE(para2.sourceMap[0].location == 13);
    REQUIRE(para2.sourceMap[0].length == 10);
    REQUIRE(para2.sourceMap[1].location == 27);
    REQUIRE(para2.sourceMap[1].length == 10);
    
    MarkdownBlock list2 = listItem.blocks[2];
    REQUIRE(list2.type == ListBlockType);
    REQUIRE(list2.blocks.size() == 1);
    REQUIRE(list2.sourceMap.size() == 2);
    REQUIRE(list2.sourceMap[0].location == 46);
    REQUIRE(list2.sourceMap[0].length == 8);
    REQUIRE(list2.sourceMap[1].location == 58);
    REQUIRE(list2.sourceMap[1].length == 12);

    MarkdownBlock listItem2 = list2.blocks[0];
    REQUIRE(listItem2.type == ListItemBlockType);
    REQUIRE(listItem2.blocks.size() == 0);
    REQUIRE(listItem2.sourceMap.size() == 2);
    REQUIRE(listItem2.sourceMap[0].location == 46); // 48:6 without marks
    REQUIRE(listItem2.sourceMap[0].length == 8);
    REQUIRE(listItem2.sourceMap[1].location == 58);
    REQUIRE(listItem2.sourceMap[1].length == 12);
}

TEST_CASE("mdparser/parse-nested", "parsing complex nested Markdown into AST")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock markdown;
    
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

    REQUIRE(markdown.type == UndefinedBlockType);
    REQUIRE(markdown.blocks.size() == 4);

    // paragraph-1
    REQUIRE(markdown.blocks[0].type == ParagraphBlockType);
    REQUIRE(markdown.blocks[0].content == "paragraph-1");
    REQUIRE(markdown.blocks[0].data == 0);
    REQUIRE(markdown.blocks[0].blocks.size() == 0);
    
    // list1 (listitem-1 & listitem-2)
    MarkdownBlock list1 = markdown.blocks[1];
    REQUIRE(list1.type == ListBlockType);
    REQUIRE(list1.content == "");
    REQUIRE(list1.data == 0);
    REQUIRE(list1.blocks.size() == 2);
    
    // listitem-1
    MarkdownBlock listItem1 = list1.blocks[0];
    REQUIRE(listItem1.type == ListItemBlockType);
    REQUIRE(listItem1.content == "");
    REQUIRE(listItem1.data == 0);
    REQUIRE(listItem1.blocks.size() == 2);
    
    REQUIRE(listItem1.blocks[0].type == ParagraphBlockType);
    REQUIRE(listItem1.blocks[0].content == "listitem-1-paragraph-1");
    REQUIRE(listItem1.blocks[0].data == 0);
    REQUIRE(listItem1.blocks[0].blocks.size() == 0);
    
    REQUIRE(listItem1.blocks[1].type == ParagraphBlockType);
    REQUIRE(listItem1.blocks[1].content == "listitem-1-paragraph-2");
    REQUIRE(listItem1.blocks[1].data == 0);
    REQUIRE(listItem1.blocks[1].blocks.size() == 0);
    
    // listitem-2
    MarkdownBlock listItem2 = list1.blocks[1];
    REQUIRE(listItem2.type == ListItemBlockType);
    REQUIRE(listItem2.content == "");
    REQUIRE(listItem2.data == 0);
    REQUIRE(listItem2.blocks.size() == 2);
    
    REQUIRE(listItem2.blocks[0].type == ParagraphBlockType);
    REQUIRE(listItem2.blocks[0].content == "listitem-2-paragraph-1");
    REQUIRE(listItem2.blocks[0].data == 0);
    REQUIRE(listItem2.blocks[0].blocks.size() == 0);
    
    REQUIRE(listItem2.blocks[1].type == ParagraphBlockType);
    REQUIRE(listItem2.blocks[1].content == "listitem-2-paragraph-2");
    REQUIRE(listItem2.blocks[1].data == 0);
    REQUIRE(listItem2.blocks[1].blocks.size() == 0);
    
    // paragraph-2
    MarkdownBlock paragraph2 = markdown.blocks[2];
    REQUIRE(paragraph2.type == ParagraphBlockType);
    REQUIRE(paragraph2.content == "paragraph-2");
    REQUIRE(paragraph2.data == 0);
    REQUIRE(paragraph2.blocks.size() == 0);
    
    // list2 (listitem-3)
    MarkdownBlock list2 = markdown.blocks[3];
    REQUIRE(list2.type == ListBlockType);
    REQUIRE(list2.content == "");
    REQUIRE(list2.data == 0);
    REQUIRE(list2.blocks.size() == 1);
    
    // listitem-3
    MarkdownBlock listItem3 = list2.blocks[0];
    REQUIRE(listItem3.type == ListItemBlockType);
    REQUIRE(listItem3.content == "");
    REQUIRE(listItem3.data == 0);
    REQUIRE(listItem3.blocks.size() == 3);
    
    REQUIRE(listItem3.blocks[0].type == ParagraphBlockType);
    REQUIRE(listItem3.blocks[0].content == "listitem-3-paragraph-1");
    REQUIRE(listItem3.blocks[0].data == 0);
    REQUIRE(listItem3.blocks[0].blocks.size() == 0);
    
    REQUIRE(listItem3.blocks[1].type == ParagraphBlockType);
    REQUIRE(listItem3.blocks[1].content == "listitem-3-paragraph-2");
    REQUIRE(listItem3.blocks[1].data == 0);
    REQUIRE(listItem3.blocks[1].blocks.size() == 0);
    
    REQUIRE(listItem3.blocks[2].type == ListBlockType);
    REQUIRE(listItem3.blocks[2].content == "");
    REQUIRE(listItem3.blocks[2].data == 0);
    REQUIRE(listItem3.blocks[2].blocks.size() == 1);
    
    // listitem-4
    MarkdownBlock listItem4 = listItem3.blocks[2].blocks[0];
    REQUIRE(listItem4.type == ListItemBlockType);
    REQUIRE(listItem4.content == "");
    REQUIRE(listItem4.data == 0);
    REQUIRE(listItem4.blocks.size() == 3);
    
    REQUIRE(listItem4.blocks[0].type == ParagraphBlockType);
    REQUIRE(listItem4.blocks[0].content == "listitem-4-paragraph-1");
    REQUIRE(listItem4.blocks[0].data == 0);
    REQUIRE(listItem4.blocks[0].blocks.size() == 0);
    
    REQUIRE(listItem4.blocks[1].type == ParagraphBlockType);
    REQUIRE(listItem4.blocks[1].content == "listitem-4-paragraph-2");
    REQUIRE(listItem4.blocks[1].data == 0);
    REQUIRE(listItem4.blocks[1].blocks.size() == 0);
    
    REQUIRE(listItem4.blocks[2].type == CodeBlockType);
    REQUIRE(listItem4.blocks[2].content == "listitem-4-code\n");
    REQUIRE(listItem4.blocks[2].data == 0);
    REQUIRE(listItem4.blocks[2].blocks.size() == 0);
}

TEST_CASE("mdparser/parse-fenced-code", "parsing fenced code block into AST")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock markdown;

    const std::string source = \
"\n\
```\n\
code\n\
```\n\
";
    
    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    
    REQUIRE(markdown.type == UndefinedBlockType);
    REQUIRE(markdown.blocks.size() == 1);
    
    REQUIRE(markdown.blocks[0].type == CodeBlockType);
    REQUIRE(markdown.blocks[0].content == "code\n");
    REQUIRE(markdown.blocks[0].data == 0);
    REQUIRE(markdown.blocks[0].blocks.size() == 0);
}

TEST_CASE("mdparser/parse-inline-list", "parsing inplace-list")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock markdown;

    const std::string source = \
"\n\
+ list1\n\
";
    
    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    
    REQUIRE(markdown.type == UndefinedBlockType);
    REQUIRE(markdown.blocks.size() == 1);
    
    REQUIRE(markdown.blocks[0].type == ListBlockType);
    REQUIRE(markdown.blocks[0].content == "");
    REQUIRE(markdown.blocks[0].data == 0);
    REQUIRE(markdown.blocks[0].blocks.size() == 1);
    
    REQUIRE(markdown.blocks[0].blocks[0].type == ListItemBlockType);
    REQUIRE(markdown.blocks[0].blocks[0].content == "list1\n");
    REQUIRE(markdown.blocks[0].blocks[0].data == 0);
    REQUIRE(markdown.blocks[0].blocks[0].blocks.size() == 0);
}

TEST_CASE("mdparser/parse-header-only", "parsing asserting header one liner")
{
    MarkdownParser parser;
    Result result;
    MarkdownBlock markdown;

    const std::string source = "# My API";
    

    parser.parse(source, result, markdown);
    
    REQUIRE(result.error.code == Error::OK);
    
    REQUIRE(markdown.type == UndefinedBlockType);
    REQUIRE(markdown.blocks.size() == 1);
    
    REQUIRE(markdown.blocks[0].type == HeaderBlockType);
    REQUIRE(markdown.blocks[0].content == "My API");
    REQUIRE(markdown.blocks[0].data == 1);
    REQUIRE(markdown.blocks[0].blocks.size() == 0);
}

