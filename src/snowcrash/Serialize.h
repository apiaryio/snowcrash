//
//  Serialize.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/27/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_SERIALIZE_H
#define SNOWCRASH_SERIALIZE_H

#include <ostream>
#include "Blueprint.h"

// Naive serialization to ostream
void Serialize(const snowcrash::Blueprint& blueprint, std::ostream &os);

#endif 
