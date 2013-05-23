//
//  Fixtures.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/23/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_FIXTURES_H
#define SNOWCRASH_FIXTURES_H

#include "MarkdownBlock.h"

namespace snowcrashtest {

    extern const snowcrash::SourceData SourceDataFixture;

    extern snowcrash::MarkdownBlock::Stack CanonicalMethodFixture();
    extern snowcrash::MarkdownBlock::Stack CanonicalPayloadFixture();
    extern snowcrash::MarkdownBlock::Stack CanonicalBodyAssetFixture();
    extern snowcrash::MarkdownBlock::Stack CanonicalSchemaAssetFixture();
}

#endif
