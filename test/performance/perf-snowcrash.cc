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
#include <cmath>
#include "snowcrash.h"

#if defined (_MSC_VER)
#include <windows.h>
#else
#include <sys/time.h>
#endif

using snowcrash::SourceAnnotation;
using snowcrash::Error;

static const int TestRunCount = 1000;

#if defined (_MSC_VER)
const __int64 DELTA_EPOCH_IN_MICROSECS = 11644473600000000;

struct timezone
{
    int tz_minuteswest;     /* minutes west of Greenwich */
    int tz_dsttime;         /* type of DST correction */
};

int gettimeofday(struct timeval* tv, struct timezone* tz)
{
    FILETIME ft;
    TIME_ZONE_INFORMATION tz_winapi;
    __int64 tmpres = 0;
    int rez = 0;

    ZeroMemory(&ft, sizeof(ft));
    ZeroMemory(&tz_winapi, sizeof(tz_winapi));

    GetSystemTimeAsFileTime(&ft);

    tmpres = ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    tmpres /= 10;
    tmpres -= DELTA_EPOCH_IN_MICROSECS;

    if (tv) {
        tv->tv_sec = (__int32)( tmpres * 0.000001 );
        tv->tv_usec = ( tmpres % 1000000 );
    }

    if (tz) {
        rez = GetTimeZoneInformation(&tz_winapi);
        tz->tz_dsttime = (rez == 2) ? true:false;
        tz->tz_minuteswest = tz_winapi.Bias + ((rez == 2) ? tz_winapi.DaylightBias:0);
    }

    return 0;
}
#endif


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
        snowcrash::ParseResult<snowcrash::Blueprint> blueprint;

		// Do the test.
		if (::gettimeofday(&stime, NULL)) {
            std::cerr << "fatal: gettimeofday failed";
			exit(EXIT_FAILURE);
		}
        
        snowcrash::parse(input, options, blueprint);
        
		if (::gettimeofday(&etime, NULL)) {
            std::cerr << "fatal: gettimeofday failed";
			exit(EXIT_FAILURE);
		}

        resultCode = blueprint.report.error.code;
        
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

void help() {
    std::cout << "usage: perf-snowcrash [options] ... <input file>" << std::endl << std::endl;
    std::cout << "API Blueprint Parser Performance Test Tool" << std::endl << std::endl;
    std::cout << "options:" << std::endl << std::endl;
    std::cout << "  -h, --help    display this help message" << std::endl;
    exit(0);
}

bool helpRequest(const std::string &arg) {
    return arg == "-h" || arg == "--help";
}

int main(int argc, const char *argv[])
{
    // FIXME: Intstrumetns helper
    //::sleep(20);

    if (argc != 2) {
        std::cerr << "one input file expected\n";
        exit(EXIT_FAILURE);
    }

    if (helpRequest(argv[1])) {
        help();
    }
    
    // Read fixture file
    std::ifstream inputFileStream;
    std::string inputFileName = argv[1];
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


