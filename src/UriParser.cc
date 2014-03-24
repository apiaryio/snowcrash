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


URITemplateCurlyBracketCharacterParser curlyBracketParser;
URITemplateSquareBracketCharacterParser squareBracketParser;

std::vector<URITemplateCharacterParser*> URITemplateParser::characterParsers;

URITemplateParser::URITemplateParser(){
    if (characterParsers.size() == 0){
        characterParsers.push_back(&curlyBracketParser);
        characterParsers.push_back(&squareBracketParser);
    }
}



void URITemplateParser::parse(const URI uri, ParsedURITemplate& result, const SourceCharactersBlock& location)
{
    CaptureGroups groups;
    size_t gSize=5;

    

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
                    (*i)->parse(result.path[c], result, location,c+1==result.path.length()); 
            }
            
            
            //TODO: parse out identifier groups
            //TODO: loop through found id groups and pass through a list of functions to validate id
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


