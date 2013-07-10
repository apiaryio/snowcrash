//
//  ParserCore.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_PARSERCORE_H
#define SNOWCRASH_PARSERCORE_H

#include <string>
#include <vector>
#include "Platform.h"

namespace snowcrash {
    
    /**
     *  \brief  Textual source data. A markdown-formatted text.
     */
    typedef std::string SourceData;
    
    /**
     *  \brief  A range of data within the source data buffer.
     */
    struct SourceDataRange {
        size_t location;
        size_t length;
    };

    /**
     *  \brief  A block of source data.
     * 
     *  NOTE: The block does not have to be continuous.
     */
    typedef std::vector<SourceDataRange> SourceDataBlock;
    
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
         *  Creates an empty anotation with the default annotation code.
         */
        SourceAnnotation() : code(OK) {}
        
        /** 
         *  \brief  %SourceAnnotation copy constructor.
         *  \param  rhs  An anotation to be copied.
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
                         const SourceDataBlock& location = SourceDataBlock()) {

            this->message = message;
            this->code = code; 
            this->location = location;
        }
        
        /** \brief  %SourceAnnotation destructor. */
        ~SourceAnnotation() {}
        
        /** 
         *  \brief  %SourceAnnotation assignment operator
         *  \param  rhs  An anotation to be assigned to this annotation.
         */
        SourceAnnotation& operator=(const SourceAnnotation& rhs) {
            this->message = rhs.message;
            this->code = rhs.code;
            this->location = rhs.location;
            return *this;
        }
        
        /** The location of this annotation within the source data buffer. */
        SourceDataBlock location;
        
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
     *  Warning source annotation.
     */
    typedef SourceAnnotation Warning;
    
    /**
     *  A set of warning source annotations.
     */
    typedef std::vector<Warning> Warnings;
    
    /**
     *  \brief A parsing result Report.
     *
     *  Resulte of a source data parsing operation. 
     *  Composed of ONE error source annotation 
     *  and a set of warning source annotations.
     */
    struct Result {
        
        /** 
         *  \brief Append a result to this one, replacing the error source annotation.
         *
         *  NOTE: A binding does not need to wrap this method.
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
    
    /**
     *  \brief  Create a %SourceDataBlock with a range.
     *  \param  loc     A location in source data buffer.
     *  \param  len     Length of the range.
     *
     *  NOTE: A binding does not need to wrap this function.
     */
    SourceDataBlock MakeSourceDataBlock(size_t loc, size_t len);
    
    // Append SourceDataBlock to existign blog, merging continuous blocks

    /**
     *  \brief  Append %SourceDataBlock to existign block, merging continuous blocks.
     *  \param  destination A block to append to.
     *  \param  append      A block to be appended.
     *
     *  NOTE: A binding does not need to wrap this function.
     */
    void AppendSourceDataBlock(SourceDataBlock& destination, const SourceDataBlock& append);

    
    /**
     *  \brief A generic pair of two blocks
     *
     *  NOTE: A binding does not need to wrap this data type.
     */
    typedef std::pair<SourceDataBlock, SourceDataBlock> SourceDataBlockPair;
    
    /**
     *  \brief  Splits %SourceDataBlock into two blocks.
     *  \param  block   A block to be split.
     *  \param  len     The length of first block after which the split occurs.
     *
     *  NOTE: A binding does not need to wrap this function.
     */
    SourceDataBlockPair SplitSourceDataBlock(const SourceDataBlock& block, size_t len);
}

#endif
