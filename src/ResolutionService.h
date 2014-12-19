//
//  ResolutionService.h
//  snowcrash
//
//  Created by Carl Griffiths on 18/11/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//
#ifndef SNOWCRASH_RESOLUTIONSERVICE_H
#define SNOWCRASH_RESOLUTIONSERVICE_H

#include "SourceAnnotation.h"


namespace snowcrash {

    /**
     *  \brief Base annotation resolver
     *  ------------------------------
     */
    class AnnotationResolver {
    public:
        virtual bool resolvesAnnotation(const SourceAnnotation& annotation) = 0;
        virtual void resolve(const mdp::ByteBuffer& source, SourceAnnotation& annotation) = 0;
        virtual ~AnnotationResolver() {
        };
    };

    /**
    *  \brief a set of annotation resolvers
    *  ------------------------------
    */
    typedef std::vector<AnnotationResolver*> AnnotationResolvers;

    /**
    *  \brief a service for providing resolutions for warnings and errors
    *  ------------------------------
    */
    class ResolutionService {
        AnnotationResolvers resolvers;
        ResolutionService();
        ~ResolutionService();
        ResolutionService(ResolutionService const&);
        void operator=(ResolutionService const&);
    public:
        static ResolutionService& getInstance();
        
        /**
        *  \brief Resolve the error and warnings for a report
        *
        *  \param report        A report with warnings and errors that may need to be resolved
        */
        void generateResolutions(const mdp::ByteBuffer& source, Report& report);

    };
}

#endif
