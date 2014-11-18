//
//  ResolutionService.h
//  snowcrash
//
//  Created by Carl Griffiths on 18/11/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "SourceAnnotation.h"

#define URI_TEMPLATE_VALID_CHARACTERS_WITH_EXPRESSION_TYPES_REGEX "([?|#|+|&|A-Z|a-z|0-9|_|,])"
#define URI_TEMPLATE_VALID_CHARACTERS_REGEX "([A-Z|a-z|0-9|_|,])"

namespace snowcrash {
 
    /**
    *  \brief Base annotation resolver
    *  ------------------------------
    */
    class AnnotationResolver {
    public :
        virtual bool resolvesAnnotation(const SourceAnnotation& annotation) = 0 ;
        virtual void resolve(const mdp::ByteBuffer& source, SourceAnnotation& annotation) = 0;
    };

    /**
    *  \brief resolver for square bracket warnings
    *   produces a resolution for each square bracket containing the message to percent encode the bracket, the location of the bracket and the appropriate percent encoding
    *  ------------------------------
    */
    class SquareBracketWarningResolver : public AnnotationResolver {
    public :
        bool resolvesAnnotation(const SourceAnnotation& annotation) {
            return ( (annotation.code == URIWarning) && (annotation.subCode == SquareBracketWarning) );
        }

        void resolve(const mdp::ByteBuffer& source, SourceAnnotation& annotation) {
            int openBracketCtr = 0;
            ResolutionAnnotation closeBracketResolution;
            for (int i = 0; i < annotation.location.size(); i++) {
                for (int ctr = annotation.location[i].location; ctr < annotation.location[i].location + annotation.location[i].length; ctr++) {
                    if (source[ctr] == '[') {
                        if (openBracketCtr>0) {
                            ResolutionAnnotation resolution;
                            resolution.message = "use %5B instead of '['";
                            resolution.resolvedSource = "%5B";
                            resolution.location.location = ctr;
                            resolution.location.length = 1;
                            annotation.resolutions.push_back(resolution);
                        }
                        openBracketCtr++;
                    }
                    if (source[ctr] == ']') {
                        if (!closeBracketResolution.message.empty()) {
                            annotation.resolutions.push_back(closeBracketResolution);
                        }
                        closeBracketResolution.message = "use %5D instead of ']'";
                        closeBracketResolution.resolvedSource = "%5D";
                        closeBracketResolution.location.location = ctr;
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
    public :
        bool resolvesAnnotation(const SourceAnnotation& annotation) {
            return ((annotation.code == URIWarning) && (annotation.subCode == ContainsSpacesWarning));
        }

        void resolve(const mdp::ByteBuffer& source, SourceAnnotation& annotation) {
            size_t expressionStartPos = 0;
            size_t expressionEndPos = 0;

            for (int i = 0; i < annotation.location.size(); i++) {
                std::string annotationSource = source.substr(annotation.location[i].location, annotation.location[i].length);
                while (expressionStartPos != std::string::npos && expressionEndPos != std::string::npos && expressionStartPos < annotation.location[i].length) {
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
        static std::string percentEncode(const char& toEncode) {
            std::stringstream ss;
            ss << "%" << std::hex << ((int)toEncode);
            return ss.str();
        }

        static bool isInvalidCharacter(const char& character, const bool& isFirstCharacter) {
            std::string characterToTest = std::string(1, character);
            if (characterToTest == ".") return false;
            if (isFirstCharacter) {
                return !RegexMatch(characterToTest, URI_TEMPLATE_VALID_CHARACTERS_WITH_EXPRESSION_TYPES_REGEX);
            }
            return !RegexMatch(characterToTest, URI_TEMPLATE_VALID_CHARACTERS_REGEX);
        }
    public:
        bool resolvesAnnotation(const SourceAnnotation& annotation) {
            return ((annotation.code == URIWarning) && (annotation.subCode == InvalidCharactersWarning));
        }

        void resolve(const mdp::ByteBuffer& source, SourceAnnotation& annotation) {
            size_t expressionStartPos = 0;
            size_t expressionEndPos = 0;

            for (int i = 0; i < annotation.location.size(); i++) {
                std::string annotationSource = source.substr(annotation.location[i].location, annotation.location[i].length);
                while (expressionStartPos != std::string::npos && expressionEndPos != std::string::npos && expressionStartPos < annotation.location[i].length) {
                    expressionStartPos = annotationSource.find("{", expressionStartPos);
                    expressionEndPos = annotationSource.find("}", expressionStartPos);
                    if (expressionStartPos != std::string::npos && expressionEndPos > expressionStartPos) {
                        Expression expression = annotationSource.substr(expressionStartPos + 1, (expressionEndPos - expressionStartPos) - 1);
                        for (int j = 0; j < expression.length(); j++) {
                            if (isInvalidCharacter(expression[j],j==0)) {
                                ResolutionAnnotation resolution;
                                resolution.location.location = annotation.location[i].location + expressionStartPos + j;
                                resolution.location.length = 1;
                                resolution.message = "percent encode invalid character";
                                resolution.resolvedSource = percentEncode(expression[j]);
                                annotation.resolutions.push_back(resolution);
                            }
                        }
                    }
                    expressionStartPos++;
                }
            }
        }
    };

    /**
    *  \brief a set of annotation resolvers
    *  ------------------------------
    */
    typedef std::vector<AnnotationResolver*> AnnotationResolvers;


    /**
    *  \brief a service for providing resolutions for warnings and errors
    *  ------------------------------
    */
    class ResolutionService {
    private :
        AnnotationResolvers resolvers;

        /**
        *  \brief Resolve an annotation creating one or more resolutions for an error/warning
        *
        *  \param annotation        An annotation to resolved.
        */
        void resolveAnnotation(const mdp::ByteBuffer& source, SourceAnnotation& annotation) {
            for (AnnotationResolvers::iterator it = resolvers.begin(); it != resolvers.end(); ++it) {
                if ((*it)->resolvesAnnotation(annotation)) {
                    (*it)->resolve(source, annotation);
                    break;
                }
            }
        }

    public :
        ResolutionService() {
            resolvers.push_back(new SquareBracketWarningResolver());
            resolvers.push_back(new ContainsSpacesWarningResolver());
            resolvers.push_back(new InvalidCharactersWarningResolver());
        }

        ~ResolutionService() {
            for (AnnotationResolvers::reverse_iterator it = resolvers.rbegin(); it != resolvers.rend(); ++it) {
                delete(*it);
            }
        }
        

        /**
        *  \brief Resolve the error and warnings for a report
        *
        *  \param report        A report with warnings and errors that may need to be resolved
        */
        void resolveBlueprintReportWarningsAndErrors(const mdp::ByteBuffer& source, Report& report) {
            for (Warnings::iterator it = report.warnings.begin(); it != report.warnings.end(); ++it) {
                resolveAnnotation(source, *it);
            }

            resolveAnnotation(source, report.error);
        }
    };
}