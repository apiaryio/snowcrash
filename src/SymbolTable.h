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
#include "ByteBuffer.h"
#include "RegexMatch.h"

#ifdef DEBUG
#include <iostream>
#include "Serialize.h"
#endif

#include "BlueprintSourcemap.h"
#include "StringUtility.h"

// Symbol identifier regex
#define SYMBOL_IDENTIFIER "([^][()]+)"

namespace snowcrashconst {
    
    /** Symbol reference matching regex */
    const char* const SymbolReferenceRegex("^[[:blank:]]*\\[" SYMBOL_IDENTIFIER "]\\[][[:blank:]]*$");
}

namespace snowcrash {
    
    // Resource Object Symbol
    typedef std::pair<SymbolName, ResourceModel> ResourceModelSymbol;

    // Resource Object Symbol source map
    typedef std::pair<SymbolName, SourceMap<ResourceModel> > ResourceModelSymbolSourceMap;

    // Resource Object Symbol Table
    typedef std::map<SymbolName, ResourceModel> ResourceModelSymbolTable;
    
    // Reesource Object Symbol Table source map
    typedef std::map<SymbolName, SourceMap<ResourceModel> > ResourceModelSymbolSourceMapTable;

    struct SymbolTable {
        
        // Resource Object Symbol Table
        ResourceModelSymbolTable resourceModels;
    };
    
    struct SymbolSourceMapTable {

        // Resource Object Symbol Table source map
        ResourceModelSymbolSourceMapTable resourceModels;
    };
    
    // Checks whether given source data represents reference to a symbol returning true if so,
    // false otherwise. If source data is represent reference referred symbol name is filled in.
    inline bool GetSymbolReference(const mdp::ByteBuffer& sourceData,
                                        SymbolName& referredSymbol) {
        
        CaptureGroups captureGroups;

        if (RegexCapture(sourceData, snowcrashconst::SymbolReferenceRegex, captureGroups, 3)) {
            referredSymbol = captureGroups[1];
            TrimString(referredSymbol);
            return true;
        }

        return false;
    }
    
#ifdef DEBUG
    // Prints markdown block recursively to stdout
    inline void PrintSymbolTable(const SymbolTable& symbolTable) {

        std::cout << "Resource Model Symbols:\n";
        for (ResourceModelSymbolTable::const_iterator it = symbolTable.resourceModels.begin();
             it != symbolTable.resourceModels.end();
             ++it) {
            
            std::cout << "- " << it->first << " - body: '" << EscapeNewlines(it->second.body) << "'\n";
        }
        
        std::cout << std::endl;
    }
#endif
    
}

#endif
