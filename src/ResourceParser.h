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
    
    // Finds a resource in blueprint by its URI template
    typedef std::pair<Collection<ResourceGroup>::const_iterator, Collection<Resource>::const_iterator> ResourceMatch;
    ResourceMatch FindResource(const Blueprint& blueprint, const Resource& resource);

    // Parse Resource
    ParseSectionResult ParseResource(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Resource &resource);
}

#endif
