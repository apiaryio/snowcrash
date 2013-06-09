//
//  SerializeJSON.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/27/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SERIALIZE_JSON_H
#define SNOWCRASH_SERIALIZE_JSON_H

#include <ostream>
#include "Blueprint.h"

namespace snowcrash {

    // Naive JSON serialization to ostream
    void SerializeJSON(const snowcrash::Blueprint& blueprint, std::ostream &os);
}

#endif 
