//
//  MethodParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_METHODPARSER_H
#define SNOWCRASH_METHODPARSER_H

#include "BlueprintParserCore.h"
#include "Blueprint.h"

namespace snowcrash {
    
    // Returns true if block has HTTP Method signature, false otherwise
    bool HasMethodSignature(const MarkdownBlock& block);

    // Parse method

}

#endif
