//
//  BlueprintParserCore.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_BLUEPRINTPARSERCORE_H
#define SNOWCRASH_BLUEPRINTPARSERCORE_H

#include <algorithm>
#include "ParserCore.h"
#include "MarkdownBlock.h"
#include "Blueprint.h"

// Recognized HTTP headers, regex string
#define HTTP_METHODS "GET|POST|PUT|DELETE|OPTIONS|PATCH|PROPPATCH|LOCK|UNLOCK|COPY|MOVE|MKCOL|HEAD"

namespace snowcrash {
    
    // Block Classification Section
    enum Section {
        UndefinedSection,
        BlueprintSection,
        ResourceGroupSection,
        ResourceSection,
        MethodSection,
        RequestSection,
        ResponseSection,
        TerminatorSection
    };
    
    // Parser iterator
    typedef MarkdownBlock::Stack::const_iterator BlockIterator;
    
    // Parsing sub routine result
    typedef std::pair<Result, BlockIterator> ParseSectionResult;
    
    // Advances iterator from sectionBegin to the same leve sectionEnd
    inline BlockIterator SkipToSectionEnd(const BlockIterator& begin,
                                          const BlockIterator& end,
                                          MarkdownBlockType sectionBegin,
                                          MarkdownBlockType sectionEnd) {

        BlockIterator currentBlock = begin;
        if (currentBlock->type == sectionBegin) {
            int level = 1;
            ++currentBlock;
            while (currentBlock != end && level) {
                if (currentBlock->type == sectionBegin)
                    ++level;
                
                if (currentBlock->type == sectionEnd)
                    --level;
                
                if (level)
                    ++currentBlock;
            }
        }
        
        return currentBlock;
    }
    
    // Name matching predicate
    template <class T>
    struct MatchName : std::binary_function<T, T, bool> {
        bool operator()(const T& first, const T& second) const {
            return first.name == second.name;
        }
    };
    
    // URI matching predicate
    template <class T>
    struct MatchURI : std::binary_function<T, T, bool> {
        bool operator()(const T& first, const T& second) const {
            return first.uri == second.uri;
        }
    };
    
    // Method matching predicate
    template <class T>
    struct MatchMethod : std::binary_function<T, T, bool> {
        bool operator()(const T& first, const T& second) const {
            return first.method == second.method;
        }
    };
    
    
    //
    // Section Parser prototype
    //
    template<class T>
    struct SectionParser {
        
        // Parse classified blocks
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& begin,
                                               const BlockIterator& end,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               T& output);
    };
    
    //
    // Section Overview Parser prototype
    //
    template<class T>
    struct SectionOverviewParser {
        
        // Parse section overview blocks
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& begin,
                                               const BlockIterator& end,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               T& output);
    };
    
    //
    // Block Classifier prototype
    //
    template <class T>
    Section TClassifyBlock(const MarkdownBlock& block, const Section& context);
    
    //
    // Block Parser, iterates over block and call section parser P<T>
    //
    template <class T, class P>
    struct BlockParser : public P {
        
        // Iterate blocks, classify & parse
        static ParseSectionResult Parse(const BlockIterator& begin,
                                        const BlockIterator& end,
                                        const SourceData& sourceData,
                                        const Blueprint& blueprint,
                                        T& output) {
            Result result;
            Section currentSection = UndefinedSection;
            BlockIterator currentBlock = begin;
            while (currentBlock != end) {
                
                currentSection = TClassifyBlock<T>(*currentBlock, currentSection);
                ParseSectionResult sectionResult = P::ParseSection(currentSection,
                                                                   currentBlock,
                                                                   end,
                                                                   sourceData,
                                                                   blueprint,
                                                                   output);
                
                result += sectionResult.first;
                if (result.error.code != Error::OK)
                    break;
                
                if (sectionResult.second == currentBlock)
                    break;

                currentBlock = sectionResult.second;
            }
            
            return std::make_pair(result, currentBlock);
        }        
    };
}

#endif
