//
//  MethodParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "MethodParser.h"
#include "BlockClassifier.h"
#include "RegexMatch.h"

using namespace snowcrash;

// Method header regex
static const std::string MethodHeaderRegex("^(" HTTP_METHODS ")[[:space:]]*$");

bool snowcrash::HasMethodSignature(const MarkdownBlock& block)
{
    if (block.type != HeaderBlockType ||
        block.content.empty())
        return false;
    
    return RegexMatch(block.content, MethodHeaderRegex);
}
