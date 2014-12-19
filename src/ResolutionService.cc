//
//  ResolutionService.h
//  snowcrash
//
//  Created by Carl Griffiths on 18/11/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "ResolutionService.h"
#include "SourceAnnotation.h"
#include "UriTemplateResolvers.h"

using namespace snowcrash;

ResolutionService::ResolutionService() {
    resolvers.push_back(new SquareBracketWarningResolver());
    resolvers.push_back(new ContainsSpacesWarningResolver());
    resolvers.push_back(new InvalidCharactersWarningResolver());
}

ResolutionService::~ResolutionService() {
    for (AnnotationResolvers::reverse_iterator it = resolvers.rbegin(); it != resolvers.rend(); ++it) {
        delete(*it);
    }
}

ResolutionService& ResolutionService::getInstance() {
    static ResolutionService instance;
    return instance;
}

/**
 *  \brief Resolve an annotation creating one or more resolutions for an error/warning
 *
 *  \param annotation        An annotation to be resolved.
 */
void resolveAnnotation(const mdp::ByteBuffer& source, AnnotationResolvers& resolvers, SourceAnnotation& annotation) {
    for (AnnotationResolvers::iterator it = resolvers.begin(); it != resolvers.end(); ++it) {
        if ((*it)->resolvesAnnotation(annotation)) {
            (*it)->resolve(source, annotation);
            break;
        }
    }
}

/**
 *  \brief Generate resolutions for the parsed report
 *
 *  \param source
 *  \param report
 */
void ResolutionService::generateResolutions(const mdp::ByteBuffer& source, Report& report) {
    for (Warnings::iterator it = report.warnings.begin(); it != report.warnings.end(); ++it) {
        resolveAnnotation(source, resolvers, *it);
    }

    resolveAnnotation(source, resolvers, report.error);
}
