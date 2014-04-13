//
//  BlueprintParserCore.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_BLUEPRINTPARSERCORE_H
#define SNOWCRASH_BLUEPRINTPARSERCORE_H

#include <algorithm>
#include <sstream>
#include "ParserCore.h"
#include "SourceAnnotation.h"
#include "MarkdownBlock.h"
#include "BlueprintSection.h"
#include "HTTP.h"
#include "Blueprint.h"
#include "BlueprintUtility.h"
#include "StringUtility.h"
#include "SymbolTable.h"

namespace snowcrash {
    
    /**
     *  \brief Blueprint Parser Options.
     *
     *  Controls blueprint parser behavior.
     */
    enum BlueprintParserOption {
        RenderDescriptionsOption = (1 << 0),    /// < Render Markdown in description.
        RequireBlueprintNameOption = (1 << 1)   /// < Treat missing blueprint name as error
    };
    
    typedef unsigned int BlueprintParserOptions;
    
    /**
     *  \brief Parsing subroutine result
     *
     *  Consists of a parsing result report (first) and
     *  %BlockIterator (second) pointing to the last parsed
     *  markdown block.
     */
    typedef std::pair<Result, BlockIterator> ParseSectionResult;
    
    
    /**
     *  Parser Core Data
     */
    struct BlueprintParserCore {
        BlueprintParserCore(BlueprintParserOptions opts,
                            const SourceData& src,
                            const Blueprint& bp)
        : options(opts), sourceData(src), blueprint(bp) {}
        
        /** Parser Options */
        BlueprintParserOptions options;
        
        /** Symbol Table */
        SymbolTable symbolTable;
        
        /** Source Data */
        const SourceData& sourceData;
        
        /** AST being parsed **/
        const Blueprint& blueprint;
        
    private:
        BlueprintParserCore();
        BlueprintParserCore(const BlueprintParserCore&);
        BlueprintParserCore& operator=(const BlueprintParserCore&);
    };
    
    /**
     *  SectionType Parser prototype.
     */
    template<class T>
    struct SectionParser {
        
        /**
         *  \brief  Parse section's blocks.
         *  \param  section     Actual section being parsed.
         *  \param  cur         Cursor within the section boundaries.
         *  \param  parser      Parser instance.
         *  \param  output      AST node parsed.
         *  \return %ParseSectionResult pointing to the last block parsed & including 
         *  any possible source annotations in the form of results or warnings.
         */
        static ParseSectionResult ParseSection(const BlueprintSection& section,
                                               const BlockIterator& cur,
                                               BlueprintParserCore& parser,
                                               T& output);
        
        /**
         *  \brief  Optional post-parse processing.
         */
        static void Finalize(const SectionBounds& bounds,
                             BlueprintParserCore& parser,
                             T& output,
                             Result& result);
    };
    
    /**
     *  \brief Internal list items classifier prototype.
     *
     *  The classifier might look ahead.
     *  DO NOT provide generic implementation.
     */
    template <class T>
    FORCEINLINE SectionType ClassifyInternaListBlock(const BlockIterator& begin,
                                                     const BlockIterator& end);
    /**
     *  \brief Classify a block with children list items.
     *
     *  Classifies internal list block & descend to children list block classifiers.
     *  DO NOT provide generic implementation.
     */
    template <class T>
    FORCEINLINE SectionType ClassifyChildrenListBlock(const BlockIterator& begin,
                                                      const BlockIterator& end);
        
    /**
     *  \brief Block Classifier prototype.
     *
     *  The classifier might look ahead.
     */
    template <class T>
    FORCEINLINE SectionType ClassifyBlock(const BlockIterator& begin,
                                          const BlockIterator& end,
                                          const SectionType& context);
    
    // Forward declaration of classifier helpers
    extern bool HasResourceGroupSignature(const MarkdownBlock& block);
    extern bool HasActionSignature(const MarkdownBlock& block);
    extern bool HasResourceSignature(const MarkdownBlock& block);
    extern bool HasPayloadSignature(const BlockIterator& begin, const BlockIterator& end);
    extern bool HasPayloadAssetSignature(const BlockIterator& begin, const BlockIterator& end);
    extern bool HasAssetSignature(const BlockIterator& begin, const BlockIterator& end);
    extern bool HasHeaderSignature(const BlockIterator& begin, const BlockIterator& end);
    extern bool HasParametersSignature(const BlockIterator& begin, const BlockIterator& end);
    
    /**
     *  \brief A Markdown block parser.
     *
     *  Iterates over blocks classifying sections and calling relevant %SectionParser P<T>.
     */
    template <class T, class P>
    struct BlockParser : public P {
        
        // Iterate blocks, classify & parse
        static ParseSectionResult Parse(const BlockIterator& begin,
                                        const BlockIterator& end,
                                        const BlueprintSection& parentSection,
                                        BlueprintParserCore& parser,
                                        T& output) {
            Result result;
            SectionType currentSectionType = UndefinedSectionType;
            BlockIterator currentBlock = begin;
            while (currentBlock != end) {
                
                currentSectionType = ClassifyBlock<T>(currentBlock, end, currentSectionType);
                BlueprintSection currentSection(currentSectionType, std::make_pair(begin, end), parentSection);
                
                ParseSectionResult sectionResult = P::ParseSection(currentSection,
                                                                   currentBlock,
                                                                   parser,
                                                                   output);
                
                result += sectionResult.first;
                if (result.error.code != Error::OK)
                    break;
                
                if (sectionResult.second == currentBlock)
                    break;

                currentBlock = sectionResult.second;

                if (currentSectionType == UndefinedSectionType)
                    break;
            }
            
            P::Finalize(std::make_pair(begin, currentBlock), parser, output, result);
            
            return std::make_pair(result, currentBlock);
        }        
    };
}

#endif
