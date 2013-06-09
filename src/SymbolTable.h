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
#include "Blueprint.h"

namespace snowcrash {

    // Name of a symbol
    typedef SourceData SymbolName;
    
    // Object Symbol
    typedef std::pair<SymbolName, ResourceObject> ObjectSymbol;
    
    // Object Symbol Table
    typedef std::map<SymbolName, ResourceObject> ObjectSymbolTable;
    
    struct SymbolTable {
        
        // Resource Object Symbol Table
        ObjectSymbolTable objects;
    };
}

#endif
