//
//  BlueprintParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
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

static const std::string ExpectedAPINameMessage = "expected API name, e.g. `# <API Name>`";

namespace snowcrash {
    
    //
    // Block Classifier, Resource Context
    //
    template <>
    inline Section ClassifyBlock<Blueprint>(const BlockIterator& begin,
                                            const BlockIterator& end,
                                            const Section& context) {
        
        if (begin->type == HRuleBlockType)
            return TerminatorSection;
        
        if (HasResourceSignature(*begin))
            return ResourceGroupSection; // Treat Resource as anonymous resource group
        
        if (context == ResourceGroupSection ||
            context == TerminatorSection)
            return ResourceGroupSection;
        
        return BlueprintSection;
    }
    
    //
    // Blueprint Section Parser
    //
    template<>
    struct SectionParser<Blueprint> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               const ParserCore& parser,
                                               Blueprint& output) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            
            if ((section == TerminatorSection || section == ResourceGroupSection) &&
                !CheckBlueprintName(cur, parser, result.first))
                return result;
            
            switch (section) {
                case TerminatorSection:
                    result.second = ++BlockIterator(cur);
                    break;
                    
                case BlueprintSection:
                    result = HandleBlueprintOverviewBlock(cur, bounds, parser, output);
                    break;
                    
                case ResourceGroupSection:
                    result = HandleResourceGroup(cur, bounds.second, parser, output);
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, cur->sourceMap);
                    break;
            }
            
            return result;
        }
        
        // Checks blueprint name. Returns true on success, false otherwise
        static bool CheckBlueprintName(const BlockIterator& cur, const ParserCore& parser, Result& result) {
            if (!(parser.options & RequireBlueprintNameOption))
                return true;
            
            if (!parser.blueprint.name.empty())
                return true;
            
            // ERR: No API name specified
            result.error = Error(ExpectedAPINameMessage,
                                 2,
                                 cur->sourceMap);
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

        static ParseSectionResult HandleBlueprintOverviewBlock(const BlockIterator& cur,
                                                               const SectionBounds& bounds,
                                                               const ParserCore& parser,
                                                               Blueprint& output) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur(cur);
            if (cur->type == HeaderBlockType &&
                IsFirstBlock(cur, bounds, parser.blueprint)) {
                output.name = cur->content;
            }
            else {
                if (sectionCur == bounds.first &&
                    sectionCur->type == ParagraphBlockType) {
                    
                    // Try to parse first paragraph as metadata
                    result = ParseMetadataBlock(sectionCur, bounds, parser, output);
                    if (result.second != sectionCur)
                        return result;
                }
                
                if (sectionCur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(cur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (sectionCur->type == ListBlockBeginType) {
                    sectionCur = SkipToSectionEnd(cur, bounds.second, ListBlockBeginType, ListBlockEndType);
                }
                
                if (IsFirstBlock(cur, bounds, parser.blueprint)) {
                    if (parser.options & RequireBlueprintNameOption) {
                        if (!CheckBlueprintName(sectionCur, parser, result.first))
                            return result;
                    }
                    else {
                        // WARN: No API name specified
                        result.first.warnings.push_back(Warning(ExpectedAPINameMessage,
                                                                0,
                                                                sectionCur->sourceMap));
                    }
                }
                
                output.description += MapSourceData(parser.sourceData, sectionCur->sourceMap);
            }
            
            result.second = ++sectionCur;
            return result;
        }
        
        static ParseSectionResult HandleResourceGroup(const BlockIterator& begin,
                                                      const BlockIterator& end,
                                                      const ParserCore& parser,
                                                      Blueprint& output)
        {
            ResourceGroup resourceGroup;
            ParseSectionResult result = ResourceGroupParser::Parse(begin, end, parser, resourceGroup);
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
                    ss << "group `" << resourceGroup.name << "`";
                }
                ss << " is already defined";
                
                result.first.warnings.push_back(Warning(ss.str(), 0, begin->sourceMap));
            }
            
            output.resourceGroups.push_back(resourceGroup); // FIXME: C++11 move
            return result;
        }
        

        static ParseSectionResult ParseMetadataBlock(const BlockIterator& cur,
                                                     const SectionBounds& bounds,
                                                     const ParserCore& parser,
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
                                                                        std::bind2nd(MatchFirst<Metadata>(), *it));
                    
                    if (duplicate != metadataCollection.end() &&
                        std::find(duplicateKeys.begin(), duplicateKeys.end(), it->first) == duplicateKeys.end()) {
                        
                        duplicateKeys.push_back(it->first);
                        
                        // WARN: duplicate metada definition
                        std::stringstream ss;
                        ss << "duplicate definition of `" << it->first << "`";
                        result.first.warnings.push_back(Warning(ss.str(),
                                                                0,
                                                                cur->sourceMap));
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
                result.first.warnings.push_back(Warning("ignoring possible metadata, expected"
                                                        " `<key> : <value>`, one one per line",
                                                        0,
                                                        cur->sourceMap));
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
            
            ParserCore parser(options, sourceData, blueprint);
            ParseSectionResult sectionResult = BlueprintParserInner::Parse(source.begin(),
                                                                           source.end(),
                                                                           parser,
                                                                           blueprint);
            result += sectionResult.first;
        }
    };
}

#endif
