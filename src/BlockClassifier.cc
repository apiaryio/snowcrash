//
//  BlockClassifier.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "BlockClassifier.h"
#include "ResourceParser.h"
#include "MethodParser.h"

using namespace snowcrash;

Section snowcrash::ClassifyBlock(const MarkdownBlock& block, const Section& context)
{
    if (context == OverviewSection) {
        
        // We are in Overview section
        // inclusive terminator: HR
        // exclusive terminator: Resource Header
        
        if (block.type != HeaderBlockType)
            return OverviewSection;
        
        if (HasResourceSignature(block))
            return ResourceSection;
        else
            return OverviewSection;
    }
    else if (context == UndefinedSection) {
        
        // We are in an undefined top-level section
        // Entering after parsed top level section. Which implies
        // only expected sections are Resource or Resource group.
        
        if (block.type != HeaderBlockType)
            return ResourceGroupSection;
        
        if (HasResourceSignature(block))
            return ResourceSection;
        else
            return ResourceGroupSection;
    }
    else if (context == ResourceGroupSection) {
        
        // We are in Resource Group section
        // exclusive terminator: Resource Header
        
        if (block.type != HeaderBlockType)
            return ResourceGroupSection;
        
        if (HasResourceSignature(block))
            return ResourceSection;
        else
            return ResourceGroupSection;
    }
    else if (context == ResourceSection) {
        
        // We are in Resource section
        // exclusive terminator: Parameters List, Headers List, Method Header, Resource Header
        
        if (block.type != HeaderBlockType &&
            block.type != ListBlockType)
            return ResourceSection;
        
        if (block.type == HeaderBlockType) {
            if (HasResourceSignature(block)) {
                return ResourceTerminatingSection;
            }
            else if (HasMethodSignature(block)) {
                return MethodSection;
            }
            
            return ResourceSection;
        }
        
        if (block.type == ListBlockType) {
            /// TODO:
            return ResourceSection;
        }
    }
    
    return UndefinedSection;
}