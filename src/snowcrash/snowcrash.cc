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
#include "Serialize.h"

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
        for (auto it = annotation.location.begin(); it != annotation.location.end(); ++it) {
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
    
    for (auto it = result.warnings.begin(); it != result.warnings.end(); ++it) {
        PrintAnnotation("warning:", *it);
    }
}

int main(int argc, const char *argv[])
{
    std::stringstream ss;
    ss << std::cin.rdbuf();

    snowcrash::parse(ss.str(), [](const snowcrash::Result& result, const snowcrash::Blueprint& blueprint){
        
        PrintResult(result);
        Serialize(blueprint, std::cout);

    });

    return 0;
}

