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
     
            checkLazyReferencing(pd, out);

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

        /**
         *  \brief  Checks both blueprint and source map AST to resolve references with `Pending` state (Lazy referencing)
         *  \param  pd       Section parser state
         *  \param  out      Processed output
         */
        static void checkLazyReferencing(SectionParserData& pd,
                                         const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<ResourceGroup> >::iterator resourceGroupSourceMapIterator;

            if (pd.exportSourceMap()) {
                resourceGroupSourceMapIterator = out.sourceMap.resourceGroups.collection.begin();
            }

            for (ResourceGroups::iterator resourceGroupIterator = out.node.resourceGroups.begin();
                 resourceGroupIterator != out.node.resourceGroups.end();
                 ++resourceGroupIterator, pd.exportSourceMap() ? ++resourceGroupSourceMapIterator : resourceGroupSourceMapIterator) {

                checkResourceLazyReferencing(*resourceGroupIterator, *resourceGroupSourceMapIterator, pd, out);
            }
        }

        /** Traverses Resource Collection to resolve references with `Pending` state (Lazy referencing) */
        static void checkResourceLazyReferencing(ResourceGroup& resourceGroup,
                                                 SourceMap<ResourceGroup>& resourceGroupSourceMap,
                                                 SectionParserData& pd,
                                                 const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<Resource> >::iterator resourceSourceMapIterator;

            if (pd.exportSourceMap()) {
                resourceSourceMapIterator = resourceGroupSourceMap.resources.collection.begin();
            }

            for (Resources::iterator resourceIterator = resourceGroup.resources.begin();
                 resourceIterator != resourceGroup.resources.end();
                 ++resourceIterator, pd.exportSourceMap() ? ++resourceSourceMapIterator : resourceSourceMapIterator) {

                checkActionLazyReferencing(*resourceIterator, *resourceSourceMapIterator, pd, out);
            }
        }

        /** Traverses Action Collection to resolve references with `Pending` state (Lazy referencing) */
        static void checkActionLazyReferencing(Resource& resource,
                                               SourceMap<Resource>& resourceSourceMap,
                                               SectionParserData& pd,
                                               const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<Action> >::iterator actionSourceMapIterator;

            if (pd.exportSourceMap()) {
                actionSourceMapIterator = resourceSourceMap.actions.collection.begin();
            }

            for (Actions::iterator actionIterator = resource.actions.begin();
                 actionIterator != resource.actions.end();
                 ++actionIterator, pd.exportSourceMap() ? ++actionSourceMapIterator : actionSourceMapIterator) {

                checkExampleLazyReferencing(*actionIterator, *actionSourceMapIterator, pd, out);
            }
        }

        /** Traverses Transaction Example Collection AST to resolve references with `Pending` state (Lazy referencing) */
        static void checkExampleLazyReferencing(Action& action,
                                                SourceMap<Action>& actionSourceMap,
                                                SectionParserData& pd,
                                                const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<TransactionExample> >::iterator exampleSourceMapIterator;

            if (pd.exportSourceMap()) {
                exampleSourceMapIterator = actionSourceMap.examples.collection.begin();
            }

            for (TransactionExamples::iterator transactionExampleIterator = action.examples.begin();
                 transactionExampleIterator != action.examples.end();
                 ++transactionExampleIterator, pd.exportSourceMap() ? ++exampleSourceMapIterator : exampleSourceMapIterator) {

                checkRequestLazyReferencing(*transactionExampleIterator, *exampleSourceMapIterator, pd, out);
                checkResponseLazyReferencing(*transactionExampleIterator, *exampleSourceMapIterator, pd, out);
            }
        }

        /** Traverses Request Collection to resolve references with `Pending` state (Lazy referencing) */
        static void checkRequestLazyReferencing(TransactionExample& transactionExample,
                                                SourceMap<TransactionExample>& transactionExampleSourceMap,
                                                SectionParserData& pd,
                                                const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<Request> >::iterator requestSourceMapIterator;

            if (pd.exportSourceMap()) {
                requestSourceMapIterator = transactionExampleSourceMap.requests.collection.begin();
            }

            for (Requests::iterator requestIterator = transactionExample.requests.begin();
                 requestIterator != transactionExample.requests.end();
                 ++requestIterator, pd.exportSourceMap() ? ++requestSourceMapIterator : requestSourceMapIterator) {

                if (!requestIterator->reference.id.empty() &&
                    requestIterator->reference.meta.state == Reference::StatePending) {

                    ParseResultRef<Payload> payload(out.report, *requestIterator, *requestSourceMapIterator);
                    resolvePendingSymbols(pd, payload);
                }
            }
        }

        /** Traverses Response Collection to resolve references with `Pending` state (Lazy referencing) */
        static void checkResponseLazyReferencing(TransactionExample& transactionExample,
                                                 SourceMap<TransactionExample>& transactionExampleSourceMap,
                                                 SectionParserData& pd,
                                                 const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<Response> >::iterator responseSourceMapIterator;

            if (pd.exportSourceMap()) {
                responseSourceMapIterator = transactionExampleSourceMap.responses.collection.begin();
            }

            for (Responses::iterator responseIterator = transactionExample.responses.begin();
                 responseIterator != transactionExample.responses.end();
                 ++responseIterator, pd.exportSourceMap() ? ++responseSourceMapIterator : responseSourceMapIterator) {

                if (!responseIterator->reference.id.empty() &&
                    responseIterator->reference.meta.state == Reference::StatePending) {

                    ParseResultRef<Payload> payload(out.report, *responseIterator, *responseSourceMapIterator);
                    resolvePendingSymbols(pd, payload);
                }
            }
        }

        /**
         *  \brief  Resolve pending references
         *  \param  pd       Section parser state
         *  \param  out      Processed output
         */
        static void resolvePendingSymbols(SectionParserData& pd,
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
                SectionProcessor<Payload>::assingReferredPayload(pd, out);
            }
        }
    };

    /** Blueprint Parser */
    typedef SectionParser<Blueprint, BlueprintSectionAdapter> BlueprintParser;
}

#endif
