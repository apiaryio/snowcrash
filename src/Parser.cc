//
//  Parser.cpp
//  snowcrash
//
//  Created by Zdenek Nemec on 4/8/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <exception>
#include <sstream>
#include "Parser.h"
#include "MarkdownParser.h"
#include "BlueprintParser.h"

using namespace snowcrash;

const int SourceAnnotation::OK;

void Parser::parse(const SourceData& source, Result& result, Blueprint& blueprint)
{
    try {
        // Parse Markdown
        MarkdownBlock::Stack markdown;
        MarkdownParser markdownParser;
        markdownParser.parse(source, result, markdown);
        
        if (result.error.code != Error::OK)
            return;
        
        // Parse Blueprint
        BlueprintParser::Parse(source, markdown, result, blueprint);
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
