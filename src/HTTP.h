//
//  HTTP.h
//  snowcrash
//
//  Created by Zdenek Nemec on 7/11/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_HTTP_H
#define SNOWCRASH_HTTP_H

#include <string>

/**
 *  HTTP Methods 
 */
#define HTTP_METHODS "GET|POST|PUT|DELETE|OPTIONS|PATCH|PROPPATCH|LOCK|UNLOCK|COPY|MOVE|MKCOL|HEAD"

/** 
 *  URI Template 
 */
#define URI_TEMPLATE "/[^]]+"

namespace snowcrash {
    
    /**
     *  Selected HTTP Header names
     */
    struct HTTPHeaderName {
        static const std::string Accept;
        static const std::string ContentType;
    };
}

#endif
