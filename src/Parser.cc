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

void Parser::parse(const SourceData &source, const ParseHandler &callback)
{
    // Do not procceed without callback, silent
    if (!callback)
        return;
    
    MarkdownParser markdownParser;
    markdownParser.parse(source, [&](const Result& markdownResult, const MarkdownBlock& markdownAST) {
        
        BlueprintParser blueprintParser;
        blueprintParser.parse(source, markdownAST, [&](const Result& blueprintResult, const Blueprint& blueprintAST) {

            // Finalize
            callback(blueprintResult, blueprintAST);

        });

    });

}
