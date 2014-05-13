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
    template <class T>
    struct MatchFirsts : std::binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const {
            return left.first == right.first;
        }
    };
    
    /**
     *  \brief Matches a pair's first against a value.
     */
    template <class T, class R>
    struct MatchFirstWith : std::binary_function<T, R, bool> {
        bool operator()(const T& left, const R& right) const {
            return left.first == right;
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
    template <class T>
    struct MatchAction : std::binary_function<T, T, bool> {
        bool operator()(const T& first, const T& second) const {
            return first.method == second.method;
        }
    };
    
    /**
     *  \brief  Find a request within given action.
     *  \param  transaction  A transaction to check.
     *  \param  request A request to look for.
     *  \return Iterator pointing to the matching request within given method requests.
     */
    FORCEINLINE Collection<Request>::const_iterator FindRequest(const TransactionExample& example, const Request& request) {
        return std::find_if(example.requests.begin(),
                            example.requests.end(),
                            std::bind2nd(MatchPayload(), request));
    }

    /**
     *  \brief  Find a response within responses of a given action.
     *  \param  transaction  A transaction to check.
     *  \param  response A response to look for.
     *  \return Iterator pointing to the matching response within given method requests.
     */
    FORCEINLINE Collection<Response>::const_iterator FindResponse(const TransactionExample& example, const Response& response) {
        return std::find_if(example.responses.begin(),
                            example.responses.end(),
                            std::bind2nd(MatchPayload(), response));
    }
}

#endif
