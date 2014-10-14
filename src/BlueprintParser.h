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
    struct SectionProcessor<Blueprint> : public SectionProcessorBase<Blueprint> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     const ParseResultRef<Blueprint>& out) {

            MarkdownNodeIterator cur = node;

            while (cur != siblings.end() &&
                   cur->type == mdp::ParagraphMarkdownNodeType) {

                IntermediateParseResult<MetadataCollection> metadata(out.report);

                parseMetadata(cur, pd, metadata);

                // First block is paragraph and is not metadata (no API name)
                if (metadata.node.empty()) {
                    return processDescription(cur, siblings, pd, out);
                } else {
                    out.node.metadata.insert(out.node.metadata.end(), metadata.node.begin(), metadata.node.end());

                    if (pd.exportSourceMap()) {
                        out.sourceMap.metadata.collection.insert(out.sourceMap.metadata.collection.end(),
                                                                 metadata.sourceMap.collection.begin(),
                                                                 metadata.sourceMap.collection.end());
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

                out.node.name = cur->text;
                TrimString(out.node.name);

                if (pd.exportSourceMap() && !out.node.name.empty()) {
                    out.sourceMap.name.sourceMap = cur->sourceMap;
                }
            } else {

                // Any other type of block, add to description
                return processDescription(cur, siblings, pd, out);
            }

            return ++MarkdownNodeIterator(cur);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<Blueprint>& out) {

            if (pd.sectionContext() == ResourceGroupSectionType ||
                pd.sectionContext() == ResourceSectionType) {

                IntermediateParseResult<ResourceGroup> resourceGroup(out.report);

                MarkdownNodeIterator cur = ResourceGroupParser::parse(node, siblings, pd, resourceGroup);

                ResourceGroupIterator duplicate = findResourceGroup(out.node.resourceGroups, resourceGroup.node);

                if (duplicate != out.node.resourceGroups.end()) {

                    // WARN: duplicate resource group
                    std::stringstream ss;

                    if (resourceGroup.node.name.empty()) {
                        ss << "anonymous group";
                    } else {
                        ss << "group '" << resourceGroup.node.name << "'";
                    }

                    ss << " is already defined";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          DuplicateWarning,
                                                          sourceMap));
                }

                out.node.resourceGroups.push_back(resourceGroup.node);

                if (pd.exportSourceMap()) {
                    out.sourceMap.resourceGroups.collection.push_back(resourceGroup.sourceMap);
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

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested;

            // Resource Group & descendants
            nested.push_back(ResourceGroupSectionType);
            SectionTypes types = SectionProcessor<ResourceGroup>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            return nested;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<Blueprint>& out) {
     
            checkLazyReferencing(node, pd, out);

            if (!out.node.name.empty())
                return;

            if (pd.options & RequireBlueprintNameOption) {

                // ERR: No API name specified
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.error = Error(ExpectedAPINameMessage,
                                         BusinessError,
                                         sourceMap);

            }
            else if (!out.node.description.empty()) {
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.warnings.push_back(Warning(ExpectedAPINameMessage,
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
                                  const ParseResultRef<MetadataCollection>& out) {

            mdp::ByteBuffer content = node->text;
            TrimStringEnd(content);

            std::vector<mdp::ByteBuffer> lines = Split(content, '\n');

            for (std::vector<mdp::ByteBuffer>::iterator it = lines.begin();
                 it != lines.end();
                 ++it) {

                Metadata metadata;

                if (CodeBlockUtility::keyValueFromLine(*it, metadata)) {
                    out.node.push_back(metadata);

                    if (pd.exportSourceMap()) {
                        SourceMap<Metadata> metadataSM;
                        metadataSM.sourceMap = node->sourceMap;
                        out.sourceMap.collection.push_back(metadataSM);
                    }
                }
            }

            if (lines.size() == out.node.size()) {

                // Check duplicates
                std::vector<mdp::ByteBuffer> duplicateKeys;

                for (MetadataCollectionIterator it = out.node.begin();
                     it != out.node.end();
                     ++it) {

                    MetadataCollectionIterator from = it;
                    if (++from == out.node.end())
                        break;

                    MetadataCollectionIterator duplicate = std::find_if(from,
                                                                        out.node.end(),
                                                                        std::bind2nd(MatchFirsts<Metadata>(), *it));

                    if (duplicate != out.node.end() &&
                        std::find(duplicateKeys.begin(), duplicateKeys.end(), it->first) == duplicateKeys.end()) {

                        duplicateKeys.push_back(it->first);

                        // WARN: duplicate metadata definition
                        std::stringstream ss;
                        ss << "duplicate definition of '" << it->first << "'";

                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        out.report.warnings.push_back(Warning(ss.str(),
                                                              DuplicateWarning,
                                                              sourceMap));
                    }
                }
            }
            else if (!out.node.empty()) {

                // WARN: malformed metadata block
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.warnings.push_back(Warning("ignoring possible metadata, expected '<key> : <value>', one one per line",
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

        /** check for lazy referencing */
        static void checkLazyReferencing(const MarkdownNodeIterator& node,
                                         SectionParserData& pd,
                                         const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<ResourceGroup> >::iterator resourceGroupSourceMapIterator;
            Collection<SourceMap<Resource> >::iterator resourceSourceMapIterator;
            Collection<SourceMap<Action> >::iterator actionSourceMapIterator;
            Collection<SourceMap<TransactionExample> >::iterator exampleSourceMapIterator;
            Collection<SourceMap<Request> >::iterator requestSourceMapIterator;
            Collection<SourceMap<Response> >::iterator responseSourceMapIterator;

            bool exportSourceMap = pd.exportSourceMap();

            resourceGroupSourceMapIterator = out.sourceMap.resourceGroups.collection.begin();

            for (ResourceGroups::iterator resourceGroupIterator = out.node.resourceGroups.begin();
                 resourceGroupIterator != out.node.resourceGroups.end();
                 ++resourceGroupIterator, exportSourceMap ? ++resourceGroupSourceMapIterator : resourceGroupSourceMapIterator) {

                if (exportSourceMap) {
                    resourceSourceMapIterator = resourceGroupSourceMapIterator->resources.collection.begin();
                }

                for (Resources::iterator resourceIterator = resourceGroupIterator->resources.begin();
                     resourceIterator != resourceGroupIterator->resources.end();
                     ++resourceIterator, exportSourceMap ? ++resourceSourceMapIterator : resourceSourceMapIterator) {

                    if (exportSourceMap) {
                        actionSourceMapIterator = resourceSourceMapIterator->actions.collection.begin();
                    }

                    for (Actions::iterator actionIterator = resourceIterator->actions.begin();
                         actionIterator != resourceIterator->actions.end();
                         ++actionIterator, exportSourceMap ? ++actionSourceMapIterator : actionSourceMapIterator) {

                        if (exportSourceMap) {
                            exampleSourceMapIterator = actionSourceMapIterator->examples.collection.begin();
                        }

                        for (TransactionExamples::iterator transactionExampleIterator = actionIterator->examples.begin();
                             transactionExampleIterator != actionIterator->examples.end();
                             ++transactionExampleIterator, exportSourceMap ? ++exampleSourceMapIterator : exampleSourceMapIterator) {

                            if (exportSourceMap) {
                                requestSourceMapIterator = exampleSourceMapIterator->requests.collection.begin();
                            }

                            for (Requests::iterator requestIterator = transactionExampleIterator->requests.begin();
                                 requestIterator != transactionExampleIterator->requests.end();
                                 ++requestIterator, exportSourceMap ? ++requestSourceMapIterator : requestSourceMapIterator) {

                                if (!requestIterator->reference.id.empty() &&
                                    requestIterator->reference.meta.state == Reference::StatePending) {

                                    ParseResultRef<Payload> payload(out.report, *requestIterator, *requestSourceMapIterator);
                                    resolvePendingSymbols(node, pd, payload);
                                }
                            }

                            if (exportSourceMap) {
                                responseSourceMapIterator = exampleSourceMapIterator->responses.collection.begin();
                            }

                            for (Responses::iterator responseIterator = transactionExampleIterator->responses.begin();
                                 responseIterator != transactionExampleIterator->responses.end();
                                 ++responseIterator, exportSourceMap ? ++responseSourceMapIterator : responseSourceMapIterator) {

                                if (!responseIterator->reference.id.empty() &&
                                    responseIterator->reference.meta.state == Reference::StatePending) {

                                    ParseResultRef<Payload> payload(out.report, *responseIterator, *responseSourceMapIterator);
                                    resolvePendingSymbols(node, pd, payload);
                                }
                            }
                        }
                    }
                }
            }
        }

        /** Resolve pending resources */
        static void resolvePendingSymbols(const MarkdownNodeIterator& node,
                                          SectionParserData& pd,
                                          const ParseResultRef<Payload>& out) {

            if (pd.symbolTable.resourceModels.find(out.node.reference.id) == pd.symbolTable.resourceModels.end()) {

                // ERR: Undefined symbol
                std::stringstream ss;
                ss << "Undefined symbol " << out.node.reference.id;

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(out.node.reference.meta.node->sourceMap, pd.sourceData);
                out.report.error = Error(ss.str(), SymbolError, sourceMap);

                out.node.reference.meta.state = Reference::StateUnresolved;
            }
            else {

                out.node.reference.meta.state = Reference::StateResolved;
                SectionProcessor<Payload>::assignSymbolToPayload(pd, out);
            }
        }
    };

    /** Blueprint Parser */
    typedef SectionParser<Blueprint, BlueprintSectionAdapter> BlueprintParser;
}

#endif
