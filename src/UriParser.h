//
//  UriParser.h
//  snowcrash
//
//  Created by Carl Griffiths 24/02/2014.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_H
#define SNOWCRASH_H

#include<string>
#include "Blueprint.h"
#include "Parser.h"

/**
 *  URI Parser Interface
 *  ------------------------------
 *
 */

#define URI_REGEX "^(http|https|ftp|file)?(://)?([^/]*)?(.*)$"

namespace snowcrash {
    
    typedef enum URIParsingResult{
        OK=0,
        SquareBracketsInPath = 1,
        UnparsableURI = 2
    };
	


    struct URIResult{
        std::string scheme;
        std::string host;
        std::string path;
        URIParsingResult Result;
    };

    


	class URIParser{
        static bool ContainsString(std::string s,std::string in);
	public:
		/**
		*  \brief Parse the URI into scheme, host and path
		*
		*  \param uri        A uri to be parsed.
		*/
		void parse(const URI uri,URIResult& result);
	};

}

#endif
