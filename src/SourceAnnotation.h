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
#include "ByteBuffer.h"

namespace snowcrash {

    /**
     *  \brief  A resolution annotation.
     *
     *  A resolution to a source annotation describing how to fix an issue described by the source annotation.
     *  Includes a location within the source data, the resolved source data and an optional message.
     */
    struct ResolutionAnnotation {

        /**
         *  \brief  %ResolutionAnnotation default constructor.
         *
         *  Creates an empty resolution annotation.
         */
        ResolutionAnnotation() {}

        /**
         *  \brief  %ResolutionAnnotation copy constructor.
         *  \param  rhs  An annotation to be copied.
         */
        ResolutionAnnotation(const ResolutionAnnotation& rhs) {

            this->message = rhs.message;
            this->location = rhs.location;
            this->resolvedSource = rhs.resolvedSource;
        }

        /**
         *  \brief  %ResolutionAnnotation constructor.
         *  \param  message     An annotation message.
         *  \param  resolvedSource   The resolved source data.
         *  \param  location    A location of the annotation.
         */
        ResolutionAnnotation(const std::string& message,
            const mdp::ByteBuffer&  resolvedSource = mdp::ByteBuffer(),
            const mdp::BytesRange& location = mdp::BytesRange()) {

            this->message = message;

            this->resolvedSource.clear();
            if (!resolvedSource.empty())
                this->resolvedSource.assign(resolvedSource.begin(), resolvedSource.end());

            this->location = location;
        }

        /** \brief  %ResolutionAnnotation destructor. */
        ~ResolutionAnnotation() {}

        /**
         *  \brief  %ResolutionAnnotation assignment operator
         *  \param  rhs  A resolution annotation to be assigned to this annotation.
         */
        ResolutionAnnotation& operator=(const ResolutionAnnotation& rhs) {
            this->message = rhs.message;
            this->location = rhs.location;
            this->resolvedSource = rhs.resolvedSource;
            return *this;
        }

        /** The location of this resolution within the source data buffer. */
        mdp::BytesRange location;

        /** The source data amended to resolve the issues described in the message */
        mdp::ByteBuffer resolvedSource;

        /** An annotation message. */
        std::string message;
    };

    /**
     *  A set of resolution source annotations.
     */
    typedef std::vector<ResolutionAnnotation> Resolutions;

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
        SourceAnnotation() : code(OK), subCode(OK) {}

        /**
         *  \brief  %SourceAnnotation copy constructor.
         *  \param  rhs  An annotation to be copied.
         */
        SourceAnnotation(const SourceAnnotation& rhs) : resolutions(rhs.resolutions) {
            this->message = rhs.message;
            this->code = rhs.code;
            this->subCode = rhs.subCode;
            this->location = rhs.location;
        }

        /**
         *  \brief  %SourceAnnotation constructor.
         *  \param  message     An annotation message.
         *  \param  code        Annotation code.
         *  \param  subCode     Annotation subCode.
         *  \param  location    A location of the annotation.
         */
        SourceAnnotation(const std::string& message,
            int code,
            const mdp::CharactersRangeSet& location) {

            this->message = message;
            this->code = code;
            this->subCode = OK;
            this->location.clear();
            if (!location.empty())
                this->location.assign(location.begin(), location.end());
        }

        /**
         *  \brief  %SourceAnnotation constructor.
         *  \param  message     An annotation message.
         *  \param  code        Annotation code.
         *  \param  subCode        Annotation subCode.
         *  \param  location    A location of the annotation.
         */
        SourceAnnotation(const std::string& message,
            int code = OK,
            int subCode = OK,
            const mdp::CharactersRangeSet& location = mdp::CharactersRangeSet()) {

            this->message = message;
            this->code = code;
            this->subCode = subCode;
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
            this->resolutions = rhs.resolutions;
            return *this;
        }

        /** The location of this annotation within the source data buffer. */
        mdp::CharactersRangeSet location;

        /** An annotation code. */
        int code;

        /** An annotation subCode. */
        int subCode;

        /** An annotation message. */
        std::string message;

        /** Resolutions to the issue described in the annotation */
        Resolutions resolutions;
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
        ModelError = 3
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
        URIWarning = 12,
        HTTPWarning = 13
    };

    /**
     * UriTemplate warning subCodes
     */
    enum UriTemplateWarningSubCode {
        NoUriTemplateWarningSubCode = 0,
        SquareBracketWarningUriTemplateWarningSubCode = 1,
        NestedCurlyBracketsWarningUriTemplateWarningSubCode = 2,
        MismatchedCurlyBracketsWarningUriTemplateWarningSubCode = 3,
        ContainsSpacesWarningUriTemplateWarningSubCode = 4,
        ContainsHyphensWarningUriTemplateWarningSubCode = 5,
        ContainsAssignmentWarningUriTemplateWarningSubCode = 6,
        InvalidCharactersWarningUriTemplateWarningSubCode = 7,
        UnsupportedExpressionWarningUriTemplateWarningSubCode = 8
    };

    /**
     *  A set of warning source annotations.
     */
    typedef std::vector<Warning> Warnings;

    /**
     *  \brief A parsing report Report.
     *
     *  Result of a source data parsing operation.
     *  Composed of ONE error source annotation
     *  and a set of warning source annotations.
     */
    struct Report {

        /**
         *  \brief Append a report to this one, replacing the error source annotation.
         *
         *  NOTE: A binding does not need to wrap this action.
         */
        Report& operator+=(const Report& rhs) {
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
