//
//  HTTP.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 7/11/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "HTTP.h"

using namespace snowcrash;

const std::string HTTPHeaderName::Accept = "Accept";
const std::string HTTPHeaderName::ContentLength = "Content-Length";
const std::string HTTPHeaderName::ContentType = "Content-Type";
const std::string HTTPHeaderName::TransferEncoding = "Transfer-Encoding";

StatusCodeTraits snowcrash::GetStatusCodeTrait(HTTPStatusCode code)
{
    StatusCodeTraits traits;
    traits.code = code;

    // Following status codes MUST NOT contain response body
    if (code == 204 || code == 304 || code/100 == 1) {
        traits.allowBody = false;
    }
    
    return traits;
}

HTTPMethodTraits snowcrash::GetMethodTrait(HTTPMethod method)
{
    HTTPMethodTraits traits;
    traits.method = method;

    // Following HTTP methods MUST NOT contain response body
    if (method == "HEAD") {
        traits.allowBody = false;
    }

    return traits;
}
