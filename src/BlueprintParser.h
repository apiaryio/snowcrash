//
//  BlueprintParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_BLUEPRINTPARSER_H
#define SNOWCRASH_BLUEPRINTPARSER_H

#include "Parser.h"
#include "Blueprint.h"
#include "MarkdownBlock.h"

namespace snowcrash {

    //
    // Blueprint Parser, (descend)
    //
    class BlueprintParser {
    public:
        using ParseHandler = std::function<void(const Result&, const Blueprint&)>;
        
        // Parse Markdown AST into API Blueprint AST
        void parse(const MarkdownBlock& source, const ParseHandler& callback);
    };
}

#endif
