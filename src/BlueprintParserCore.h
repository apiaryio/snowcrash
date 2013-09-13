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
#include "HTTP.h"
#include "Blueprint.h"
#include "BlueprintUtility.h"
#include "StringUtility.h"
#include "SymbolTable.h"

namespace snowcrash {
    
    /** Block Classification Section. */
    enum Section {
        UndefinedSection,
        BlueprintSection,
        ResourceGroupSection,
        ResourceSection,
        ResourceMethodSection,
        ActionSection,
        RequestSection,
        RequestBodySection,
        ResponseSection,
        ResponseBodySection,
        ObjectSection,
        ObjectBodySection,
        ModelSection,
        ModelBodySection,
        BodySection,
        DanglingBodySection,
        SchemaSection,
        DanglingSchemaSection,
        HeadersSection,
        ForeignSection,
        ParametersSection,
        ParameterDefinitionSection,
        ParameterValuesSection
    };
    
    /** Returns human readable name for given %Section */
    FORCEINLINE std::string SectionName(const Section& section) {
        switch (section) {

            case ModelSection:
            case ModelBodySection:
                return "model";
                
            case ObjectSection:
            case ObjectBodySection:
                return "object";
                
            case RequestSection:
            case RequestBodySection:
                return "request";
                
            case ResponseSection:
            case ResponseBodySection:
                return "response";
                
            case BodySection:
                return "message-body";
                
            case SchemaSection:
                return "message-schema";
                
            case HeadersSection:
                return "headers";
                
            default:
                return "section";
        }
    }
    
    /** Markdown block iterator */
    typedef MarkdownBlock::Stack::const_iterator BlockIterator;
    
    /**
     *  \brief Parsing subroutine result 
     *
     *  Consists of a parsing result report (first) and
     *  %BlockIterator (second) pointing to the last parsed 
     *  markdown block.
     */
    typedef std::pair<Result, BlockIterator> ParseSectionResult;
    
    /** 
     *  \brief Section boundaries.
     *
     *  A continuous range of markdown blocks <first, second).
     */
    typedef std::pair<BlockIterator, BlockIterator> SectionBounds;
    
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
     *  Parser Core Data
     */
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
    
    /**
     *  Section Parser prototype.
     */
    template<class T>
    struct SectionParser {
        
        // Parse classified blocks
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               BlueprintParserCore& parser,
                                               T& output);
    };
    
    /**
     *  \brief Internal list items classifier prototype.
     *
     *  The classifier might look ahead.
     */
    template <class T>
    Section ClassifyInternaListBlock(const BlockIterator& begin,
                                     const BlockIterator& end);
        
    /**
     *  \brief Block Classifier prototype.
     *
     *  The classifier might look ahead.
     */
    template <class T>
    Section ClassifyBlock(const BlockIterator& begin,
                          const BlockIterator& end,
                          const Section& context);
    
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
     *  Iterates over blocks classifying sections and calling relevant section parser P<T>.
     */
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
    
    /**
     *  \brief  Skip to the end of a section.
     *  \param  begin   Begin of the section inside a block buffer.
     *  \param  end     End of the block buffer.
     *  \param  sectionBegin    A %MarkdownBlockType of the beginning.
     *  \param  sectionEnd      A %MarkdownBlockType of the end.
     *  \return An iterator pointing to the end of the section.
     *
     *  Advances iterator from the begin of a section to the end 
     *  of a section at the same nesting level.
     */
    FORCEINLINE BlockIterator SkipToSectionEnd(const BlockIterator& begin,
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
    
    /**
     *  \brief  Parse one line of raw `key:value` data.
     *  \param  line    A line to parse.
     *  \param  keyValuePair    The output buffer to place the parsed data into.
     *  \return True on success, false otherwise.
     */
    FORCEINLINE bool KeyValueFromLine(const std::string& line,
                                      KeyValuePair& keyValuePair) {
        
        std::vector<std::string> rawMetadata = SplitOnFirst(line, ':');
        if (rawMetadata.size() != 2)
            return false;
        
        keyValuePair = std::make_pair(rawMetadata[0], rawMetadata[1]);
        TrimString(keyValuePair.first);
        TrimString(keyValuePair.second);
        
        return (!keyValuePair.first.empty() && !keyValuePair.second.empty());
    }
    
    /**
     *  \brief Checks cursor validity within its container.
     *  \param cur  An iterator to be checked.
     *  \param bounds   Boundaries to check against.
     *  \param parent   Cursor's parent block to be used in case of error reporting.
     *  \param result   Error result output, an error object is added in case of failed check.
     *  \returns True if cursor appears to be valid, false otherwise.
     */
    FORCEINLINE bool CheckCursor(const BlockIterator& cur,
                                 const SectionBounds& bounds,
                                 const BlockIterator& parent,
                                 Result& result) {
        if (cur != bounds.second)
            return true;

        if (parent->sourceMap.empty())
            return false;
        
        // ERR: Sanity check
        result.error = Error("unexpected markdown closure", ApplicationError, parent->sourceMap);
        return false;
    }
    
    /**
     *  \brief  Construct an Unexpected block error.
     *  \param  block   A Markdown block that is unexpected.
     *  \return An Error with description relevant to the type of the unexpected block.
     */
    FORCEINLINE Error UnexpectedBlockError(const MarkdownBlock& block) {
        static const char *NotExpectedMessage = " is either not appropriate for the current context or its keyword has not been recognized, ";
        
        std::stringstream ss;
        ss << "unexpected block, ";
        
        switch (block.type) {
            case HeaderBlockType:
                ss << "this Markdown header" << NotExpectedMessage;
                ss << "recognized keywords include: Group, <HTTP method>, <URI template>, Object ...";
                break;
                
            case ListBlockBeginType:
            case ListItemBlockBeginType:
                
                ss << "this Markdown list item"  << NotExpectedMessage;
                ss << "recognized keywords include: Request, Response <HTTP status code>, Headers, Parameters ...";
                break;
                
                
            default:
                ss << "a block of this type is not expected in the current context";
                break;
        }
        
        return Error(ss.str(), BusinessError, block.sourceMap);
    }
}

#endif
