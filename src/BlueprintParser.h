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

    /** Internal type alias for Collection of Metadata */
    typedef Collection<Metadata>::type MetadataCollection;

    typedef Collection<Metadata>::iterator MetadataCollectionIterator;

    /**
     * Blueprint processor
     */
    template<>
    struct SectionProcessor<Blueprint> : public SectionProcessorBase<Blueprint> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     bool& parsingRedirect,
                                                     Report& report,
                                                     Blueprint& out) {

            MarkdownNodeIterator cur = node;

            if (cur->type == mdp::ParagraphMarkdownNodeType) {

                parseMetadata(node, pd, report, out.metadata);

                // First block is paragraph and is not metadata (no API name)
                if (out.metadata.empty()) {
                    return processDescription(cur, pd, report, out);
                }

                cur++;
            }

            if (cur->type == mdp::HeaderMarkdownNodeType) {

                SectionType nestedType = nestedSectionType(cur);

                // If starting with Resource or Resource Group
                if (nestedType != UndefinedSectionType) {

                    pd.sectionsContext.push_back(nestedType);
                    cur = processNestedSection(cur, cur->parent().children(), pd, report, out);
                    pd.sectionsContext.pop_back();

                    return cur;
                }

                out.name = cur->text;
                TrimString(out.name);
            } else {

                // Any other type of block, add to description
                return processDescription(cur, pd, report, out);
            }

            return ++MarkdownNodeIterator(cur);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         Blueprint& out) {

            if (pd.sectionContext() == ResourceGroupSectionType ||
                pd.sectionContext() == ResourceSectionType ||
                pd.sectionContext() == ResourceMethodSectionType) {

                ResourceGroup resourceGroup;
                MarkdownNodeIterator cur = ResourceGroupParser::parse(node, siblings, pd, report, resourceGroup);

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
            nestedType = SectionProcessor<Resource>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            // Check if ResourceGroup section
            nestedType = SectionProcessor<ResourceGroup>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            return UndefinedSectionType;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             Report& report,
                             Blueprint& out) {

            if (out.name.empty()) {

                if (pd.options & RequireBlueprintNameOption) {

                    // ERR: No API name specified
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.error = Error(ExpectedAPINameMessage,
                                         BusinessError,
                                         sourceMap);
                } else {

                    // WARN: No API name
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(Warning(ExpectedAPINameMessage,
                                                      APINameWarning,
                                                      sourceMap));
                }
            }
        }

        static void parseMetadata(const MarkdownNodeIterator& node,
                                  SectionParserData& pd,
                                  Report& report,
                                  MetadataCollection& out) {

            mdp::ByteBuffer content = node->text;
            TrimStringEnd(content);

            std::vector<mdp::ByteBuffer> lines = Split(content, '\n');

            for (std::vector<mdp::ByteBuffer>::iterator it = lines.begin();
                 it != lines.end();
                 ++it) {

                Metadata metadata;

                if (CodeBlockUtility::keyValueFromLine(*it, metadata)) {
                    out.push_back(metadata);
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
    typedef SectionParser<Blueprint, BlueprintSectionAdapter> BlueprintParser;
}

#endif
