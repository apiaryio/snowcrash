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
#define URI_TEMPLATE_EXPRESSION_REGEX "^([?|#|+|&]?(([A-Z|a-z|0-9|_|,])*|(%[A-F|a-f|0-9]{2})*)*\\*?)$"


namespace snowcrash {
    
    /**
    *  \brief URI template parse result.
    */
    struct ParsedURITemplate {
        std::string scheme;
        std::string host;
        std::string path;
   
        Result result;
    };
    
    /**
    *  \brief URI template expression.
    */
    typedef std::string Expression;

    /**
    *  \brief collection for expressions for a single URI template.
    */
    typedef std::vector<Expression> Expressions;

    /**
    *  \brief iterator for collection of expressions.
    */
    typedef std::vector<Expression>::const_iterator ExpressionIterator;

    /**
    *  \brief base class for URI template expression once classified.
    */
    class ClassifiedExpression {
    protected :
        bool isSupported;
    public :
        ClassifiedExpression(std::string expression) {
            isSupported = false;
            unsupportedWarningText = "";
            innerExpression = expression;
        }

        std::string unsupportedWarningText;
        
        snowcrash::Expression innerExpression;
        
        virtual bool IsExpressionType(){
            return false;
        }
                
        FORCEINLINE bool ContainsSpaces() {
            return innerExpression.find(" ") != std::string::npos;
        }

        FORCEINLINE bool ContainsAssignment() {
            return innerExpression.find("=") != std::string::npos;
        }

        FORCEINLINE bool ContainsHyphens() {
            return innerExpression.find("-") != std::string::npos;
        }

        FORCEINLINE bool IsInvalidExpressionName() {
            std::string tmpExpression = innerExpression;
            if (tmpExpression.find("..") != std::string::npos) return true;
            
            size_t start_pos = 0;
            while ((start_pos = tmpExpression.find(".", start_pos)) != std::string::npos) {
                tmpExpression.replace(start_pos, 1, "_");
                start_pos++; 
            }

            return !RegexMatch(tmpExpression, URI_TEMPLATE_EXPRESSION_REGEX);
        }

        FORCEINLINE bool IsSupportedExpressionType() {
            return isSupported;
        }
    };

    /**
    *  \brief level one basic variable expansion URI template expression.
    */
    class VariableExpression : public ClassifiedExpression {
    public :
        VariableExpression(std::string expression):ClassifiedExpression(expression) {
            isSupported = true;
        }

        bool IsExpressionType(){
            return !RegexMatch(innerExpression.substr(0, 1), URI_TEMPLATE_OPERATOR_REGEX);
        }
    };

    /**
    *  \brief level three query string expansion URI template expression.
    */
    class QueryStringExpression : public ClassifiedExpression {
    public :
        QueryStringExpression(std::string expression):ClassifiedExpression(expression) {
            isSupported =  true;
        }

        bool IsExpressionType(){
            return innerExpression.substr(0, 1) == "?";
        }
    };

    /**
    *  \brief level two fragment expansion URI template expression.
    */
    class FragmentExpression : public ClassifiedExpression {
    public :
        FragmentExpression(std::string expression):ClassifiedExpression(expression) {
            isSupported = true;
        }

        bool IsExpressionType() {
            return innerExpression.substr(0, 1) == "#";
        }
    };

    /**
    *  \brief level two reserved expansion URI template expression.
    */
    class ReservedExpansionExpression : public ClassifiedExpression {
    public :
        ReservedExpansionExpression(std::string expression):ClassifiedExpression(expression) {
            isSupported = true;
        }

        bool IsExpressionType() {
            return innerExpression.substr(0, 1) == "+";
        }
    };

    /**
    *  \brief level three label expansion URI template expression.
    */
    class LabelExpansionExpression : public ClassifiedExpression {
    public :
        LabelExpansionExpression(std::string expression):ClassifiedExpression(expression) {
            unsupportedWarningText = "URI template label expansion is not supported.";
        }

        bool IsExpressionType() {
            return innerExpression.substr(0, 1) == ".";
        }
    };

    /**
    *  \brief level three path segment expansion URI template expression.
    */
    class PathSegmentExpansionExpression : public ClassifiedExpression {
    public :
        PathSegmentExpansionExpression(std::string expression):ClassifiedExpression(expression) {
            unsupportedWarningText = "URI template path segment expansion is not supported.";
        }

        bool IsExpressionType() {
            return innerExpression.substr(0, 1) == "/";
        }
    };

    /**
    *  \brief level three path style parameter expansion URI template expression.
    */
    class PathStyleParameterExpansionExpression : public ClassifiedExpression {
    public :
        PathStyleParameterExpansionExpression(std::string expression):ClassifiedExpression(expression) {
            unsupportedWarningText = "URI template path style parameter expansion is not supported.";
        }

        bool IsExpressionType() {
            return innerExpression.substr(0, 1) == ";";
        }
    };
   
    /**
    *  \brief level three form style query continuation expansion URI template expression.
    */
    class FormStyleQueryContinuationExpression : public ClassifiedExpression {
    public :
        FormStyleQueryContinuationExpression(std::string expression):ClassifiedExpression(expression) {
            isSupported = true;
        }

        bool IsExpressionType() {
            return innerExpression.substr(0, 1) == "&";
        }
    };

    /**
    *  \brief undefined URI template expression.
    */
    class UndefinedExpression : public ClassifiedExpression{
    public :
        UndefinedExpression(std::string expression) :ClassifiedExpression(expression) {
            unsupportedWarningText = "Unidentified expression.";
        }

        bool IsExpressionType() {
            return false;
        }
    };

    /**
    *  URI Template Parser Interface
    *  ------------------------------
    */
    class URITemplateParser{
	public :
		/**
		*  \brief Parse the URI template into scheme, host and path and then parse for supported URI template expressions
		*
		*  \param uri        A uri to be parsed.
		*/
        static void parse(const URITemplate uri, const SourceCharactersBlock& sourceBlock, ParsedURITemplate& result);
	};
}

#endif
