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
const std::string SerializeKey::Groups = "groups";

std::string snowcrash::EscapeNewlines(const std::string& input)
{
    std::ostringstream oss;
    std::istringstream iss(input);
    std::string line;
    while (std::getline(iss, line))
        oss << line << "\\n";
    
    return oss.str();
}
