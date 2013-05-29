//
//  snowcrash.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/27/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <iostream>
#include <sstream>
#include "snowcrash.h"
#include "SerializeJSON.h"
#include "SerializeYAML.h"

using snowcrash::SourceAnnotation;
using snowcrash::Error;

void PrintAnnotation(const std::string& prefix, const snowcrash::SourceAnnotation& annotation)
{
    std::cerr << prefix;
    
    if (annotation.code != SourceAnnotation::OK) {
        std::cerr << " (" << annotation.code << ") ";
    }
    
    if (!annotation.message.empty()) {
        std::cerr << " " << annotation.message;
    }
    
    if (!annotation.location.empty()) {
        for (snowcrash::SourceDataBlock::const_iterator it = annotation.location.begin(); it != annotation.location.end(); ++it) {
            std::cerr << ((it == annotation.location.begin()) ? " :" : ";");
            std::cerr << it->location << ":" << it->length;
        }
    }
    
    std::cerr << std::endl;
}

void PrintResult(const snowcrash::Result& result)
{
    std::cerr << std::endl;
    
    if (result.error.code == Error::OK) {
        std::cerr << "OK.\n";
    }
    else {
        PrintAnnotation("error:", result.error);
    }
    
    for (snowcrash::Warnings::const_iterator it = result.warnings.begin(); it != result.warnings.end(); ++it) {
        PrintAnnotation("warning:", *it);
    }
}

int main(int argc, const char *argv[])
{
    std::stringstream ss;
    ss << std::cin.rdbuf();

    snowcrash::Result result;
    snowcrash::Blueprint blueprint;
    snowcrash::parse(ss.str(), snowcrash::RequireBlueprintNameOption, result, blueprint);
        
    PrintResult(result);
    SerializeYAML(blueprint, std::cout);

    return 0;
}

