//
//  test-ListUtility.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 9/5/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "Fixture.h"
#include "ListBlockUtility.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Content Block", "[list]")
{
    MarkdownBlock::Stack markdown;
    
    //+ Top 1
    //    + Mid
    //        + Inner
    //+ Top 2
    
    // Top 1 begin
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Top", 0, MakeSourceDataBlock(1, 1)));
    
    // Mid begin
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));

    // Inner begin
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));

    // Inner end
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Inner", 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    
    // Mid end
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Mid", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(5, 1)));
    
    // Top 1 end
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));

    // Top 2 begin
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Top 2", 0, MakeSourceDataBlock(7, 1)));

    // Top 2 end
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(8, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(9, 1)));
    
    const MarkdownBlock::Stack &blocks = markdown;
    BlockIterator begin = blocks.begin();
    
    // Top list begin to Top 1 list item
    BlockIterator firstContentBlock = ContentBlock(begin, markdown.end());
    REQUIRE(std::distance(begin, firstContentBlock) == 1);
    
    // Top 1 list item to Mid list begin
    std::advance(begin, 1);
    firstContentBlock = ContentBlock(begin, markdown.end());
    REQUIRE(std::distance(begin, firstContentBlock) == 2);
    
    // Mid list begin to Mid list's first list item
    begin = firstContentBlock;
    firstContentBlock = ContentBlock(begin, markdown.end());
    REQUIRE(std::distance(begin, firstContentBlock) == 1);
    
    // Mid list's first list item to Inner list begin
    begin = firstContentBlock;
    firstContentBlock = ContentBlock(begin, markdown.end());
    REQUIRE(std::distance(begin, firstContentBlock) == 1);
    
    // Inner list begin to first Inner list item
    begin = firstContentBlock;
    firstContentBlock = ContentBlock(begin, markdown.end());
    REQUIRE(std::distance(begin, firstContentBlock) == 1);
    
    // Inner list item - nowhere to go
    begin = firstContentBlock;
    firstContentBlock = ContentBlock(begin, markdown.end());
    REQUIRE(std::distance(begin, firstContentBlock) == 0);
    
}
