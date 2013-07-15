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
 *  \brief HTTP Methods 
 *
 *  Technical note: Using preprocessor macro instead of strict
 *  defined type due to C++98 string concatenation limitations.
 *  To be improved with migration to C++11.
 */
#define HTTP_METHODS "GET|POST|PUT|DELETE|OPTIONS|PATCH|PROPPATCH|LOCK|UNLOCK|COPY|MOVE|MKCOL|HEAD"

/** 
 *  \brief URI Template.
 *
 *  See previous technical note (using macro).
 */
#define URI_TEMPLATE "/[^]]+"

namespace snowcrash {
    
    /**
     *  Selected HTTP Header names.
     */
    struct HTTPHeaderName {
        static const std::string Accept;
        static const std::string ContentType;
    };
    
    /**
     *  A HTTP Status code.
     */
    typedef unsigned int HTTPStatusCode;
    
    /**
     *  Traits of a HTTP response.
     */
    struct HTTPResponseTraits {
        
        bool allowBody; /// < Response body is allowed.
        
        HTTPResponseTraits() : allowBody(true) {}
    };

    /**
     *  Response traits for a HTTP status code.
     *
     *  Status-related response prescription.
     *  Ref: http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
     */
    struct StatusCodeTraits : HTTPResponseTraits
    {
        HTTPStatusCode code;
        StatusCodeTraits() : code(0) {}
    };
    
    /**
     *  \brief  Retrieve response traits for given status code.
     *  \param  code    A HTTP status code to retrieve traits for.
     *  \return A %StatusCodeTraits for given code.
     */
    extern StatusCodeTraits GetStatusCodeTrait(HTTPStatusCode code);
}

#endif
