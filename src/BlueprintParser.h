//
//  BlueprintParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_BLUEPRINTPARSER_H
#define SNOWCRASH_BLUEPRINTPARSER_H

#include <functional>
#include <sstream>
#include <iterator>
#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "ResourceParser.h"
#include "ResourceGroupParser.h"

static const std::string ExpectedAPINameMessage = "expected API name, e.g. '# <API Name>'";

namespace snowcrash {
    
    //
    // Block Classifier, Resource Context
    //
    template <>
    FORCEINLINE SectionType ClassifyBlock<Blueprint>(const BlockIterator& begin,
                                                     const BlockIterator& end,
                                                     const SectionType& context) {
        
        if (HasResourceGroupSignature(*begin) ||
            HasResourceSignature(*begin))
            return ResourceGroupSectionType; // Treat Resource as anonymous resource group
        
        return (context == ResourceGroupSectionType) ? UndefinedSectionType : BlueprintSectionType;
    }
    
    //
    // Blueprint SectionType Parser
    //
    template<>
    struct SectionParser<Blueprint> {
        
        static ParseSectionResult ParseSection(const BlueprintSection& section,
                                               const BlockIterator& cur,
                                               BlueprintParserCore& parser,
                                               Blueprint& output) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            
            if ((section.type != BlueprintSectionType) &&
                !CheckBlueprintName(*cur, parser, result.first))
                return result;
            
            switch (section.type) {
                    
                case BlueprintSectionType:
                    result = HandleBlueprintOverviewBlock(section, cur, parser, output);
                    break;
                    
                case ResourceGroupSectionType:
                    result = HandleResourceGroup(section, cur, parser, output);
                    break;
                    
                default:
                    result.first.error = UnexpectedBlockError(*cur, parser.sourceData);
                    break;
            }
            
            return result;
        }
        
        // Checks blueprint name. Returns true on success, false otherwise
        static bool CheckBlueprintName(const MarkdownBlock& block, BlueprintParserCore& parser, Result& result) {
            if (!(parser.options & RequireBlueprintNameOption))
                return true;
            
            if (!parser.blueprint.name.empty())
                return true;
            
            // ERR: No API name specified
            result.error = Error(ExpectedAPINameMessage,
                                 BusinessError,
                                 MapSourceDataBlock(block.sourceMap, parser.sourceData));
            return false;
        }
        
        // Returns true if given block is first block in document
        // after metadata block, false otherwise.
        static bool IsFirstBlock(const BlockIterator& cur,
                                 const SectionBounds& bounds,
                                 const Blueprint& blueprint) {
            
            if (blueprint.metadata.empty())
                return cur == bounds.first;
            
            return std::distance(bounds.first, cur) == 1;
        }

        static ParseSectionResult HandleBlueprintOverviewBlock(const BlueprintSection& section,
                                                               const BlockIterator& cur,
                                                               BlueprintParserCore& parser,
                                                               Blueprint& output) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur(cur);
            if (cur->type == HeaderBlockType &&
                IsFirstBlock(cur, section.bounds, output)) {
                output.name = cur->content;
            }
            else {
                if (sectionCur == section.bounds.first &&
                    sectionCur->type == ParagraphBlockType) {
                    
                    // Try to parse first paragraph as metadata
                    result = ParseMetadataBlock(sectionCur, section.bounds, parser, output);
                    if (result.second != sectionCur)
                        return result;
                }
                
                if (sectionCur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(cur, section.bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (sectionCur->type == ListBlockBeginType) {
                    sectionCur = SkipToSectionEnd(cur, section.bounds.second, ListBlockBeginType, ListBlockEndType);
                }
                
                if (IsFirstBlock(cur, section.bounds, output)) {
                    if (parser.options & RequireBlueprintNameOption) {
                        if (!CheckBlueprintName(*sectionCur, parser, result.first))
                            return result;
                    }
                    else {
                        // WARN: No API name specified
                        SourceCharactersBlock sourceBlock = CharacterMapForBlock(sectionCur, section.bounds, cur, parser.sourceData);
                        result.first.warnings.push_back(Warning(ExpectedAPINameMessage,
                                                                APINameWarning,
                                                                sourceBlock));
                    }
                }
                
                if (!CheckCursor(section, sectionCur, parser.sourceData, result.first))
                    return result;
                output.description += MapSourceData(parser.sourceData, sectionCur->sourceMap);
            }
            
            result.second = ++sectionCur;
            return result;
        }
        
        static ParseSectionResult HandleResourceGroup(const BlueprintSection& section,
                                                      const BlockIterator& cur,
                                                      BlueprintParserCore& parser,
                                                      Blueprint& output)
        {
            ResourceGroup resourceGroup;
            ParseSectionResult result = ResourceGroupParser::Parse(cur,
                                                                   section.bounds.second,
                                                                   section,
                                                                   parser,
                                                                   resourceGroup);
            if (result.first.error.code != Error::OK)
                return result;
            
            Collection<ResourceGroup>::const_iterator duplicate = FindResourceGroup(parser.blueprint, resourceGroup);
            if (duplicate != parser.blueprint.resourceGroups.end()) {
                
                // WARN: duplicate group
                std::stringstream ss;
                if (resourceGroup.name.empty()) {
                    ss << "anonymous group";
                }
                else {
                    ss << "group '" << resourceGroup.name << "'";
                }
                ss << " is already defined";
                
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(cur, section.bounds, section.bounds.second, parser.sourceData);
                result.first.warnings.push_back(Warning(ss.str(),
                                                        DuplicateWarning,
                                                        sourceBlock));
            }
            
            output.resourceGroups.push_back(resourceGroup); // FIXME: C++11 move
            return result;
        }
        

        static ParseSectionResult ParseMetadataBlock(const BlockIterator& cur,
                                                     const SectionBounds& bounds,
                                                     BlueprintParserCore& parser,
                                                     Blueprint& output) {
            
            typedef Collection<Metadata>::type MetadataCollection;
            typedef Collection<Metadata>::iterator MetadataCollectionIterator;
            MetadataCollection metadataCollection;
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            SourceData content = cur->content;
            TrimStringEnd(content);
            std::vector<std::string> lines = Split(content, '\n');
            for (std::vector<std::string>::iterator line = lines.begin();
                 line != lines.end();
                 ++line) {
                
                Metadata metadata;
                if (KeyValueFromLine(*line, metadata))
                    metadataCollection.push_back(metadata);
            }
            
            if (lines.size() == metadataCollection.size()) {
                
                // Check duplicates
                std::vector<std::string> duplicateKeys;
                for (MetadataCollectionIterator it = metadataCollection.begin();
                     it != metadataCollection.end();
                     ++it) {
                    
                    MetadataCollectionIterator from = it;
                    if (++from == metadataCollection.end())
                        break;
                    
                    MetadataCollectionIterator duplicate = std::find_if(from,
                                                                        metadataCollection.end(),
                                                                        std::bind2nd(MatchFirsts<Metadata>(), *it));
                    
                    if (duplicate != metadataCollection.end() &&
                        std::find(duplicateKeys.begin(), duplicateKeys.end(), it->first) == duplicateKeys.end()) {
                        
                        duplicateKeys.push_back(it->first);
                        
                        // WARN: duplicate metada definition
                        std::stringstream ss;
                        ss << "duplicate definition of '" << it->first << "'";
                        
                        SourceCharactersBlock sourceBlock = CharacterMapForBlock(cur, bounds, bounds.second, parser.sourceData);
                        result.first.warnings.push_back(Warning(ss.str(),
                                                                DuplicateWarning,
                                                                sourceBlock));
                    }
                }
                
                // Insert parsed metadata into output
                output.metadata.insert(output.metadata.end(),
                                       metadataCollection.begin(),
                                       metadataCollection.end());
                
                ++result.second;
            }
            else if (!metadataCollection.empty()) {
                // WARN: malformed metadata block
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(cur, bounds, bounds.second, parser.sourceData);
                result.first.warnings.push_back(Warning("ignoring possible metadata, expected"
                                                        " '<key> : <value>', one one per line",
                                                        FormattingWarning,
                                                        sourceBlock));
            }
            
            return result;
        }
    };
    
    typedef BlockParser<Blueprint, SectionParser<Blueprint> > BlueprintParserInner;
    
    
    //
    // Blueprint Parser
    //
    class BlueprintParser {
    public:
        // Parse Markdown AST into API Blueprint AST
        static void Parse(const SourceData& sourceData,
                          const MarkdownBlock::Stack& source,
                          BlueprintParserOptions options,
                          Result& result,
                          Blueprint& blueprint) {
            
            BlueprintParserCore parser(options, sourceData, blueprint);
            BlueprintSection rootSection(std::make_pair(source.begin(), source.end()));
            ParseSectionResult sectionResult = BlueprintParserInner::Parse(source.begin(),
                                                                           source.end(),
                                                                           rootSection,
                                                                           parser,
                                                                           blueprint);
            result += sectionResult.first;
            
#ifdef DEBUG
            PrintSymbolTable(parser.symbolTable);
#endif
            if (result.error.code != Error::OK)
                return;
            
            PostParseCheck(sourceData, source, parser, result);
        }
        
        // Perform additional post-parsing result checks
        static void PostParseCheck(const SourceData& sourceData,
                                   const MarkdownBlock::Stack& source,
                                   BlueprintParserCore& parser,
                                   Result& result) {
            
            if (parser.options & RequireBlueprintNameOption) {
                MarkdownBlock b;
                b.sourceMap = MakeSourceDataBlock(0, 0);
                BlueprintParserInner::CheckBlueprintName(b, parser, result);
            }
        }
    };
}

#endif
