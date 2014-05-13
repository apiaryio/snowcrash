//
//  UriTemplateTeParser.h
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
#define URI_TEMPLATE_OPERATOR_REGEX "([+|#|.|/|;|?|&|=|,|!|@||])"
#define URI_TEMPLATE_EXPRESSION_REGEX "^([?|#|+]?(([A-Z|a-z|0-9|_|,])*|(%[A-F|a-f|0-9]{2})*)*\\*?)$"


namespace snowcrash {
    
    /*
    *  \brief URI template parse result.
    */
    struct ParsedURITemplate{
        std::string scheme;
        std::string host;
        std::string path;
   
        std::vector<Error> errors;
        Warnings warnings;
    };
    
    /*
    *  \brief URI template expression.
    */
    typedef std::string Expression;

    /*
    *  \brief collection for expressions for a single URI template.
    */
    typedef std::vector<Expression> Expressions;

    /*
    *  \brief iterator for collection of expressions.
    */
    typedef std::vector<Expression>::const_iterator ExpressionIterator;

    /*
    *  \brief base class for URI template expression once classified.
    */
    class ClassifiedExpression{
    public:
        ClassifiedExpression(){
            IsSupported = false;
            UnsupportedWarningText = "";
            Expression = "";
        }

        std::string UnsupportedWarningText;
        bool IsSupported;
        Expression Expression;
    };

    /*
    *  \brief unidentified URI template expression.
    */
    class UndefinedExpression :public ClassifiedExpression{
    public:
        UndefinedExpression(){
            UnsupportedWarningText = "Could not identify the URI template expression.";
        }
    };

    /*
    *  \brief level one basic variable expansion URI template expression.
    */
    class VariableExpression : public ClassifiedExpression{
    public:
        VariableExpression(){
            IsSupported = true;
        }
    };

    /*
    *  \brief level three query string expansion URI template expression.
    */
    class QueryStringExpression : public ClassifiedExpression{
    public:
        QueryStringExpression(){
            IsSupported =  true;
        }
    };

    /*
    *  \brief level two fragment expansion URI template expression.
    */
    class FragmentExpression : public ClassifiedExpression{
    public:
        FragmentExpression(){
            IsSupported = true;
        }
    };

    /*
    *  \brief level two reserved expansion URI template expression.
    */
    class ReservedExpansionExpression : public ClassifiedExpression{
    public:
        ReservedExpansionExpression(){
            IsSupported = true;
        }
    };

    /*
    *  \brief level three label expansion URI template expression.
    */
    class LabelExpansionExpression : public ClassifiedExpression{
    public:
        LabelExpansionExpression(){
            UnsupportedWarningText = "URI template label expansion is not supported.";
        }
    };

    /*
    *  \brief level three path segment expansion URI template expression.
    */
    class PathSegmentExpansionExpression : public ClassifiedExpression{
    public:
        PathSegmentExpansionExpression(){
            UnsupportedWarningText = "URI template path segment expansion is not supported.";
        }
    };

    /*
    *  \brief level three path style parameter expansion URI template expression.
    */
    class PathStyleParameterExpansionExpression : public ClassifiedExpression{
    public:
        PathStyleParameterExpansionExpression(){
            UnsupportedWarningText = "URI template path style parameter expansion is not supported.";
        }
    };
   
    /*
    *  \brief level three form style query continuation expansion URI template expression.
    */
    class FormStyleQueryContinuationExpression : public ClassifiedExpression{
    public:
        FormStyleQueryContinuationExpression(){
            UnsupportedWarningText = "URI template form style query continuation expansion is not supported.";
        }
    };

    /**
    *  URI Template Parser Interface
    *  ------------------------------
    */
    class URITemplateParser{
	public:
		/**
		*  \brief Parse the URI template into scheme, host and path and then parse for supported URI template expressions
		*
		*  \param uri        A uri to be parsed.
		*/
        static void parse(const URITemplate uri, ParsedURITemplate& result);
	};
}

#endif
