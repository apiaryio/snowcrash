//
//  UriTemplateParser.cc
//  snowcrash
//
//  Created by Carl Griffiths on 24/02/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//
#include <iomanip>
#include "UriTemplateParser.h"
#include "RegexMatch.h"

using namespace snowcrash;

static bool HasMismatchedCurlyBrackets(const URITemplate& uriTemplate) {
    int openCount = 0;
    int closeCount = 0;
    for (int i = 0; i < uriTemplate.length(); i++) {
        if (uriTemplate[i] == '{') openCount++;
        if (uriTemplate[i] == '}') closeCount++;
    }
    return openCount != closeCount;
}

static bool HasNestedCurlyBrackets(const URITemplate& uriTemplate) {
    char lastBracket = ' ';
    bool result = false;
    for (int i = 0; i < uriTemplate.length(); i++) {
        if (uriTemplate[i] == '{') {
            if (lastBracket == '{') {
                result = true;
                break;
            }
            lastBracket = '{';
        }
        if (uriTemplate[i] == '}') {
            if (lastBracket == '}') {
                result = true;
                break;
            }
            lastBracket = '}';
        }
    }
    return result;
}

static bool PathContainsSquareBrackets(const URITemplate& uriTemplate) {
    return (uriTemplate.find('[') != std::string::npos || uriTemplate.find(']') != std::string::npos);
}

static Expressions GetUriTemplateExpressions(const URITemplate& uriTemplate) {
    Expressions expressions;
    int expressionStartPos = 0;
    int expressionEndPos = 0;
    
    while (expressionStartPos != std::string::npos && expressionEndPos!=std::string::npos && expressionStartPos<uriTemplate.length()) {
        expressionStartPos = uriTemplate.find("{", expressionStartPos);
        expressionEndPos = uriTemplate.find("}", expressionStartPos);
        if (expressionStartPos != std::string::npos && expressionEndPos > expressionStartPos) {
            expressions.push_back(uriTemplate.substr(expressionStartPos + 1, (expressionEndPos - expressionStartPos) - 1));
        }
        expressionStartPos++;
    }
    return expressions;
}

FORCEINLINE bool IsAVariableExpression(const Expression& expression) {
    return !RegexMatch(expression.substr(0, 1), URI_TEMPLATE_OPERATOR_REGEX);
}

FORCEINLINE bool IsAQueryStringExpression(const Expression& expression) {
    return expression.substr(0, 1) == "?";
}

FORCEINLINE bool IsAFragmentExpression(const Expression& expression) {
    return expression.substr(0, 1) == "#";
}

FORCEINLINE bool IsAReservedExpansionExpression(const Expression& expression) {
    return expression.substr(0, 1) == "+";
}

FORCEINLINE bool IsALabelExpansionExpression(const Expression& expression) {
    return expression.substr(0, 1) == ".";
}

FORCEINLINE bool IsAPathSegmentExpansionExpression(const Expression& expression) {
    return expression.substr(0, 1) == "/";
}

FORCEINLINE bool IsAPathStyleParameterExpansionExpression(const Expression& expression) {
    return expression.substr(0, 1) == ";";
}

FORCEINLINE bool IsAFormStyleQueryContinuationExpansionExpression(const Expression& expression) {
    return expression.substr(0, 1) == "&";
}

static ClassifiedExpression ClassifyExpression(const Expression expression) {
    if (IsAVariableExpression(expression)) {
        VariableExpression classifiedExpression;
        classifiedExpression.expression = expression;
        return classifiedExpression;
    }

    if (IsAQueryStringExpression(expression)) {
        QueryStringExpression classifiedExpression;
        classifiedExpression.expression = expression;
        return classifiedExpression;
    }

    if (IsAFragmentExpression(expression)) {
        FragmentExpression classifiedExpression;
        classifiedExpression.expression = expression;
        return classifiedExpression;
    }

    if (IsAReservedExpansionExpression(expression)) {
        ReservedExpansionExpression classifiedExpression;
        classifiedExpression.expression = expression;
        return classifiedExpression;
    }

    if (IsALabelExpansionExpression(expression)) {
        LabelExpansionExpression classifiedExpression;
        classifiedExpression.expression = expression;
        return classifiedExpression;
    }

    if (IsAPathSegmentExpansionExpression(expression)) {
        PathSegmentExpansionExpression classifiedExpression;
        classifiedExpression.expression = expression;
        return classifiedExpression;
    }

    if (IsAPathStyleParameterExpansionExpression(expression)) {
        PathStyleParameterExpansionExpression classifiedExpression;
        classifiedExpression.expression = expression;
        return classifiedExpression;
    }

    if (IsAFormStyleQueryContinuationExpansionExpression(expression)) {
        FormStyleQueryContinuationExpression classifiedExpression;
        classifiedExpression.expression = expression;
        return classifiedExpression;
    }

    UndefinedExpression classifiedExpression;
    classifiedExpression.expression = expression;
    return classifiedExpression;
}

FORCEINLINE bool IsSupportedExpressionType(const ClassifiedExpression& expression) {
    return expression.isSupported;
}

FORCEINLINE bool ExpressionContainsSpaces(const ClassifiedExpression& expression) {
    return expression.expression.find(" ") != std::string::npos;
}

FORCEINLINE bool ExpressionContainsHyphens(const ClassifiedExpression& expression) {
    return expression.expression.find("-") != std::string::npos;
}

FORCEINLINE bool ExpressionContainsAssignment(const ClassifiedExpression& expression) {
    return expression.expression.find("=") != std::string::npos;
}
bool IsInvalidExpressionName(const ClassifiedExpression& expression) {
    return !RegexMatch(expression.expression,URI_TEMPLATE_EXPRESSION_REGEX);
}

void URITemplateParser::parse(const URITemplate uri, const SourceCharactersBlock& sourceBlock, ParsedURITemplate& result)
{
    CaptureGroups groups;
    Expressions expressions;
    size_t gSize=5;

    if (uri.empty()) return;

    if (RegexCapture(uri, URI_REGEX, groups, gSize)) {
        result.scheme = groups[1];
        result.host = groups[3];
        result.path = groups[4];
        
        if (HasMismatchedCurlyBrackets(result.path)) {
            result.result.warnings.push_back(Warning("The URI template contains mismatched expression brackets.", URIWarning, sourceBlock));
            return;
        }
       
        if (HasNestedCurlyBrackets(result.path)) {
            result.result.warnings.push_back(Warning("The URI template contains nested expression brackets.", URIWarning, sourceBlock));
            return;
        }

        if (PathContainsSquareBrackets(result.path)) {
            result.result.warnings.push_back(Warning("The URI template contains square brackets, please percent encode square brackets as %5B and %5D.", URIWarning, sourceBlock));
        }

        expressions = GetUriTemplateExpressions(result.path);

        ExpressionIterator currentExpression = expressions.begin();

        while (currentExpression != expressions.end()) {

            ClassifiedExpression classifiedExpression = ClassifyExpression(*currentExpression);
            
            if (IsSupportedExpressionType(classifiedExpression)) {
                bool hasIllegalCharacters = false;

                if (ExpressionContainsSpaces(classifiedExpression)) {
                    std::stringstream ss;
                    ss << "URI template expression \"" << classifiedExpression.expression << "\" contains spaces. Allowed characters for expressions are A-Z a-z 0-9 _ and percent encoded characters.";
                    result.result.warnings.push_back(Warning(ss.str(), URIWarning, sourceBlock));
                    hasIllegalCharacters = true;
                }

                if (ExpressionContainsHyphens(classifiedExpression)) {
                    std::stringstream ss;
                    ss << "URI template expression \"" << classifiedExpression.expression << "\" contains hyphens. Allowed characters for expressions are A-Z a-z 0-9 _ and percent encoded characters.";
                    result.result.warnings.push_back(Warning(ss.str(), URIWarning, sourceBlock));
                    hasIllegalCharacters = true;
                }

                if (ExpressionContainsAssignment(classifiedExpression)) {
                    std::stringstream ss;
                    ss << "URI template expression \"" << classifiedExpression.expression << "\" contains assignment. Allowed characters for expressions are A-Z a-z 0-9 _ and percent encoded characters.";
                    result.result.warnings.push_back(Warning(ss.str(), URIWarning, sourceBlock));
                    hasIllegalCharacters = true;
                }
               
                if (!hasIllegalCharacters) {
                    if (IsInvalidExpressionName(classifiedExpression)) {
                        std::stringstream ss;
                        ss << "URI template expression \"" << classifiedExpression.expression << "\" contains invalid characters. Allowed characters for expressions are A-Z a-z 0-9 _ and percent encoded characters.";
                        result.result.warnings.push_back(Warning(ss.str(), URIWarning, sourceBlock));
                    }
                }
            }
            else{
                result.result.warnings.push_back(Warning(classifiedExpression.unsupportedWarningText, URIWarning, sourceBlock));
            }
            currentExpression++;
        }
    }
    else{
        result.result.error = Error("Failed to parse URI Template", URIWarning);
    }
}








