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



#define URI_REGEX "^(http|https|ftp|file)?(://)?([^/]*)?(.*)$"
#define URI_TEMPLATE_EXPRESSION_REGEX "\\{(.*)\\}"

namespace snowcrash {
    
    /**
    *  URI Template Parser Result
    *  ------------------------------
    *
    */
    struct ParsedURITemplate{
        std::string scheme;
        std::string host;
        std::string path;
   
        std::vector<Error> errors;
        std::vector<Warning> warnings;
    };

    /**
    *  URI Template Character Parser
    *  ------------------------------
    *
    */
    class URITemplateCharacterParser{
    public:
        /**
        *  \brief Initialise the individual character parser
        *
        */
        virtual void initialiseParsing()=0;

        /**
        *  \brief Parse the URI Template a character at a time
        *
        *  \param c character to be parsed
        *  \param result result of the parsing
        *  \location block containing uri template being parsed
        *  \lastCharacter is this the last character in the uri being parsed
        */
        virtual void parse(const char c, ParsedURITemplate& result, const SourceCharactersBlock& location,bool lastCharacter) = 0;

        /**
        *  \brief destructor
        *
        */
        virtual ~URITemplateCharacterParser()=0;
    };


    /**
    *  URI Template Expression Parser
    *  ------------------------------
    *
    */
    class URITemplateExpressionParser{
    public:
        /**
        *  \brief Initialise the individual expression parser
        *
        */
        virtual void initialiseParsing()=0;

        /**
        *  \brief Parse each expression in the uri template
        *
        *  \param expression the uri template expression being parsed
        *  \param result result of the parsing
        *  \location block containing uri template being parsed
        */
        virtual void parse(const std::string expression, ParsedURITemplate& result, const SourceCharactersBlock& location) = 0;
        virtual ~URITemplateExpressionParser() = 0;
    };



    /**
    *  URI Template Parser Interface
    *  ------------------------------
    *
    */
    class URITemplateParser{
        static std::vector<URITemplateCharacterParser*> characterParsers;
        static std::vector<URITemplateExpressionParser*> expressionParsers;
        
        
	public:
        URITemplateParser();
		/**
		*  \brief Parse the URI template into scheme, host and path
		*
		*  \param uri        A uri to be parsed.
		*/
        static void parse(const URI uri, ParsedURITemplate& result, const SourceCharactersBlock& location);
        
	};

    class URITemplateCurlyBracketCharacterParser :public URITemplateCharacterParser{
        char lastBracket;
        bool alreadyWarned;
    public:
        void initialiseParsing();
        void parse(const char c, ParsedURITemplate& result, const SourceCharactersBlock& location,bool lastCharacter);
        ~URITemplateCurlyBracketCharacterParser();
    };

    class URITemplateSquareBracketCharacterParser :public URITemplateCharacterParser{
        char lastBracket;
        bool alreadyWarned;
    public:
        void initialiseParsing();
        void parse(const char c, ParsedURITemplate& result, const SourceCharactersBlock& location, bool lastCharacter);
        ~URITemplateSquareBracketCharacterParser();
    };

    class URITemplatePrefixSlashExpressionParser :public URITemplateExpressionParser{
        bool alreadyWarned;
    public:
        void initialiseParsing();
        void parse(const std::string expression, ParsedURITemplate& result, const SourceCharactersBlock& location);
        ~URITemplatePrefixSlashExpressionParser();
    };
}

#endif
