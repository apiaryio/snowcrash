//
//  SerializeYAML.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/3/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SERIALIZE_YAML_H
#define SNOWCRASH_SERIALIZE_YAML_H

#include <ostream>
#include "BlueprintSourcemap.h"

namespace snowcrash {

    // Naive YAML serialization to ostream
    void SerializeYAML(const snowcrash::Blueprint& blueprint, std::ostream &os);

    // Naive Sourcmap YAML serialization to ostream
    void SerializeSourceMapYAML(const snowcrash::SourceMap<snowcrash::Blueprint>& blueprint, std::ostream &os);
}

#endif
