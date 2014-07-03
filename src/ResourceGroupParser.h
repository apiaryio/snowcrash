//
//  ResourceGroupParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_RESOURCEGROUPPARSER_H
#define SNOWCRASH_RESOURCEGROUPPARSER_H

#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "ResourceParser.h"

namespace snowcrashconst {
    
    const char* const GroupHeaderRegex = "^[[:blank:]]*[Gg]roup[[:blank:]]+" SYMBOL_IDENTIFIER "[[:blank:]]*$";
}

namespace snowcrash {
    
    /**
     *  \brief Find a group in the blueprint by the group name.
     *  \param blueprint A blueprint AST to be searched.
     *  \param group A resource group to look for.
     *  \returns An iterator at matching group within blueprint's resource groups collection.
     */
    FORCEINLINE Collection<ResourceGroup>::const_iterator FindResourceGroup(const Blueprint& blueprint,
                                                                            const ResourceGroup& group) {

        return std::find_if(blueprint.resourceGroups.begin(),
                            blueprint.resourceGroups.end(),
                            std::bind2nd(MatchName<ResourceGroup>(), group));
    }
    
    /**
     *  \brief Check a markdown block of Group signature, also retrieves a group name.
     *  \param block A markdown block to query for its signature and retrieve a group name from.
     *  \param name An output buffer to retrieve a Resource Group Name into.
     *  \returns True if the given markdown block has Resource group signature, false otherwise.
     */
    FORCEINLINE bool GetResourceGroupSignature(const MarkdownBlock& block,
                                               Name& name) {
        if (block.type != HeaderBlockType ||
            block.content.empty())
            return false;
        
        CaptureGroups captureGroups;
        if (RegexCapture(block.content, snowcrashconst::GroupHeaderRegex, captureGroups, 3)) {
            name = captureGroups[1];
            return true;
        }
        
        return false;
    }
    
    /**
     *  \brief Query whether given block has resource group signature.
     *  \param block A markdown block to query its signature
     *  \returns True if the given markdown block has Resource group signature, false otherwise.
     */
    FORCEINLINE bool HasResourceGroupSignature(const MarkdownBlock& block)
    {
        Name name;
        return GetResourceGroupSignature(block, name);
    }
    
    /** Internal list items classifier, Resource Group Context */
    template <>
    FORCEINLINE SectionType ClassifyInternaListBlock<ResourceGroup>(const BlockIterator& begin,
                                                                    const BlockIterator& end) {
        return UndefinedSectionType;
    }
    
    /** Block Classifier, Resource Group Context */
    template <>
    FORCEINLINE SectionType ClassifyBlock<ResourceGroup>(const BlockIterator& begin,
                                                         const BlockIterator& end,
                                                         const SectionType& context) {
        
        if (HasResourceGroupSignature(*begin))
            return (context == UndefinedSectionType) ? ResourceGroupSectionType : UndefinedSectionType;
        
        if (HasResourceSignature(*begin))
            return ResourceSectionType;

        return (context == ResourceGroupSectionType) ? context : UndefinedSectionType;
    }
    
    /** Resource Group SectionType Parser */
    template<>
    struct SectionParser<ResourceGroup> {
        
        static ParseSectionResult ParseSection(const BlueprintSection& section,
                                               const BlockIterator& cur,
                                               BlueprintParserCore& parser,
                                               ResourceGroup& group) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            switch (section.type) {
                    
                case ResourceGroupSectionType:
                    result = HandleResourceGroupOverviewBlock(section, cur, parser, group);
                    break;
                    
                case ResourceSectionType:
                    result = HandleResource(section, cur, parser, group);
                    break;
                    
                case UndefinedSectionType:
                    break;
                    
                default:
                    result.first.error = UnexpectedBlockError(section, cur, parser.sourceData);
                    break;
            }
            
            return result;
        }

        static void Finalize(const SectionBounds& bounds,
                             BlueprintParserCore& parser,
                             ResourceGroup& group,
                             Result& result) {}

        static ParseSectionResult HandleResourceGroupOverviewBlock(const BlueprintSection& section,
                                                                   const BlockIterator& cur,
                                                                   BlueprintParserCore& parser,
                                                                   ResourceGroup& group) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur(cur);

            // Group Name
            if (sectionCur == section.bounds.first) {
                
                GetResourceGroupSignature(*cur, group.name);
                result.second = ++sectionCur;
                return result;
            }
            
            // Group Description
            result = ParseDescriptionBlock<ResourceGroup>(section,
                                                           sectionCur,
                                                           parser.sourceData,
                                                           group);
            return result;
            
        }
        
        static ParseSectionResult HandleResource(const BlueprintSection& section,
                                                 const BlockIterator& cur,
                                                 BlueprintParserCore& parser,
                                                 ResourceGroup& group)
        {
            Resource resource;
            ParseSectionResult result = ResourceParser::Parse(cur,
                                                              section.bounds.second,
                                                              section,
                                                              parser,
                                                              resource);
            if (result.first.error.code != Error::OK)
                return result;
            
            ResourceIterator duplicate = FindResource(group, resource);
            ResourceIteratorPair globalDuplicate;
            if (duplicate == group.resources.end())
                globalDuplicate = FindResource(parser.blueprint, resource);

         
            
            if (duplicate != group.resources.end() ||
                globalDuplicate.first != parser.blueprint.resourceGroups.end()) {
                
                // WARN: Duplicate resource
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(cur, section.bounds.second, section.bounds, parser.sourceData);
                result.first.warnings.push_back(Warning("the resource '" +
                                                        resource.uriTemplate +
                                                        "' is already defined",
                                                        DuplicateWarning,
                                                        sourceBlock));
            }


            
            group.resources.push_back(resource); // FIXME: C++11 move
            return result;
        }
    };
    
    typedef BlockParser<ResourceGroup, SectionParser<ResourceGroup> > ResourceGroupParser;
}

#endif
