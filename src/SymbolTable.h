//
//  SymbolTable.h
//  snowcrash
//
//  Created by Zdenek Nemec on 6/9/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SYMBOLTABLE_H
#define SNOWCRASH_SYMBOLTABLE_H

#include <string>
#include <map>
#include "RegexMatch.h"

#ifdef DEBUG
#include <iostream>
#include "Serialize.h"
#endif

#include "Blueprint.h"

// Symbol identifier regex
#define SYMBOL_IDENTIFIER "([A-Za-z0-9_\\-]|[ \t])*"

// Symbol reference regex
static const std::string SymbolReferenceRegex("^[ \t]*\\[(" SYMBOL_IDENTIFIER ")\\]\\[\\][ \t]*$");

namespace snowcrash {

    // Name of a symbol
    typedef SourceData SymbolName;
    
    // Resource Object Symbol
    typedef std::pair<SymbolName, ResourceObject> ResourceObjectSymbol;
    
    // Resource Object Symbol Table
    typedef std::map<SymbolName, ResourceObject> ResourceObjectSymbolTable;
    
    struct SymbolTable {
        
        // Resource Object Symbol Table
        ResourceObjectSymbolTable resourceObjects;
    };
    
    
    // Checks wheter given source data represents reference to a symbol returning true if so,
    // false otherwise. If source data is represent reference referred symbol name is filled in.
    inline bool GetSymbolReference(const SourceData& sourceData, SymbolName& referredSymbol) {
        
        CaptureGroups captureGroups;
        if (RegexCapture(sourceData, SymbolReferenceRegex, captureGroups, 3)) {
            referredSymbol = captureGroups[1];
            TrimString(referredSymbol);
            return true;
        }
        return false;
    }
    
#ifdef DEBUG
    // Prints markdown block recursively to stdout
    inline void PrintSymbolTable(const SymbolTable& symbolTable) {

        std::cout << "Resource Object Symbols:\n";
        for (ResourceObjectSymbolTable::const_iterator it = symbolTable.resourceObjects.begin();
             it != symbolTable.resourceObjects.end();
             ++it) {
            
            std::cout << "- " << it->first << " - body: `" << EscapeNewlines(it->second.body) << "`\n";
        }
        
        std::cout << std::endl;
    }
#endif
    
}

#endif
