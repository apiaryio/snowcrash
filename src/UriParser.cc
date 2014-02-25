//
//  UriParser.cc
//  snowcrash
//
//  Created by Carl Griffiths on 24/02/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "UriParser.h"
#include <regex>

using namespace snowcrash;


struct DoesNotContainString{
	std::string s;
	DoesNotContainString(std::string  s) :s{ s }{}
	bool operator()(const std::string &r){
		return r.find(s) == std::string::npos;
	};
};


void URIParser::parse(const URI uri, URIResult& result)
{
	try {
		std::smatch smatch;
		std::regex pattern(URI_REGEX, std::regex_constants::icase);
		std::regex_search(uri, smatch, pattern);
		result.scheme = smatch[1];
		result.host = smatch[3];
		result.path = smatch[4];
		result.isValid = DoesNotContainString{ "[" }(result.path) && DoesNotContainString{ "]" }(result.path);
		
	}
	catch (const std::regex_error&) {
	}
	catch (...) {
	}
}
