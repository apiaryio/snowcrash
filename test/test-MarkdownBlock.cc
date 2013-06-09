//
//  test-MarkdownBlock.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/15/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "catch.hpp"
#include "MarkdownBlock.h"

using namespace snowcrash;

TEST_CASE("mdblock/init", "MD block construction")
{
    MarkdownBlock* block;
    REQUIRE_NOTHROW(block = ::new MarkdownBlock);

    REQUIRE(block->type == UndefinedBlockType);
    REQUIRE(block->content.length() == 0);
    REQUIRE(block->data == static_cast<MarkdownBlock::Data>(0));
    REQUIRE(block->sourceMap.size() == 0);
    
    REQUIRE_NOTHROW(::delete block);
}

TEST_CASE("mdblock/init-param", "MD block construction with parameters")
{
    const std::string source = "lorem ipsum";
    
    MarkdownBlock block(CodeBlockType, source);
    REQUIRE(block.type == CodeBlockType);
    REQUIRE(block.content == source);
    REQUIRE(block.data == static_cast<MarkdownBlock::Data>(0));
    REQUIRE(block.sourceMap.size() == 0);
}
