//
//  Parser.cpp
//  snowcrash
//
//  Created by Zdenek Nemec on 4/8/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <exception>
#include <sstream>
#include "Parser.h"
#include "MarkdownParser.h"
#include "BlueprintParser.h"

using namespace snowcrash;

const int SourceAnnotation::OK = 0;

// Check source for unsupported character \t & \r
// Returns true if passed (not found), false otherwise
static bool CheckSource(const SourceData& source, Result& result)
{
    std::string::size_type pos = source.find("\t");
    if (pos != std::string::npos) {
        result.error = Error("the use of tab(s) '\\t' in source data isn't currently supported, please contact makers",
                             2,
                             MapSourceDataBlock(MakeSourceDataBlock(pos, 1), source));
        return false;
    }

    pos = source.find("\r");
    if (pos != std::string::npos) {
        result.error = Error("the use of carriage return(s) '\\r' in source data isn't currently supported, please contact makers",
                             2,
                             MapSourceDataBlock(MakeSourceDataBlock(pos, 1), source));
        return false;
    }
    
    return true;
}

void Parser::parse(const SourceData& source, BlueprintParserOptions options, Result& result, Blueprint& blueprint)
{
    try {
        
        // Sanity Check
        if (!CheckSource(source, result))
            return;
        
        // Parse Markdown
        MarkdownBlock::Stack markdown;
        MarkdownParser markdownParser;
        markdownParser.parse(source, result, markdown);
        
        if (result.error.code != Error::OK)
            return;
        
        // Parse Blueprint
        BlueprintParser::Parse(source, markdown, options, result, blueprint);
    }
    catch (const std::exception& e) {

        std::stringstream ss;
        ss << "parser exception: '" << e.what() << "'";
        result.error = Error(ss.str(), 1);
    }
    catch (...) {
        
        result.error = Error("parser exception has occured", 1);
    }
}
