//
//  perf-snowcrash.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 10/8/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/time.h>
#include <cmath>
#include "cmdline.h"
#include "snowcrash.h"

using snowcrash::SourceAnnotation;
using snowcrash::Error;

static const int TestRunCount = 1000;

/**
 *  \brief  Parse input @TestRunCount -times
 *  \param  input   A blueprint source data.
 *  \param  total   Total time spent parsing (s).
 *  \param  mean    Mean time spent parsing (s).
 *  \param  stddev  Standard deviation.
 *  \return Result code of snowcrash::parse operation.
 */
static int testfunc(const std::string& input, double& total, double& mean, double& stddev)
{
	struct timeval stime, etime;
	double t = 0, sum = 0, sum2 = 0;
	int resultCode = snowcrash::Error::OK;

	for (int i = 0; i < TestRunCount; ++i) {
        snowcrash::BlueprintParserOptions options = 0;
        snowcrash::Result result;
        snowcrash::Blueprint blueprint;

		// Do the test.
		if (::gettimeofday(&stime, NULL)) {
            std::cerr << "fatal: gettimeofday failed";
			exit(EXIT_FAILURE);
		}
        
        snowcrash::parse(input, options, result, blueprint);
        
		if (::gettimeofday(&etime, NULL)) {
            std::cerr << "fatal: gettimeofday failed";
			exit(EXIT_FAILURE);
		}

        resultCode = result.error.code;
        
		// Compute the time taken and add it to the sums.
		t = (etime.tv_sec - stime.tv_sec) + (etime.tv_usec - stime.tv_usec) / 1000000.0;
		sum += t;
		sum2 += t * t;
	}
    
	// Compute statistics and return result
    total = sum;
	mean = sum / TestRunCount;
	stddev = std::sqrt((sum2 / TestRunCount) - (mean * mean));
	return resultCode;
}

int main(int argc, const char *argv[])
{
    // FIXME: Intstrumetns helper
    //::sleep(20);
    
    // Setup commandline Argument Parser
    cmdline::parser argumentParser;
    argumentParser.set_program_name("perf-snowcrash");
    std::stringstream ss;
    ss << "<input file>\n\n";
    ss << "API Blueprint Parser Performance Test Tool\n";

    argumentParser.footer(ss.str());
    argumentParser.add("help", 'h', "display this help message");

    argumentParser.parse_check(argc, argv);
    if (argumentParser.rest().size() != 1) {
        std::cerr << "one input file expected\n";
        exit(EXIT_FAILURE);
    }
    
    // Read fixture file
    std::ifstream inputFileStream;
    std::string inputFileName = argumentParser.rest().front();
    inputFileStream.open(inputFileName.c_str());
    if (!inputFileStream.is_open()) {
        std::cerr << "fatal: unable to open input file '" << inputFileName << "'\n";
        exit(EXIT_FAILURE);
    }
    
    std::stringstream inputStream;
    inputStream << inputFileStream.rdbuf();
    inputFileStream.close();
    
    std::cout << "running snowcrash performance test...\n";
    
    double mean = 0, total = 0, stddev = 0;
    int result = testfunc(inputStream.str(), total, mean, stddev);
    
    std::cout << "parsing '" << inputFileName << "' " << TestRunCount << "-times (" << result << "):\n";
    std::cout << "total: " << total << "s mean: " << mean << " +/- " << stddev << "s\n";
    
    // FIXME: Intstrumetns helper
    //::sleep(20);
}


