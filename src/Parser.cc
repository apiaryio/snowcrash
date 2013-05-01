//
//  Parser.cpp
//  snowcrash
//
//  Created by Zdenek Nemec on 4/8/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "Parser.h"
#include "MarkdownParser.h"
#include "BlueprintParser.h"

using namespace snowcrash;

const int SourceAnnotation::OK;

void Parser::parse(const SourceData& source, Result& result, Blueprint& blueprint)
{
    // Parse Markdown
    MarkdownBlock markdown;
    MarkdownParser markdownParser;
    markdownParser.parse(source, result, markdown);
    
    if (result.error.code != Error::OK)
        return;
    
    // Parse Blueprint
    BlueprintParser blueprintParser;
    blueprintParser.parse(source, markdown, result, blueprint);
}
