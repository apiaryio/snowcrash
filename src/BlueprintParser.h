//
//  BlueprintParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_BLUEPRINTPARSER_H
#define SNOWCRASH_BLUEPRINTPARSER_H

#include <functional>
#include "BlueprintParserCore.h"
#include "Blueprint.h"

namespace snowcrash {

    //
    // Blueprint Parser, (descend)
    //
    class BlueprintParser {
    public:
        // Parse Markdown AST into API Blueprint AST
        // TODO: make result return value
        // TODO: rename parse to Parse
        void parse(const SourceData& sourceData, const MarkdownBlock::Stack& source, Result& result, Blueprint& blueprint);
    };
}

#endif
