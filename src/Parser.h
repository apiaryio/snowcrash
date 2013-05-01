//
//  Parser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/8/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_PARSER_H
#define SNOWCRASH_PARSER_H

#include <functional>
#include "Blueprint.h"
#include "MarkdownBlock.h"

namespace snowcrash {
    
    //
    // Source module line anotation
    //
    struct SourceAnnotation {
        
        static const int OK = 0;

        SourceAnnotation()
        : code(OK) {}

        SourceAnnotation(const SourceAnnotation& rhs)
        { this->message = rhs.message; this->code = rhs.code; this->location = rhs.location; }
        
        SourceAnnotation(const std::string& message, int code = OK, const SourceDataBlock& location = SourceDataBlock())
        { this->message = message; this->code = code; this->location = location; }
        
        ~SourceAnnotation() {}

        SourceAnnotation& operator=(const SourceAnnotation& rhs)
        { this->message = rhs.message; this->code = rhs.code; this->location = rhs.location; return *this; }
        
        // Location of this annotation
        SourceDataBlock location;
        
        // Annotation code
        int code;
        
        // Annotation message
        std::string message;
    };
    
    typedef SourceAnnotation Error;
    typedef SourceAnnotation Warning;
    typedef std::vector<Warning> Warnigns;
    
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
        Warnigns warnings;
    };
        
    //
    // API Blueprint Parser
    //
    class Parser {
    public:
        // using ParseHandler = std::function<void(const Result&, const Blueprint&)>;
        
        // Parse source data into Blueprint AST
        void parse(const SourceData& source, Result& result, Blueprint& blueprint);
    };
}

#endif
