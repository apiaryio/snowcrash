//
//  Serialize.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/3/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_SERIALIZE_H
#define SNOWCRASH_SERIALIZE_H

#include <string>
#include "SerializeJSON.h"
#include "SerializeYAML.h"

namespace snowcrash {
    
    std::string EscapeNewlines(const std::string& input);
    
    struct SerializeKey {
        static const std::string Name;
        static const std::string Description;
        static const std::string Groups;
    };
}

#endif
