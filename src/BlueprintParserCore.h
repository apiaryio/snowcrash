//
//  BlueprintParserCore.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_BLUEPRINTPARSERCORE_H
#define SNOWCRASH_BLUEPRINTPARSERCORE_H

#include "ParserCore.h"
#include "MarkdownBlock.h"

// Recognized HTTP headers, regex string
#define HTTP_METHODS "GET|POST|PUT|DELETE|OPTIONS|PATCH|PROPPATCH|LOCK|UNLOCK|COPY|MOVE|MKCOL|HEAD"

namespace snowcrash {
    
    // Block Classification Section
    enum Section {
        UndefinedSection,
        BlueprintSection,
        ResourceGroupSection,
        ResourceSection,
        MethodSection,
        TerminatorSection
    };
    
    // Parser iterator
    typedef MarkdownBlock::Stack::const_iterator BlockIterator;
    
    // Parsing sub routine result
    typedef std::pair<Result, BlockIterator> ParseSectionResult;
    
    // Name matching predicate
    template <class T>
    struct MatchName : std::binary_function<T, T, bool> {
        bool operator()(const T& first, const T& second) const {
            return first.name == second.name;
        }
    };
    
    // URI matching predicate
    template <class T>
    struct MatchURI : std::binary_function<T, T, bool> {
        bool operator()(const T& first, const T& second) const {
            return first.uri == second.uri;
        }
    };
    
    // Method matching predicate
    template <class T>
    struct MatchMethod : std::binary_function<T, T, bool> {
        bool operator()(const T& first, const T& second) const {
            return first.method == second.method;
        }
    };
}

#endif
