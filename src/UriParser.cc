//
//  UriParser.cc
//  snowcrash
//
//  Created by Carl Griffiths on 24/02/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "UriParser.h"
#include "RegexMatch.h"

using namespace snowcrash;




 bool URIParser::ContainsString(std::string s,std::string in){
     return in.find(s) != std::string::npos;
}


void URIParser::parse(const URI uri, URIResult& result)
{
    CaptureGroups groups;
    size_t gSize=5;

    if (RegexCapture(uri, URI_REGEX, groups, gSize)){
        result.scheme = groups[1];
        result.host = groups[3];
        result.path = groups[4];
        result.isValid = true;

        if (ContainsString("[", result.path) || ContainsString("]", result.path)){
            result.isValid = false;
            result.InvalidURIReason = InvalidURIReason::SquareBracketsInPath;
        }
    }
    else{
        result.isValid = false;
        result.InvalidURIReason = InvalidURIReason::UnparsableURI;
    }
}
