//
//  SectionParserData.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/21/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SECTIONPARSERDATA_H
#define SNOWCRASH_SECTIONPARSERDATA_H

#include "BlueprintSourcemap.h"
#include "Section.h"
#include "SymbolTable.h"

namespace snowcrash {

    /**
     *  \brief Blueprint Parser Options.
     *
     *  Controls blueprint parser behavior.
     */
    enum BlueprintParserOption {
        RenderDescriptionsOption = (1 << 0),    /// < Render Markdown in description.
        RequireBlueprintNameOption = (1 << 1),  /// < Treat missing blueprint name as error
        ExportSourcemapOption = (1 << 2)        /// < Export source maps AST
    };

    typedef unsigned int BlueprintParserOptions;

    /**
     *  \brief Section Parser Data
     *
     *  State of the parser.
     */
    struct SectionParserData {
        SectionParserData(BlueprintParserOptions opts,
                          const mdp::ByteBuffer& src,
                          const Blueprint& bp)
        : options(opts), sourceData(src), blueprint(bp) {}

        /** Parser Options */
        BlueprintParserOptions options;

        /** Symbol Table */
        SymbolTable symbolTable;

        /** Symbol Table Sourcemap */
        SymbolSourceMapTable symbolSourceMapTable;

        /** Source Data */
        const mdp::ByteBuffer& sourceData;

        /** AST being parsed **/
        const Blueprint& blueprint;

        /** Sections Context */
        typedef std::vector<SectionType> SectionsStack;
        SectionsStack sectionsContext;

        /** \returns Actual Section Context */
        SectionType sectionContext() const {
            return (sectionsContext.empty()) ? UndefinedSectionType : sectionsContext.back();
        }

        /** \returns Parent Section Context */
        SectionType parentSectionContext() const {
            if (sectionsContext.empty())
                return UndefinedSectionType;

            size_t size = sectionsContext.size();
            if (size == 1)
                return sectionsContext.back();
            else
                return sectionsContext[size-2];
        }

        /** \returns True if exporting source maps */
        bool exportSourceMap() const {
            return options & ExportSourcemapOption;
        }

    private:
        SectionParserData();
        SectionParserData(const SectionParserData&);
        SectionParserData& operator=(const SectionParserData&);
    };
}

#endif
