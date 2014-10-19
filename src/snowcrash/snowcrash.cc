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
static const std::string SourcemapArgument = "sourcemap";
static const std::string ValidateArgument = "validate";
static const std::string VersionArgument = "version";
static const std::string AnnotationByLineArgument = "annotation-line";

/** enum Snow Crash AST output format. */
enum SerializationFormat {
    YAMLSerializationFormat,
    JSONSerializationFormat
};

/**
 *  \brief Convert character index mapping to line and column number
 *  \param source, Source sting
 *  \param range Character index mapping as input
 *  \param fromLine Starting line number as output
 *  \param fromColumn Starting character number as output
 *  \param toLine Ending line number as output
 *  \param toColumn Ending character number as output
 */
void getLineFromMap(const std::string source,
                    const mdp::Range range,
                    size_t& fromLine,
                    size_t& fromColumn,
                    size_t& toLine,
                    size_t& toColumn)
{

    fromLine = 1;
    fromColumn = 1;

    // Finds starting line and column position
    for (int i = 0 ; i < range.location ; i++) {

        fromColumn++;

        if(source[i] == '\n') {

            fromLine++;
            fromColumn = 1;
        }
    }

    toLine = fromLine;
    toColumn = fromColumn;

    // Finds ending line and column position
    for (int i = range.location ; i < (range.location + range.length - 1) ; i++) {

        toColumn++;

        if(source[i] == '\n') {

            toLine++;
            toColumn = 1;
        }
    }
}

/**
 *  \brief Print Markdown source annotation.
 *  \param prefix A string prefix for the annotation
 *  \param annotation An annotation to print
 *  \param source Source sting
 *  \param showAnnotationByLine True if the annotations needs to be printed by line and column number
 */
void PrintAnnotation(const std::string& prefix,
                     const snowcrash::SourceAnnotation& annotation,
                     const std::string source,
                     const bool showAnnotationByLine)
{

    std::cerr << prefix;

    if (annotation.code != SourceAnnotation::OK) {
        std::cerr << " (" << annotation.code << ") ";
    }

    if (!annotation.message.empty()) {
        std::cerr << " " << annotation.message;
    }

    if (!annotation.location.empty()) {

        for (mdp::CharactersRangeSet::const_iterator it = annotation.location.begin();
             it != annotation.location.end();
             ++it) {

            if (showAnnotationByLine) {

                std::cerr << ((it == annotation.location.begin()) ? " :" : ";");

                size_t fromLine, fromColumn, toLine, toColumn;
                getLineFromMap(source, *it, fromLine, fromColumn, toLine, toColumn);

                std::cerr << " on line " << fromLine << ", column " << fromColumn;
                std::cerr << " - line " << toLine << ", column " << toColumn;
            }
            else {

                std::cerr << ((it == annotation.location.begin()) ? " :" : ";");
                std::cerr << it->location << ":" << it->length;
            }
        }
    }

    std::cerr << std::endl;
}

/**
 *  \brief Print parser report to stderr.
 *  \param report A parser report to print
 *  \param source Source sting
 *  \param showAnnotationByLine True if the annotations needs to be printed by line and column number
 */
void PrintReport(const snowcrash::Report& report,
                 const std::string source,
                 const bool showAnnotationByLine)
{

    std::cerr << std::endl;

    if (report.error.code == Error::OK) {
        std::cerr << "OK.\n";
    }
    else {
        PrintAnnotation("error:", report.error, source, showAnnotationByLine);
    }

    for (snowcrash::Warnings::const_iterator it = report.warnings.begin(); it != report.warnings.end(); ++it) {
        PrintAnnotation("warning:", *it, source, showAnnotationByLine);
    }
}

int main(int argc, const char *argv[])
{

    cmdline::parser argumentParser;
    std::stringstream inputStream;
    bool showAnnotationByLine = false;
    std::stringstream footerStream;

    argumentParser.set_program_name("snowcrash");

    footerStream << "<input file>\n\n";
    footerStream << "API Blueprint Parser\n";
    footerStream << "If called without <input file>, 'snowcrash' will listen on stdin.\n";

    argumentParser.footer(footerStream.str());

    argumentParser.add<std::string>(OutputArgument, 'o', "save output AST into file", false);
    argumentParser.add<std::string>(FormatArgument, 'f', "output AST format", false, "yaml", cmdline::oneof<std::string>("yaml", "json"));
    argumentParser.add<std::string>(SourcemapArgument, 's', "export sourcemap AST into file", false);
    // TODO: argumentParser.add("render", 'r', "render markdown descriptions");
    argumentParser.add("help", 'h', "display this help message");
    argumentParser.add(VersionArgument, 'v', "print Snow Crash version");
    argumentParser.add(ValidateArgument, 'l', "validate input only, do not print AST");
    argumentParser.add(AnnotationByLineArgument, 'a', "report the annotation, according to line and column number");

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

    // check if annotation map should be reported by line and column number
    if (argumentParser.exist(AnnotationByLineArgument)) {
        showAnnotationByLine = true;
    }

    // Initialize
    snowcrash::BlueprintParserOptions options = 0;  // Or snowcrash::RequireBlueprintNameOption
    snowcrash::ParseResult<snowcrash::Blueprint> blueprint;

    if (argumentParser.exist(SourcemapArgument)) {
        options |= snowcrash::ExportSourcemapOption;
    }

    // Parse
    snowcrash::parse(inputStream.str(), options, blueprint);

    // Output
    if (!argumentParser.exist(ValidateArgument)) {

        std::stringstream outputStream;
        std::stringstream sourcemapOutputStream;

        if (argumentParser.get<std::string>(FormatArgument) == "json") {

            SerializeJSON(blueprint.node, outputStream);
            SerializeSourceMapJSON(blueprint.sourceMap, sourcemapOutputStream);
        }
        else if (argumentParser.get<std::string>(FormatArgument) == "yaml") {

            SerializeYAML(blueprint.node, outputStream);
            SerializeSourceMapYAML(blueprint.sourceMap, sourcemapOutputStream);
        }

        std::string outputFileName = argumentParser.get<std::string>(OutputArgument);
        std::string sourcemapOutputFileName = argumentParser.get<std::string>(SourcemapArgument);

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

        if (!sourcemapOutputFileName.empty()) {

            // Serialize to file
            std::ofstream sourcemapOutputFileStream;
            sourcemapOutputFileStream.open(sourcemapOutputFileName.c_str());

            if (!sourcemapOutputFileStream.is_open()) {

                std::cerr << "fatal: unable to write to file '" << sourcemapOutputFileName << "'\n";
                exit(EXIT_FAILURE);
            }

            sourcemapOutputFileStream << sourcemapOutputStream.rdbuf();
            sourcemapOutputFileStream.close();
        }
    }

    // report
    PrintReport(blueprint.report, inputStream.str(), showAnnotationByLine);

    return blueprint.report.error.code;
}
