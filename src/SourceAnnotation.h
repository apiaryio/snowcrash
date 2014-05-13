//
//  SourceAnnotation.h
//  snowcrash
//
//  Created by Zdenek Nemec on 7/12/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SOURCEANNOTATION_H
#define SNOWCRASH_SOURCEANNOTATION_H

#include <string>
#include <vector>

namespace snowcrash {
    
    /**
     *  \brief  A character range within the source code string.
     */
    struct SourceCharactersRange {
        size_t location;
        size_t length;
    };
    
    /**
     *  \brief  A block (set of ranges) of source code characters. A character map.
     *
     *  NOTE: The block does not have to be continuous.
     */
    typedef std::vector<SourceCharactersRange> SourceCharactersBlock;
    
    /**
     *  \brief  A source data annotation.
     *
     *  Annotation bound to a source data block. Includes an
     *  annotation code and an optional message.
     */
    struct SourceAnnotation {
        
        /**
         *  \brief Default annotation code representing success.
         */
        static const int OK;
        
        /**
         *  \brief  %SourceAnnotation default constructor.
         *
         *  Creates an empty annotation with the default annotation code.
         */
        SourceAnnotation() : code(OK) {}
        
        /**
         *  \brief  %SourceAnnotation copy constructor.
         *  \param  rhs  An annotation to be copied.
         */
        SourceAnnotation(const SourceAnnotation& rhs) {
            
            this->message = rhs.message;
            this->code = rhs.code;
            this->location = rhs.location;
        }
        
        /**
         *  \brief  %SourceAnnotation constructor.
         *  \param  message     An annotation message.
         *  \param  code        Annotation code.
         *  \param  location    A location of the annotation.
         */
        SourceAnnotation(const std::string& message,
                         int code = OK,
                         const SourceCharactersBlock& location = SourceCharactersBlock()) {
            
            this->message = message;
            this->code = code;
            
            this->location.clear();
            if (!location.empty())
                this->location.assign(location.begin(), location.end());
        }
        
        /** \brief  %SourceAnnotation destructor. */
        ~SourceAnnotation() {}
        
        /**
         *  \brief  %SourceAnnotation assignment operator
         *  \param  rhs  An annotation to be assigned to this annotation.
         */
        SourceAnnotation& operator=(const SourceAnnotation& rhs) {
            this->message = rhs.message;
            this->code = rhs.code;
            this->location = rhs.location;
            return *this;
        }
        
        /** The location of this annotation within the source data buffer. */
        SourceCharactersBlock location;
        
        /** An annotation code. */
        int code;
        
        /** A annotation message. */
        std::string message;
    };
    
    /**
     *  Error source annotation.
     */
    typedef SourceAnnotation Error;
    
    /**
     *  Error codes
     */
    enum ErrorCode {
        NoError = 0,
        ApplicationError = 1,
        BusinessError = 2,
        SymbolError = 3
    };
    
    /**
     *  Warning source annotation.
     */
    typedef SourceAnnotation Warning;
    
    /**
     *  Warning codes
     */
    enum WarningCode {
        NoWarning = 0,
        APINameWarning = 1,
        DuplicateWarning = 2,
        FormattingWarning = 3,
        RedefinitionWarning = 4,
        IgnoringWarning = 5,
        EmptyDefinitionWarning = 6,
        NotEmptyDefinitionWarning = 7,
        LogicalErrorWarning = 8,
        DeprecatedWarning = 9,
        IndentationWarning = 10,
        AmbiguityWarning = 11,
        URIWarning = 12
    };
    
    /**
     *  A set of warning source annotations.
     */
    typedef std::vector<Warning> Warnings;
    
    /**
     *  \brief A parsing result Report.
     *
     *  Result of a source data parsing operation.
     *  Composed of ONE error source annotation
     *  and a set of warning source annotations.
     */
    struct Result {
        
        /**
         *  \brief Append a result to this one, replacing the error source annotation.
         *
         *  NOTE: A binding does not need to wrap this action.
         */
        Result& operator+=(const Result& rhs) {
            error = rhs.error;
            warnings.insert(warnings.end(), rhs.warnings.begin(), rhs.warnings.end());
            return *this;
        }
        
        /** Result error source annotation */
        Error error;
        
        /** Result warning source annotations */
        Warnings warnings;
    };
}

#endif
