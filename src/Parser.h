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

        SourceAnnotation(const SourceAnnotation&) = default;
        
        SourceAnnotation(SourceAnnotation&&) = default;
        
        SourceAnnotation(const std::string& message, int code = OK, const SourceDataBlock& location = SourceDataBlock())
        : SourceAnnotation() { this->message = message; this->code = code; this->location = location; }
        
        ~SourceAnnotation() = default;

        SourceAnnotation& operator=(const SourceAnnotation&) = default;
        
        // Location of this annotation
        SourceDataBlock location;
        
        // Annotation code
        int code;
        
        // Annotation message
        std::string message;
    };
    
    using Error = SourceAnnotation;
    using Warning = SourceAnnotation;
    using Warnigns = std::vector<Warning>;
    
    //
    // Module parsing report
    //
    struct Result {

        // Append another result to this one, replacing error.
        Result& operator+=(const Result& rhs) {
            error = rhs.error;
            warnings.insert(std::end(warnings), std::begin(rhs.warnings), std::end(rhs.warnings));
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
        using ParseHandler = std::function<void(const Result&, const Blueprint&)>;
        
        // Parse source data into Blueprint AST
        void parse(const SourceData& source, const ParseHandler& callback);
    };
}

#endif
