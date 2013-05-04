//
//  BlockClassifier.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_BLOCKCLASSIFIER_H
#define SNOWCRASH_BLOCKCLASSIFIER_H

#include "MarkdownBlock.h"

namespace snowcrash {

    // Classification Sections
    enum Section {
        UndefinedSection,
        OverviewSection,
        ResourceGroupSection,
        ResourceGroupTerminatingSection,
        ResourceSection,
        ResourceTerminatingSection,
        MethodSection,
        MethodTerminatingSection
    };
    
    // Classifies Markdown Block to one of the Sections
    Section ClassifyBlock(const MarkdownBlock& block, const Section& context);
}

#endif
