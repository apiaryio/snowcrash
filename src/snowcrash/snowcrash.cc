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
#include "SerializeAST.h"
#include "SerializeSourcemap.h"
#include "cmdline.h"
#include "sosJSON.h"
#include "sosYAML.h"
#include "Version.h"

using snowcrash::SourceAnnotation;
using snowcrash::Error;

static const std::string OutputArgument = "output";
static const std::string FormatArgument = "format";
static const std::string RenderArgument = "render";
static const std::string SourcemapArgument = "sourcemap";
static const std::string ValidateArgument = "validate";
static const std::string VersionArgument = "version";
static const std::string UseLineNumbersArgument = "use-line-num";

/** enum Snow Crash AST output format. */
enum SerializationFormat {
    YAMLSerializationFormat,
    JSONSerializationFormat
};

/** structure contains starting and ending position of a error/warning. */
struct AnnotationPosition {
    size_t fromLine;
    size_t fromColumn;
    size_t toLine;
    size_t toColumn;
};

/**
 *  \brief Convert character index mapping to line and column number
 *  \param linesEndIndex Vector containing indexes of end line characters
 *  \param range Character index mapping as input
 *  \param out Position of the given range as output
 */
void GetLineFromMap(const std::vector<size_t>& linesEndIndex,
                    const mdp::Range& range,
                    AnnotationPosition& out)
{

    std::vector<size_t>::const_iterator annotationPositionIt;

    out.fromLine = 0;
    out.fromColumn = 0;
    out.toLine = 0;
    out.toColumn = 0;

    // Finds starting line and column position
    annotationPositionIt = std::upper_bound(linesEndIndex.begin(), linesEndIndex.end(), range.location) - 1;

    if (annotationPositionIt != linesEndIndex.end()) {

        out.fromLine = std::distance(linesEndIndex.begin(), annotationPositionIt) + 1;
        out.fromColumn = range.location - *annotationPositionIt + 1;
    }

    // Finds ending line and column position
    annotationPositionIt = std::lower_bound(linesEndIndex.begin(), linesEndIndex.end(), range.location + range.length) - 1;

    if (annotationPositionIt != linesEndIndex.end()) {

        out.toLine = std::distance(linesEndIndex.begin(), annotationPositionIt) + 1;
        out.toColumn = (range.location + range.length) - *annotationPositionIt + 1;

        if (*(annotationPositionIt + 1) == (range.location + range.length)) {
            out.toColumn--;
        }
    }
}

/**
 *  \brief Given the source returns the length of all the lines in source as a vector
 *  \param source Source data
 *  \param out Vector containing indexes of all end line character in source
 */
void GetLinesEndIndex(const std::string& source,
                      std::vector<size_t>& out)
{

    out.push_back(0);

    for (size_t i = 0; i < source.length(); i++) {

        if (source[i] == '\n') {
            out.push_back(i + 1);
        }
    }
}

/**
 *  \brief Print Markdown source annotation.
 *  \param prefix A string prefix for the annotation
 *  \param annotation An annotation to print
 *  \param source Source data
 *  \param isUseLineNumbers True if the annotations needs to be printed by line and column number
 */
void PrintAnnotation(const std::string& prefix,
                     const snowcrash::SourceAnnotation& annotation,
                     const std::string& source,
                     const bool isUseLineNumbers)
{

    std::cerr << prefix;

    if (annotation.code != SourceAnnotation::OK) {
        std::cerr << " (" << annotation.code << ") ";
    }

    if (!annotation.message.empty()) {
        std::cerr << " " << annotation.message;
    }

    std::vector<size_t> linesEndIndex;

    if (isUseLineNumbers) {
        GetLinesEndIndex(source, linesEndIndex);
    }

    if (!annotation.location.empty()) {

        for (mdp::CharactersRangeSet::const_iterator it = annotation.location.begin();
             it != annotation.location.end();
             ++it) {

            if (isUseLineNumbers) {

                AnnotationPosition annotationPosition;
                GetLineFromMap(linesEndIndex, *it, annotationPosition);

                std::cerr << "; line " << annotationPosition.fromLine << ", column " << annotationPosition.fromColumn;
                std::cerr << " - line " << annotationPosition.toLine << ", column " << annotationPosition.toColumn;
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
 *  \param source Source data
 *  \param isUseLineNumbers True if the annotations needs to be printed by line and column number
 */
void PrintReport(const snowcrash::Report& report,
                 const std::string& source,
                 const bool isUseLineNumbers)
{

    std::cerr << std::endl;

    if (report.error.code == Error::OK) {
        std::cerr << "OK.\n";
    }
    else {
        PrintAnnotation("error:", report.error, source, isUseLineNumbers);
    }

    for (snowcrash::Warnings::const_iterator it = report.warnings.begin(); it != report.warnings.end(); ++it) {
        PrintAnnotation("warning:", *it, source, isUseLineNumbers);
    }
}

int main(int argc, const char *argv[])
{

    cmdline::parser argumentParser;
    bool isUseLineNumbers = false;

    argumentParser.set_program_name("snowcrash");
    std::stringstream ss;

    ss << "<input file>\n\n";
    ss << "API Blueprint Parser\n";
    ss << "If called without <input file>, 'snowcrash' will listen on stdin.\n";

    argumentParser.footer(ss.str());

    argumentParser.add<std::string>(OutputArgument, 'o', "save output AST into file", false);
    argumentParser.add<std::string>(FormatArgument, 'f', "output AST format", false, "yaml", cmdline::oneof<std::string>("yaml", "json"));
    argumentParser.add<std::string>(SourcemapArgument, 's', "export sourcemap AST into file", false);
    // TODO: argumentParser.add("render", 'r', "render markdown descriptions");
    argumentParser.add("help", 'h', "display this help message");
    argumentParser.add(VersionArgument, 'v', "print Snow Crash version");
    argumentParser.add(ValidateArgument, 'l', "validate input only, do not print AST");
    argumentParser.add(UseLineNumbersArgument, 'u', "use line and row number instead of character index when printing annotation");

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

    // check if annotation map should be reported by line and column number
    if (argumentParser.exist(UseLineNumbersArgument)) {
        isUseLineNumbers = true;
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

        sos::Object outputObject = snowcrash::wrapBlueprint(blueprint.node);
        sos::Object sourcemapOutputObject = snowcrash::wrapBlueprintSourcemap(blueprint.sourceMap);

        if (argumentParser.get<std::string>(FormatArgument) == "json") {

            sos::SerializeJSON serializer = sos::SerializeJSON();

            serializer.process(outputObject, outputStream);
            serializer.process(sourcemapOutputObject, sourcemapOutputStream);
        }
        else if (argumentParser.get<std::string>(FormatArgument) == "yaml") {

            sos::SerializeYAML serializer = sos::SerializeYAML();

            serializer.process(outputObject, outputStream);
            serializer.process(sourcemapOutputObject, sourcemapOutputStream);
        }

        outputStream << "\n";
        sourcemapOutputStream << "\n";

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
    PrintReport(blueprint.report, inputStream.str(), isUseLineNumbers);

    return blueprint.report.error.code;
}
