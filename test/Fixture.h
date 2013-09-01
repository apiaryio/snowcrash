//
//  Fixtures.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/23/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_FIXTURES_H
#define SNOWCRASH_FIXTURES_H

#include "MarkdownBlock.h"

namespace snowcrashtest {

    extern const snowcrash::SourceData SourceDataFixture;

    extern snowcrash::MarkdownBlock::Stack CanonicalBlueprintFixture();
    extern snowcrash::MarkdownBlock::Stack CanonicalResourceGroupFixture();
    extern snowcrash::MarkdownBlock::Stack CanonicalResourceFixture();
    extern snowcrash::MarkdownBlock::Stack CanonicalActionFixture();
    extern snowcrash::MarkdownBlock::Stack CanonicalPayloadFixture();
    extern snowcrash::MarkdownBlock::Stack CanonicalBodyAssetFixture();
    extern snowcrash::MarkdownBlock::Stack CanonicalSchemaAssetFixture();
    extern snowcrash::MarkdownBlock::Stack CanonicalParametersFixture();
    extern snowcrash::MarkdownBlock::Stack CanonicalParameterDefinitionFixture();
}

#endif
