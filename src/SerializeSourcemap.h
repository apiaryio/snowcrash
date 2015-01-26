//
//  SerializeSourcemap.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 18/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SERIALIZE_SOURCEMAP_H
#define SNOWCRASH_SERIALIZE_SOURCEMAP_H

#include "Serialize.h"

namespace snowcrash {

    sos::Object WrapBlueprintSourcemap(const SourceMap<Blueprint>& blueprint);
}

#endif
