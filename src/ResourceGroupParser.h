//
//  ResourceGroupParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_RESOURCEGROUPPARSER_H
#define SNOWCRASH_RESOURCEGROUPPARSER_H

#include "BlueprintParserCore.h"
#include "Blueprint.h"

namespace snowcrash {
    
    // Finds a group in blueprint by name
    Collection<ResourceGroup>::iterator FindResourceGroup(Blueprint& blueprint, const ResourceGroup& group);
    
    // Parse Resource Group descending into Resources
    ParseSectionResult ParseResourceGroup(const BlockIterator& begin,
                                          const BlockIterator& end,
                                          const SourceData& sourceData,
                                          const Blueprint& blueprint,
                                          ResourceGroup& group);
}

#endif