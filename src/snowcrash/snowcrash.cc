//
//  snowcrash.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/27/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include "snowcrash.h"
#include "SerializeJSON.h"
#include "SerializeYAML.h"
#include "cmdline.h"
#include "Version.h"

using snowcrash::SourceAnnotation;
using snowcrash::Error;

static const std::string OutputArgument = "output";
static const std::string FormatArgument = "format";
static const std::string RenderArgument = "render";
static const std::string ValidateArgument = "validate";
static const std::string VersionArgument = "version";

/// \enum Snow Crash AST output format.
enum SerializationFormat {
    YAMLSerializationFormat,
    JSONSerializationFormat
};

/// \brief Print Markdown source annotation.
/// \param prefix A string prefix for the annotation
/// \param annotation An annotation to print
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
        for (snowcrash::SourceCharactersBlock::const_iterator it = annotation.location.begin();
             it != annotation.location.end();
             ++it) {
            std::cerr << ((it == annotation.location.begin()) ? " :" : ";");
            std::cerr << it->location << ":" << it->length;
        }
    }
    
    std::cerr << std::endl;
}

/// \brief Print parser result to stderr.
/// \param result A parser result to print
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
    cmdline::parser argumentParser;

    argumentParser.set_program_name("snowcrash");
    std::stringstream ss;
    ss << "<input file>\n\n";
    ss << "API Blueprint Parser\n";
    ss << "If called without <input file>, 'snowcrash' will listen on stdin.\n";
    argumentParser.footer(ss.str());

    argumentParser.add<std::string>(OutputArgument, 'o', "save output AST into file", false);
    argumentParser.add<std::string>(FormatArgument, 'f', "output AST format", false, "yaml", cmdline::oneof<std::string>("yaml", "json"));
    // TODO: argumentParser.add("render", 'r', "render markdown descriptions");
    argumentParser.add("help", 'h', "display this help message");
    argumentParser.add(VersionArgument, 'v', "print Snow Crash version");
    argumentParser.add(ValidateArgument, 'l', "validate input only, do not print AST");
    
    argumentParser.parse_check(argc, argv);
    
    // Check arguments
    if (argumentParser.rest().size() > 1) {
        std::cerr << "one input file expected, got " << argumentParser.rest().size() << std::endl;
        exit(EXIT_FAILURE);
    }
    
    // Version query
    if (argumentParser.exist(VersionArgument)) {
        std::cout << SNOWCRASH_VERSION_STRING << std::endl;
        exit(EXIT_SUCCESS);
    }

    // Input
    std::stringstream inputStream;
    if (argumentParser.rest().empty()) {
        // Read stdin
        inputStream << std::cin.rdbuf();
    }
    else {
        // Read from file
        std::ifstream inputFileStream;
        std::string inputFileName = argumentParser.rest().front();
        inputFileStream.open(inputFileName.c_str());
        if (!inputFileStream.is_open()) {
            std::cerr << "fatal: unable to open input file '" << inputFileName << "'\n";
            exit(EXIT_FAILURE);
        }
        
        inputStream << inputFileStream.rdbuf();
        inputFileStream.close();
    }

    // Parse
    snowcrash::BlueprintParserOptions options = 0;  // Or snowcrash::RequireBlueprintNameOption
    snowcrash::Result result;
    snowcrash::Blueprint blueprint;
    snowcrash::parse(inputStream.str(), options, result, blueprint);
    
    // Output
    if (!argumentParser.exist(ValidateArgument)) {
        
        std::stringstream outputStream;

        if (argumentParser.get<std::string>(FormatArgument) == "json") {
            SerializeJSON(blueprint, outputStream);
        }
        else if (argumentParser.get<std::string>(FormatArgument) == "yaml") {
            SerializeYAML(blueprint, outputStream);
        }
        
        std::string outputFileName = argumentParser.get<std::string>(OutputArgument);
        if (!outputFileName.empty()) {
            // Serialize to file
            std::ofstream outputFileStream;
            outputFileStream.open(outputFileName.c_str());
            if (!outputFileStream.is_open()) {
                std::cerr << "fatal: unable to write to file '" <<  outputFileName << "'\n";
                exit(EXIT_FAILURE);
            }
            
            outputFileStream << outputStream.rdbuf();
            outputFileStream.close();
        }
        else {
            // Serialize to stdout
            std::cout << outputStream.rdbuf();
        }
    }
    
    // Result
    PrintResult(result);
    return result.error.code;
}
