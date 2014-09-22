//
//  BlueprintParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_BLUEPRINTPARSER_H
#define SNOWCRASH_BLUEPRINTPARSER_H

#include "ResourceParser.h"
#include "ResourceGroupParser.h"
#include "SectionParser.h"
#include "RegexMatch.h"
#include "CodeBlockUtility.h"

namespace snowcrash {

    const char* const ExpectedAPINameMessage = "expected API name, e.g. '# <API Name>'";

    /** Internal type alias for Collection iterator of Metadata */
    typedef Collection<Metadata>::iterator MetadataCollectionIterator;

    /**
     * Blueprint processor
     */
    template<>
    struct SectionProcessor<Blueprint, BlueprintSM> : public SectionProcessorBase<Blueprint, BlueprintSM> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     Report& report,
                                                     Blueprint& out,
                                                     BlueprintSM& outSM) {

            MarkdownNodeIterator cur = node;

            while (cur != siblings.end() &&
                   cur->type == mdp::ParagraphMarkdownNodeType) {

                MetadataCollection metadata;
                MetadataCollectionSM metadataSM;
                parseMetadata(cur, pd, report, metadata, metadataSM);

                // First block is paragraph and is not metadata (no API name)
                if (metadata.empty()) {
                    return processDescription(cur, siblings, pd, report, out, outSM);
                } else {
                    out.metadata.insert(out.metadata.end(), metadata.begin(), metadata.end());

                    if (pd.exportSM()) {
                        outSM.metadata.insert(outSM.metadata.end(), metadataSM.begin(), metadataSM.end());
                    }
                }

                cur++;
            }
            
            // Ideally this parsing metadata should be handled by separate parser
            // that way the following check would be covered in SectionParser::parse()
            if (cur == siblings.end())
                return cur;

            if (cur->type == mdp::HeaderMarkdownNodeType) {

                SectionType nestedType = nestedSectionType(cur);

                // Resources Groups only, parse as exclusive nested sections
                if (nestedType != UndefinedSectionType) {
                    layout = ExclusiveNestedSectionLayout;
                    return cur;
                }

                out.name = cur->text;
                TrimString(out.name);

                if (pd.exportSM() && !out.name.empty()) {
                    outSM.name = cur->sourceMap;
                }
            } else {

                // Any other type of block, add to description
                return processDescription(cur, siblings, pd, report, out, outSM);
            }

            return ++MarkdownNodeIterator(cur);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         Blueprint& out,
                                                         BlueprintSM& outSM) {

            if (pd.sectionContext() == ResourceGroupSectionType ||
                pd.sectionContext() == ResourceSectionType) {

                ResourceGroup resourceGroup;
                ResourceGroupSM resourceGroupSM;

                MarkdownNodeIterator cur = ResourceGroupParser::parse(node, siblings, pd, report, resourceGroup, resourceGroupSM);

                ResourceGroupIterator duplicate = findResourceGroup(out.resourceGroups, resourceGroup);

                if (duplicate != out.resourceGroups.end()) {

                    // WARN: duplicate resource group
                    std::stringstream ss;

                    if (resourceGroup.name.empty()) {
                        ss << "anonymous group";
                    } else {
                        ss << "group '" << resourceGroup.name << "'";
                    }

                    ss << " is already defined";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(Warning(ss.str(),
                                                      DuplicateWarning,
                                                      sourceMap));
                }

                out.resourceGroups.push_back(resourceGroup);

                if (pd.exportSM()) {
                    outSM.resourceGroups.push_back(resourceGroupSM);
                }

                return cur;
            }

            return node;
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            return BlueprintSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            SectionType nestedType = UndefinedSectionType;

            // Check if Resource section
            nestedType = SectionProcessor<Resource, ResourceSM>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            // Check if ResourceGroup section
            nestedType = SectionProcessor<ResourceGroup, ResourceGroupSM>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            return UndefinedSectionType;
        }

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested;

            // Resource Group & descendants
            nested.push_back(ResourceGroupSectionType);
            SectionTypes types = SectionProcessor<ResourceGroup, ResourceGroupSM>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            return nested;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             Report& report,
                             Blueprint& out,
                             BlueprintSM& outSM) {
            
            if (!out.name.empty())
                return;

            if (pd.options & RequireBlueprintNameOption) {

                // ERR: No API name specified
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.error = Error(ExpectedAPINameMessage,
                                     BusinessError,
                                     sourceMap);
                
            }
            else if (!out.description.empty()) {
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning(ExpectedAPINameMessage,
                                                  APINameWarning,
                                                  sourceMap));
            }
        }

        static bool isUnexpectedNode(const MarkdownNodeIterator& node,
                                     SectionType sectionType) {
            
            // Since Blueprint is currently top-level node any unprocessed node should be reported
            return true;
        }


        static void parseMetadata(const MarkdownNodeIterator& node,
                                  SectionParserData& pd,
                                  Report& report,
                                  MetadataCollection& out,
                                  MetadataCollectionSM& outSM) {

            mdp::ByteBuffer content = node->text;
            TrimStringEnd(content);

            std::vector<mdp::ByteBuffer> lines = Split(content, '\n');

            for (std::vector<mdp::ByteBuffer>::iterator it = lines.begin();
                 it != lines.end();
                 ++it) {

                Metadata metadata;

                if (CodeBlockUtility::keyValueFromLine(*it, metadata)) {
                    out.push_back(metadata);

                    if (pd.exportSM()) {
                        outSM.push_back(node->sourceMap);
                    }
                }
            }

            if (lines.size() == out.size()) {

                // Check duplicates
                std::vector<mdp::ByteBuffer> duplicateKeys;

                for (MetadataCollectionIterator it = out.begin();
                     it != out.end();
                     ++it) {

                    MetadataCollectionIterator from = it;
                    if (++from == out.end())
                        break;

                    MetadataCollectionIterator duplicate = std::find_if(from,
                                                                        out.end(),
                                                                        std::bind2nd(MatchFirsts<Metadata>(), *it));

                    if (duplicate != out.end() &&
                        std::find(duplicateKeys.begin(), duplicateKeys.end(), it->first) == duplicateKeys.end()) {

                        duplicateKeys.push_back(it->first);

                        // WARN: duplicate metadata definition
                        std::stringstream ss;
                        ss << "duplicate definition of '" << it->first << "'";

                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        report.warnings.push_back(Warning(ss.str(),
                                                          DuplicateWarning,
                                                          sourceMap));
                    }
                }
            }
            else if (!out.empty()) {

                // WARN: malformed metadata block
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning("ignoring possible metadata, expected '<key> : <value>', one one per line",
                                                  FormattingWarning,
                                                  sourceMap));
            }
        }

        /** Finds a resource group inside an resource groups collection */
        static ResourceGroupIterator findResourceGroup(const ResourceGroups& resourceGroups,
                                                       const ResourceGroup& resourceGroup) {

            return std::find_if(resourceGroups.begin(),
                                resourceGroups.end(),
                                std::bind2nd(MatchName<ResourceGroup>(), resourceGroup));
        }
    };

    /** Blueprint Parser */
    typedef SectionParser<Blueprint, BlueprintSM, BlueprintSectionAdapter> BlueprintParser;
}

#endif
