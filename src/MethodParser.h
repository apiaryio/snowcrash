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
    
    // Finds a method inside resource
    Collection<Method>::iterator FindMethod(Resource& resource, const Method& method);

    // Parse method
    ParseSectionResult ParseMethod(const BlockIterator& begin,
                                   const BlockIterator& end,
                                   const SourceData& sourceData,
                                   Method& method);
}

#endif
