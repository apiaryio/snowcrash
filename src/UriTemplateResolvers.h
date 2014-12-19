//
//  UriTemplateResolvers.h
//  snowcrash
//
//  Created by Carl Griffiths on 21/11/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//
#include <algorithm>
#include "SourceAnnotation.h"
#include "ResolutionService.h"
#include "RegexMatch.h"
#include "UriTemplateParser.h"

#define URI_TEMPLATE_VALID_CHARACTERS_WITH_EXPRESSION_TYPES_REGEX "([?|#|+|&|A-Z|a-z|0-9|_|,])"
#define URI_TEMPLATE_VALID_CHARACTERS_REGEX "([A-Z|a-z|0-9|_|,])"

namespace snowcrash {

    /**
    *  \brief resolver for square bracket warnings
    *   produces a resolution for each square bracket containing the message to percent encode the bracket, the location of the bracket and the appropriate percent encoding
    *  ------------------------------
    */
    class SquareBracketWarningResolver : public AnnotationResolver {
    public:
        virtual bool resolvesAnnotation(const SourceAnnotation& annotation) {
            return ((annotation.code == URIWarning) && (annotation.subCode == SquareBracketWarningUriTemplateWarningSubCode));
        }

        virtual void resolve(const mdp::ByteBuffer& source, SourceAnnotation& annotation) {
            int openBracketCtr = 0;
            ResolutionAnnotation closeBracketResolution;
            
            for (size_t i = 0; i < annotation.location.size(); i++) {

                for (size_t j = annotation.location[i].location; j < annotation.location[i].location + annotation.location[i].length; j++) {

                    if (source[j] == '[') {

                        if (openBracketCtr>0) {
                            ResolutionAnnotation resolution;
                            resolution.message = "use %5B instead of '['";
                            resolution.resolvedSource = "%5B";
                            resolution.location.location = j;
                            resolution.location.length = 1;
                            annotation.resolutions.push_back(resolution);
                        }

                        openBracketCtr++;
                    }
                    if (source[j] == ']') {

                        if (!closeBracketResolution.message.empty()) {
                            annotation.resolutions.push_back(closeBracketResolution);
                        }

                        closeBracketResolution.message = "use %5D instead of ']'";
                        closeBracketResolution.resolvedSource = "%5D";
                        closeBracketResolution.location.location = j;
                        closeBracketResolution.location.length = 1;
                    }
                }
            }
        }
    };

    /**
    *  \brief resolver for contains spaces warnings
    *   produces a resolution for each expression containing spaces that contains the location of the expression and the expression with the spaces removed
    *  ------------------------------
    */
    class ContainsSpacesWarningResolver : public AnnotationResolver {
    public:
        virtual bool resolvesAnnotation(const SourceAnnotation& annotation) {
            return ((annotation.code == URIWarning) && (annotation.subCode == ContainsSpacesWarningUriTemplateWarningSubCode));
        }

        virtual void resolve(const mdp::ByteBuffer& source, SourceAnnotation& annotation) {
            size_t expressionStartPos = 0;
            size_t expressionEndPos = 0;

            for (size_t i = 0; i < annotation.location.size(); i++) {

                std::string annotationSource = source.substr(annotation.location[i].location, annotation.location[i].length);
                
                // For all the expressions in the annotation
                while (expressionStartPos != std::string::npos && expressionEndPos != std::string::npos && expressionStartPos < annotation.location[i].length) {
                
                    // Find the begining and end of the expression
                    expressionStartPos = annotationSource.find("{", expressionStartPos);
                    expressionEndPos = annotationSource.find("}", expressionStartPos);
                    
                    if (expressionStartPos != std::string::npos && expressionEndPos > expressionStartPos) {

                        Expression expression = annotationSource.substr(expressionStartPos, (expressionEndPos - expressionStartPos) + 1);
                    
                        if (expression.find(" ", 0) != std::string::npos) {
                            ResolutionAnnotation resolution;
                            resolution.location.location = annotation.location[i].location + expressionStartPos;
                            resolution.location.length = expressionEndPos - expressionStartPos;
                            resolution.message = "remove spaces from expressions";
                            expression.erase(remove_if(expression.begin(), expression.end(), isspace), expression.end());
                            resolution.resolvedSource = expression;
                            annotation.resolutions.push_back(resolution);
                        }
                    }
                    expressionStartPos++;
                }
            }
        }
    };

    /**
    *  \brief resolver for invalid character warnings
    *   produces a resolution for each invalid character containing the location of the character and the correct percent encoding for the character
    *  ------------------------------
    */
    class InvalidCharactersWarningResolver : public AnnotationResolver {
        std::string percentEncode(const char& toEncode) {
            std::stringstream ss;
            ss << "%" << std::hex << ((int)toEncode);
            return ss.str();
        }

        bool isInvalidCharacter(const char& character, const bool& isFirstCharacter) {
            std::string characterToTest = std::string(1, character);
            if (characterToTest == ".") return false;
            if (isFirstCharacter) {
                return !RegexMatch(characterToTest, URI_TEMPLATE_VALID_CHARACTERS_WITH_EXPRESSION_TYPES_REGEX);
            }
            return !RegexMatch(characterToTest, URI_TEMPLATE_VALID_CHARACTERS_REGEX);
        }
    public:
        virtual bool resolvesAnnotation(const SourceAnnotation& annotation) {
            return ((annotation.code == URIWarning) && (annotation.subCode == InvalidCharactersWarningUriTemplateWarningSubCode));
        }

        virtual void resolve(const mdp::ByteBuffer& source, SourceAnnotation& annotation) {
            size_t expressionStartPos = 0;
            size_t expressionEndPos = 0;

            for (size_t i = 0; i < annotation.location.size(); i++) {
                
                std::string annotationSource = source.substr(annotation.location[i].location, annotation.location[i].length);
                
                while (expressionStartPos != std::string::npos && expressionEndPos != std::string::npos && expressionStartPos < annotation.location[i].length) {
                
                    expressionStartPos = annotationSource.find("{", expressionStartPos);
                    expressionEndPos = annotationSource.find("}", expressionStartPos);
                    
                    if (expressionStartPos != std::string::npos && expressionEndPos > expressionStartPos) {

                        Expression expression = annotationSource.substr(expressionStartPos + 1, (expressionEndPos - expressionStartPos) - 1);
                    
                        for (size_t j = 0; j < expression.length(); j++) {
                            if (isInvalidCharacter(expression[j], j == 0)) {

                                ResolutionAnnotation resolution;
                                resolution.location.location = annotation.location[i].location + expressionStartPos + 1 + j;
                                resolution.location.length = 1;
                                resolution.resolvedSource = percentEncode(expression[j]);

                                std::stringstream ss;
                                ss << "replace '" << expression[j] << "' with '" << resolution.resolvedSource << "'";
                                resolution.message = ss.str();
                                
                                annotation.resolutions.push_back(resolution);
                            }
                        }
                    }
                    expressionStartPos++;
                }
            }
        }
    };


}
