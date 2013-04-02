//
//  test-MarkdownBlock.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/15/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "catch.hpp"
#include "MarkdownBlock.h"

using namespace snowcrash;

TEST_CASE("mdblock/init", "MD block construction")
{
    MarkdownBlock* block;
    REQUIRE_NOTHROW(block = ::new MarkdownBlock);

    REQUIRE(block->type == MarkdownBlockType::Undefined);
    REQUIRE(block->content.length() == 0);
    REQUIRE(block->data == static_cast<MarkdownBlock::Data>(0));
    REQUIRE(block->blocks.size() == 0);
    REQUIRE(block->sourceMap.size() == 0);
    
    REQUIRE_NOTHROW(::delete block);
}

TEST_CASE("mdblock/init-param", "MD block construction with parameters")
{
    const std::string source = "lorem ipsum";
    
    MarkdownBlock block(MarkdownBlockType::Code, source);
    REQUIRE(block.type == MarkdownBlockType::Code);
    REQUIRE(block.content == source);
    REQUIRE(block.data == static_cast<MarkdownBlock::Data>(0));
    REQUIRE(block.blocks.size() == 0);
    REQUIRE(block.sourceMap.size() == 0);
}

TEST_CASE("mdblock/move", "MD block move semantics")
{
    MarkdownBlock::Stack stack = {  MarkdownBlock(MarkdownBlockType::Paragraph, "1"),
                                    MarkdownBlock(MarkdownBlockType::Paragraph, "2") };
    
    REQUIRE(stack.front().content == "1");
    REQUIRE(stack.back().content == "2");
    
    MarkdownBlock block(MarkdownBlockType::ListItem);
    REQUIRE(block.blocks.size() == 0);    
    
    block.blocks = std::move(stack);
    
    REQUIRE(block.blocks.size() == 2);
    REQUIRE(block.blocks.front().content == "1");
    REQUIRE(block.blocks.back().content == "2");
    
}

