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
const std::string HTTPHeaderName::ContentType = "Content-Type";

StatusCodeTraits snowcrash::GetStatusCodeTrait(HTTPStatusCode code)
{
    StatusCodeTraits traits;
    traits.code = code;

    // Following status codes MUST NOT containt response body
    if (code == 204 || code == 304) {
        traits.allowBody = false;
    }
    
    return traits;
}
