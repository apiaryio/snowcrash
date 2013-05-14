//
//  ParserCore.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_PARSERCORE_H
#define SNOWCRASH_PARSERCORE_H

#include <string>
#include <vector>

namespace snowcrash {
    
    //
    // Source (string) data
    //
    typedef std::string SourceData;
    
    //
    // Interval of Source data
    //
    struct SourceDataRange {
        size_t location;
        size_t length;
    };
    
    //
    // Block of Source Data, set of intervals
    //
    typedef std::vector<SourceDataRange> SourceDataBlock;
    
    // Create SourceDataBlock 
    SourceDataBlock MakeSourceDataBlock(size_t loc, size_t len);
    
    // Append SourceDataBlock to existign blog, merging continuous blocks
    void AppendSourceDataBlock(SourceDataBlock& destination, const SourceDataBlock& append);
    
    // Split source data block at len
    typedef std::pair<SourceDataBlock, SourceDataBlock> SourceDataBlockPair;
    SourceDataBlockPair SplitSourceDataBlock(const SourceDataBlock& block, size_t len);
    
    //
    // Source module line anotation
    //
    struct SourceAnnotation {
        
        static const int OK = 0;
        
        SourceAnnotation() : code(OK) {}
        
        SourceAnnotation(const SourceAnnotation& rhs) {

            this->message = rhs.message;
            this->code = rhs.code;
            this->location = rhs.location;
        }
        
        SourceAnnotation(const std::string& message,
                         int code = OK,
                         const SourceDataBlock& location = SourceDataBlock()) {

            this->message = message;
            this->code = code; 
            this->location =
            location; }
        
        ~SourceAnnotation() {}
        
        SourceAnnotation& operator=(const SourceAnnotation& rhs) {
            this->message = rhs.message;
            this->code = rhs.code;
            this->location = rhs.location;
            return *this;
        }
        
        // Location of this annotation
        SourceDataBlock location;
        
        // Annotation code
        int code;
        
        // Annotation message
        std::string message;
    };
    
    typedef SourceAnnotation Error;
    typedef SourceAnnotation Warning;
    typedef std::vector<Warning> Warnings;
    
    //
    // Module parsing report
    //
    struct Result {
        
        // Append another result to this one, replacing error.
        Result& operator+=(const Result& rhs) {
            error = rhs.error;
            warnings.insert(warnings.end(), rhs.warnings.begin(), rhs.warnings.end());
            return *this;
        }
        
        Error error;
        Warnings warnings;
    };
}

#endif
