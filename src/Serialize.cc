//
//  Serialize.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/3/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "Serialize.h"
#include "StringUtility.h"

using namespace snowcrash;

const std::string SerializeKey::Metadata = "metadata";
const std::string SerializeKey::Name = "name";
const std::string SerializeKey::Description = "description";
const std::string SerializeKey::ResourceGroups = "resourceGroups";
const std::string SerializeKey::Resources = "resources";
const std::string SerializeKey::URI = "uri";
const std::string SerializeKey::URITemplate = "uriTemplate";
const std::string SerializeKey::Actions = "actions";
const std::string SerializeKey::Action = "action";
const std::string SerializeKey::Method = "method";
const std::string SerializeKey::Transactions = "transactions";
const std::string SerializeKey::Transaction = "transaction";
const std::string SerializeKey::Requests = "requests";
const std::string SerializeKey::Responses = "responses";
const std::string SerializeKey::Body = "body";
const std::string SerializeKey::Schema = "schema";
const std::string SerializeKey::Headers = "headers";
const std::string SerializeKey::Model = "model";

std::string snowcrash::EscapeNewlines(const std::string& input)
{
    return ReplaceString(input, "\n", "\\n");
}

std::string snowcrash::EscapeDoubleQuotes(const std::string& input)
{
    return ReplaceString(input, "\"", "\\\"");
}
