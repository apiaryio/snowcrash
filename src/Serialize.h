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

namespace snowcrash {
    
    std::string EscapeNewlines(const std::string& input);
    
    struct SerializeKey {
        static const std::string Metadata;
        static const std::string Name;
        static const std::string Description;
        static const std::string ResourceGroups;
        static const std::string Resources;
        static const std::string URI;
        static const std::string Methods;
        static const std::string Method;
        static const std::string Requests;
        static const std::string Responses;
        static const std::string Body;
        static const std::string Schema;
        static const std::string Headers;
    };
}

#endif
