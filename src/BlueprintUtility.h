//
//  BlueprintUtility.h
//  snowcrash
//
//  Created by Zdenek Nemec on 7/11/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_BLUEPRINTUTILITY_H
#define SNOWCRASH_BLUEPRINTUTILITY_H

#include <utility>
#include <functional>
#include "Blueprint.h"
#include "HTTP.h"

namespace snowcrash {

    /**
     *  \brief Pair firsts matching predicate.
     *
     *  Two pairs are a match if their %first matches.
     */
    template <class T, class Predicate = std::equal_to<typename T::first_type> >
    struct MatchFirsts : std::binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const {
            return Predicate()(left.first, right.first);
        }
    };

    /**
     *  \brief Matches a pair's first against a value.
     */
    template <class T, class R = typename T::first_type, class Predicate = std::equal_to<R> >
    struct MatchFirstWith : std::binary_function<T, R, bool> {
        bool operator()(const T& left, const R& right) const {
            return Predicate()(left.first, right);
        }
    };

    /**  A name matching predicate. */
    template <class T>
    struct MatchName : std::binary_function<T, T, bool> {
        bool operator()(const T& first, const T& second) const {
            return first.name == second.name;
        }
    };

    /**
     *  \brief  Payload matching predicate.
     *
     *  Matches payloads if their name and media type matches.
     */
    struct MatchPayload : std::binary_function<Payload, Payload, bool> {
        bool operator()(const first_argument_type& left, const second_argument_type& right) const {


            if (left.name != right.name)
                return false;

            // Resolve left content type
            Collection<Header>::const_iterator header;

            header = std::find_if(left.headers.begin(),
                                  left.headers.end(),
                                  std::bind2nd(MatchFirstWith<Header, std::string>(),
                                               HTTPHeaderName::ContentType));

            std::string leftContentType;

            if (header != left.headers.end())
                leftContentType = header->second;

            // Resolve right content type
            header = std::find_if(right.headers.begin(),
                                  right.headers.end(),
                                  std::bind2nd(MatchFirstWith<Header, std::string>(),
                                               HTTPHeaderName::ContentType));

            std::string rightContentType;

            if (header != right.headers.end())
                rightContentType = header->second;

            return leftContentType == rightContentType;
        }
    };

    /** URI matching predicate. */
    struct MatchResource : std::binary_function<Resource, Resource, bool> {
        bool operator()(const first_argument_type& first, const second_argument_type& second) const {
            return first.uriTemplate == second.uriTemplate;
        }
    };

    /**  Action matching predicate. */
    struct MatchAction : std::binary_function<Action, Action, bool> {
        bool operator()(const first_argument_type& first, const second_argument_type& second) const {
            return first.uriTemplate == second.uriTemplate && first.method == second.method;
        }
    };

    /** Relation matching predicate. */
    struct MatchRelation : std::binary_function<Action, Relation, bool> {
        bool operator()(const first_argument_type& first, const second_argument_type& second) const {
            return !first.relation.str.empty() && !second.str.empty() && first.relation.str == second.str;
        }
    };
}

#endif
