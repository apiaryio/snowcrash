//
//  ResourceParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_RESOURCEPARSER_H
#define SNOWCRASH_RESOURCEPARSER_H

#include "BlueprintParserCore.h"
#include "Blueprint.h"

namespace snowcrash {
    
    // Returns true if block has resource header signature, false otherwise
    bool HasResourceSignature(const MarkdownBlock& block);

    // Parse Resource into Blueprint AST
    ParseSectionResult ParseResource(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Blueprint &blueprint);
}

#endif
