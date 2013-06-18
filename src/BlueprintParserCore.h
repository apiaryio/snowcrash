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
#include "StringUtility.h"
#include "ParserCore.h"
#include "MarkdownBlock.h"
#include "Blueprint.h"
#include "SymbolTable.h"

// Recognized HTTP headers, regex string
#define HTTP_METHODS "GET|POST|PUT|DELETE|OPTIONS|PATCH|PROPPATCH|LOCK|UNLOCK|COPY|MOVE|MKCOL|HEAD"

// (Naive) URI Template, regex string
#define URI_TEMPLATE "/.+"

namespace snowcrash {
    
    // Block Classification Section
    enum Section {
        UndefinedSection,
        BlueprintSection,
        ResourceGroupSection,
        ResourceSection,
        ResourceMethodSection,
        MethodSection,
        RequestSection,
        RequestBodySection,
        ResponseSection,
        ResponseBodySection,
        ObjectSection,
        ObjectBodySection,
        BodySection,
        SchemaSection,
        HeadersSection,
        ForeignSection
    };
    
    // Returns human readable name of given <Section>
    inline std::string SectionName(const Section& section) {
        switch (section) {
                
            case ObjectSection:
                return "object";
                
            case RequestSection:
                return "request";
                
            case ResponseSection:
                return "response";
                
            case BodySection:
                return "body";
                
            case SchemaSection:
                return "schema";
                
            case HeadersSection:
                return "headers";
                
            default:
                return "section";
        }
    }
    
    // Parser iterator
    typedef MarkdownBlock::Stack::const_iterator BlockIterator;
    
    // Parsing sub routine result
    typedef std::pair<Result, BlockIterator> ParseSectionResult;
    
    // Section boundaries (begin : end)
    typedef std::pair<BlockIterator, BlockIterator> SectionBounds;
    
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
            return first.uriTemplate == second.uriTemplate;
        }
    };
    
    // Method matching predicate
    template <class T>
    struct MatchMethod : std::binary_function<T, T, bool> {
        bool operator()(const T& first, const T& second) const {
            return first.method == second.method;
        }
    };
    
    // Pair first matching predicate
    template <class T>
    struct MatchFirst : std::binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const {
            return left.first == right.first;
        }
    };
    
    //
    // Blueprint Parser Options
    //
    enum BlueprintParserOption {
        RenderDescriptionsOption = (1 << 0),    // Render Markdown in description
        RequireBlueprintNameOption = (1 << 1)   // Treat missing blueprint name as error
    };
    typedef unsigned int BlueprintParserOptions;
    
    //
    // Parser Core Data
    //
    struct BlueprintParserCore {
        BlueprintParserCore(BlueprintParserOptions opts,
                            const SourceData& src,
                            const Blueprint& bp)
        : options(opts), sourceData(src), blueprint(bp) {}
        
        BlueprintParserOptions options;
        SymbolTable symbolTable;
        const SourceData& sourceData;
        const Blueprint& blueprint;
        
    private:
        BlueprintParserCore();
        BlueprintParserCore(const BlueprintParserCore&);
        BlueprintParserCore& operator=(const BlueprintParserCore&);
    };
    
    //
    // Section Parser prototype
    //
    template<class T>
    struct SectionParser {
        
        // Parse classified blocks
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               BlueprintParserCore& parser,
                                               T& output);
    };
    
    
    //
    // Classifier of internal list items prototype
    //
    template <class T>
    Section ClassifyInternaListBlock(const BlockIterator& begin,
                                     const BlockIterator& end);
        
    //
    // Block Classifier prototype, Look Ahead
    //
    template <class T>
    Section ClassifyBlock(const BlockIterator& begin,
                          const BlockIterator& end,
                          const Section& context);
    
    // Forward declaration of classifier helpers
    extern bool HasResourceGroupSignature(const MarkdownBlock& block);
    extern bool HasMethodSignature(const MarkdownBlock& block);
    extern bool HasResourceSignature(const MarkdownBlock& block);
    extern bool HasPayloadSignature(const BlockIterator& begin, const BlockIterator& end);
    extern bool HasPayloadAssetSignature(const BlockIterator& begin, const BlockIterator& end);
    extern bool HasAssetSignature(const BlockIterator& begin, const BlockIterator& end);
    extern bool HasHeaderSignature(const BlockIterator& begin, const BlockIterator& end);
    
    //
    // Block Parser, iterates over block and call section parser P<T>
    //
    template <class T, class P>
    struct BlockParser : public P {
        
        // Iterate blocks, classify & parse
        static ParseSectionResult Parse(const BlockIterator& begin,
                                        const BlockIterator& end,
                                        BlueprintParserCore& parser,
                                        T& output) {
            Result result;
            Section currentSection = UndefinedSection;
            BlockIterator currentBlock = begin;
            while (currentBlock != end) {
                
                currentSection = ClassifyBlock<T>(currentBlock, end, currentSection);
                ParseSectionResult sectionResult = P::ParseSection(currentSection,
                                                                   currentBlock,
                                                                   std::make_pair(begin, end),
                                                                   parser,
                                                                   output);
                
                result += sectionResult.first;
                if (result.error.code != Error::OK)
                    break;
                
                if (sectionResult.second == currentBlock)
                    break;

                currentBlock = sectionResult.second;

                if (currentSection == UndefinedSection)
                    break;
            }
            
            return std::make_pair(result, currentBlock);
        }        
    };
    
    // Advances iterator from sectionBegin to the same level' sectionEnd
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
    
    // Parse one line of raw `key:value` data.
    // Returns true on success, false otherwise.
    static inline bool KeyValueFromLine(const std::string& line, KeyValuePair& keyValuePair) {
        
        std::vector<std::string> rawMetadata = SplitOnFirst(line, ':');
        if (rawMetadata.size() != 2)
            return false;
        
        keyValuePair = std::make_pair(rawMetadata[0], rawMetadata[1]);
        TrimString(keyValuePair.first);
        TrimString(keyValuePair.second);
        
        return (!keyValuePair.first.empty() && !keyValuePair.second.empty());
    }
    
    ///! \brief Checks cursor validity within its container.
    ///! \param cur an iterator to be checked
    ///! \param bounds boundaries to check against
    ///! \param parent cursor's parent block to be used in case of error reporting
    ///! \param result error result output, an error object is added in case of failed check
    ///! \returns true if cursor appears to be valid false otherwise
    inline bool CheckCursor(const BlockIterator& cur,
                            const SectionBounds& bounds,
                            const BlockIterator& parent,
                            Result& result) {
        if (cur != bounds.second)
            return true;
        
        result.error = Error("unexpected markdown closure", 1, parent->sourceMap);
        return false;
    }
    
    
}

#endif
