//
//  UriParser.cc
//  snowcrash
//
//  Created by Carl Griffiths on 24/02/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//
#include<iomanip>
#include "UriParser.h"
#include "RegexMatch.h"

using namespace snowcrash;

URITemplateCharacterParser::~URITemplateCharacterParser(){}
URITemplateExpressionParser::~URITemplateExpressionParser(){}

URITemplateCurlyBracketCharacterParser curlyBracketParser;
URITemplateSquareBracketCharacterParser squareBracketParser;
URITemplatePrefixSlashExpressionParser prefixSlashExpressionParser;

std::vector<URITemplateCharacterParser*> URITemplateParser::characterParsers;
std::vector<URITemplateExpressionParser*> URITemplateParser::expressionParsers;


URITemplateParser::URITemplateParser(){
    if (characterParsers.size() == 0){
        characterParsers.push_back(&curlyBracketParser);
        characterParsers.push_back(&squareBracketParser);
    }

    if (expressionParsers.size() == 0){
        expressionParsers.push_back(&prefixSlashExpressionParser);
    }
}



void URITemplateParser::parse(const URI uri, ParsedURITemplate& result, const SourceCharactersBlock& location)
{
    CaptureGroups groups;
    size_t gSize=0;

    

    if (RegexCapture(uri, URI_REGEX, groups, gSize)){
        result.scheme = groups[1];
        result.host = groups[3];
        result.path = groups[4];
        
        for (std::vector<URITemplateCharacterParser*>::iterator i = characterParsers.begin();
            i != characterParsers.end(); ++i){
            (*i)->initialiseParsing();
        }

        for (int c = 0; c < result.path.length(); c++){
            for (std::vector<URITemplateCharacterParser*>::iterator i = characterParsers.begin();
                i != characterParsers.end(); ++i){
                (*i)->parse(result.path[c], result, location, c + 1 == result.path.length());
            }
        }

        CaptureGroups expressions;
        size_t eSize = 0;
        if (RegexCapture(result.path, URI_TEMPLATE_EXPRESSION_REGEX, expressions, eSize)){
            for (std::vector<URITemplateExpressionParser*>::iterator i = expressionParsers.begin();
                i != expressionParsers.end(); ++i){
                (*i)->initialiseParsing();
            }

            for (std::vector<URITemplateExpressionParser*>::iterator i = expressionParsers.begin();
                i != expressionParsers.end(); ++i){
                for (CaptureGroups::iterator e = expressions.begin()+1; e != expressions.end(); ++e){
                    (*i)->parse(*e, result, location);
                }
            }
        }
    }
    else{
        result.errors.push_back(Error("Failed to parse URI Template", URIWarning, location));
    }
}






void URITemplateCurlyBracketCharacterParser::initialiseParsing(){
    alreadyWarned = false;
    lastBracket = ' ';
}

void URITemplateCurlyBracketCharacterParser::parse(const char c, ParsedURITemplate& result, const SourceCharactersBlock& location,bool lastCharacter){
   
    if (!alreadyWarned && (
        (c == '{' && lastBracket == '{') || 
        (c == '}' && lastBracket != '{') || 
        (lastCharacter && (lastBracket=='{' || c=='{')&& c!='}')
        )){
        result.warnings.push_back(Warning("URI Template contains malformed curley brackets", URIWarning, location));
        alreadyWarned = true;
    }
 
    if (c == '{' || c == '}'){
        lastBracket = c;
    }
}

URITemplateCurlyBracketCharacterParser::~URITemplateCurlyBracketCharacterParser(){}



void URITemplateSquareBracketCharacterParser::initialiseParsing(){
    alreadyWarned = false;
}

void URITemplateSquareBracketCharacterParser::parse(const char c, ParsedURITemplate& result, const SourceCharactersBlock& location, bool lastCharacter){
    if(!alreadyWarned && (c == '[' || c == ']')){
        result.warnings.push_back(Warning("URI Template contains square brackets outside of host", URIWarning, location));
        alreadyWarned = true;
    }
}

URITemplateSquareBracketCharacterParser::~URITemplateSquareBracketCharacterParser(){}




void URITemplatePrefixSlashExpressionParser::initialiseParsing(){
    alreadyWarned = false;
}

void URITemplatePrefixSlashExpressionParser::parse(const std::string expression, ParsedURITemplate& result, const SourceCharactersBlock& location){
    if (expression[0] == '/' && !alreadyWarned){
        result.warnings.push_back(Warning("URI Template Expression unsupported slash prefix path segments", URIWarning, location));
        alreadyWarned = true;
    }
}

URITemplatePrefixSlashExpressionParser::~URITemplatePrefixSlashExpressionParser(){}