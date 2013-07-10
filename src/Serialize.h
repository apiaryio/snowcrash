//
//  Serialize.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/3/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SERIALIZE_H
#define SNOWCRASH_SERIALIZE_H

#include <string>

namespace snowcrash {
    
    /**
     *  \brief  Escape new lines in input string.
     *  \param  input   A string to escape its new lines.
     *  \return A new string with lines escaped.
     *
     *  This function replaces all occurences of "\n" with "\\n".
     */
    std::string EscapeNewlines(const std::string& input);
    
    /**
     *  \brief  Escape every double quote in input string.
     *  \param  input   A string to escape its double quotes.
     *  \return A new string with double quotes escaped.
     */
    std::string EscapeDoubleQuotes(const std::string& input);
    
    /**
     *  AST entities serialization keys
     */
    struct SerializeKey {
        static const std::string Metadata;
        static const std::string Name;
        static const std::string Description;
        static const std::string ResourceGroups;
        static const std::string Resources;
        static const std::string URI;
        static const std::string URITemplate;
        static const std::string Methods;
        static const std::string Method;
        static const std::string Requests;
        static const std::string Responses;
        static const std::string Body;
        static const std::string Schema;
        static const std::string Headers;
        static const std::string Object;
    };
}

#endif
