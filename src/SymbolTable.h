//
//  SymbolTable.h
//  snowcrash
//
//  Created by Zdenek Nemec on 6/9/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_SYMBOLTABLE_H
#define SNOWCRASH_SYMBOLTABLE_H

#include <string>
#include <map>

#ifdef DEBUG
#include <iostream>
#include "Serialize.h"
#endif

#include "Blueprint.h"

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
