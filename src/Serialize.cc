//
//  Serialize.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/3/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "Serialize.h"

using namespace snowcrash;

const std::string SerializeKey::Name = "name";
const std::string SerializeKey::Description = "description";
const std::string SerializeKey::ResourceGroups = "resourceGroups";
const std::string SerializeKey::Resources = "resources";
const std::string SerializeKey::URI = "uri";
const std::string SerializeKey::Methods = "methods";
const std::string SerializeKey::Method = "method";
const std::string SerializeKey::Requests = "requests";
const std::string SerializeKey::Responses = "responses";
const std::string SerializeKey::Body = "body";

std::string snowcrash::EscapeNewlines(const std::string& input)
{
    std::ostringstream oss;
    std::istringstream iss(input);
    std::string line;
    while (std::getline(iss, line))
        oss << line << "\\n";
    
    return oss.str();
}
