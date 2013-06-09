//
//  Parser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/8/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_PARSER_H
#define SNOWCRASH_PARSER_H

#include <functional>
#include "Blueprint.h"
#include "BlueprintParserCore.h"

namespace snowcrash {
    
    //
    // API Blueprint Parser
    //
    class Parser {
    public:
        
        // Parse source data into Blueprint AST
        void parse(const SourceData& source, BlueprintParserOptions options, Result& result, Blueprint& blueprint);
    };
}

#endif
