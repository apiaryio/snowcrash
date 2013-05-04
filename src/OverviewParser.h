//
//  OverviewParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_OVERVIEWPARSER_H
#define SNOWCRASH_OVERVIEWPARSER_H

#include "BlueprintParserCore.h"
#include "Blueprint.h"

namespace snowcrash {

    // Parse Overview section
    ParseSectionResult ParseOverview(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Blueprint &blueprint);
}

#endif
