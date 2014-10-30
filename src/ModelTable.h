//
//  ModelTable.h
//  snowcrash
//
//  Created by Zdenek Nemec on 6/9/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MODELTABLE_H
#define SNOWCRASH_MODELTABLE_H

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

namespace snowcrash {

    /** Symbol reference matching regex */
    const char* const ModelReferenceRegex("^[[:blank:]]*\\[" SYMBOL_IDENTIFIER "]\\[][[:blank:]]*$");

    // Resource Object Symbol
    typedef std::pair<Identifier, ResourceModel> ResourceModelSymbol;

    // Resource Object Symbol source map
    typedef std::pair<Identifier, SourceMap<ResourceModel> > ResourceModelSymbolSourceMap;

    // Resource Object Model Table
    typedef std::map<Identifier, ResourceModel> ResourceModelTable;

    // Resource Object Model Table source map
    typedef std::map<Identifier, SourceMap<ResourceModel> > ResourceModelSourceMapTable;

    struct ModelTable {

        // Resource Object Model Table
        ResourceModelTable resourceModels;
    };

    struct ModelSourceMapTable {

        // Resource Object Model Table source map
        ResourceModelSourceMapTable resourceModels;
    };

    // Checks whether given source data represents reference to a symbol returning true if so,
    // false otherwise. If source data is represent reference referred symbol name is filled in.
    inline bool GetModelReference(const mdp::ByteBuffer& sourceData,
                                   Identifier& referredModel) {

        CaptureGroups captureGroups;

        if (RegexCapture(sourceData, ModelReferenceRegex, captureGroups, 3)) {
            referredModel = captureGroups[1];
            TrimString(referredModel);
            return true;
        }

        return false;
    }

#ifdef DEBUG
    // Prints markdown block recursively to stdout
    inline void PrintModelTable(const ModelTable& modelTable) {

        std::cout << "Resource Model Symbols:\n";

        for (ResourceModelTable::const_iterator it = modelTable.resourceModels.begin();
             it != modelTable.resourceModels.end();
             ++it) {

            std::cout << "- " << it->first << " - body: '" << EscapeNewlines(it->second.body) << "'\n";
        }

        std::cout << std::endl;
    }
#endif

}

#endif
