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
#include "DataStructuresParser.h"
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

                // Nested Sections only, parse as exclusive nested sections
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
            else if (pd.sectionContext() == DataStructuresSectionType) {

                ParseResultRef<DataStructures> dataStructures(out.report, out.node.dataStructures, out.sourceMap.dataStructures);
                return DataStructuresParser::parse(node, siblings, pd, dataStructures);
            }

            return node;
        }

        static void preprocessNestedSections(const MarkdownNodeIterator& node,
                                             const MarkdownNodes& siblings,
                                             SectionParserData& pd,
                                             const ParseResultRef<Blueprint>& out) {

            MarkdownNodeIterator cur = node, contextCur;
            SectionType sectionType, contextSectionType = UndefinedSectionType;

            // Iterate over nested sections
            while (cur != siblings.end()) {

                sectionType = SectionKeywordSignature(cur);

                // Complete Action is recognized as resource section
                if (sectionType == ResourceSectionType) {

                    ActionType actionType = SectionProcessor<Action>::actionType(cur);

                    if (actionType == CompleteActionType) {
                        sectionType = ActionSectionType;
                    }
                }

                if (cur->type == mdp::HeaderMarkdownNodeType) {

                    if (sectionType == ResourceSectionType ||
                        sectionType == DataStructuresSectionType) {

                        contextSectionType = sectionType;
                        contextCur = cur;
                    }
                    else if (contextSectionType != DataStructuresSectionType) {

                        contextSectionType = UndefinedSectionType;
                    }

                    if (contextSectionType == DataStructuresSectionType) {

                        if (sectionType != MSONSampleDefaultSectionType &&
                            sectionType != MSONPropertyMembersSectionType &&
                            sectionType != MSONValueMembersSectionType &&
                            sectionType != UndefinedSectionType &&
                            sectionType != DataStructuresSectionType) {

                            contextSectionType = UndefinedSectionType;
                        }
                        else if (sectionType == UndefinedSectionType) {
                            fillNamedTypeTables(cur, pd, cur->text, out.report);
                        }
                    }
                }
                else if (cur->type == mdp::ListItemMarkdownNodeType &&
                         contextSectionType == ResourceSectionType &&
                         sectionType == AttributesSectionType) {

                    Resource resource;
                    SectionProcessor<Resource>::matchNamedResourceHeader(contextCur, resource);

                    if (!resource.name.empty()) {
                        fillNamedTypeTables(cur, pd, cur->children().front().text, out.report, resource.name);
                    }
                }

                cur++;
            }

            // Resolve all named type base table entries
            resolveNamedTypeTables(pd);
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

            // Check if DataStructures section
            nestedType = SectionProcessor<DataStructures>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            return UndefinedSectionType;
        }

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested;

            // Resource Group & descendants
            nested.push_back(ResourceGroupSectionType);
            nested.push_back(DataStructuresSectionType);
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

        /**
         * \brief Fill named type table entries from the signature information
         *
         * \param node Markdown node to process
         * \param pd Section parser data
         * \param subject Signature of the named type
         * \param report Parse report
         * \param name Name of the named type (only given in case of named resource)
         */
        static void fillNamedTypeTables(const MarkdownNodeIterator& node,
                                        SectionParserData& pd,
                                        const std::string& subject,
                                        Report& report,
                                        const std::string& name = "") {

            std::string buffer = subject, identifier;
            mson::TypeDefinition typeDefinition;

            SignatureTraits traits(SignatureTraits::IdentifierTrait |
                                   SignatureTraits::AttributesTrait);

            Signature signature = SignatureSectionProcessorBase<Blueprint>::parseSignature(node, pd, traits, report, buffer);
            mson::parseTypeDefinition(node, pd, signature.attributes, report, typeDefinition, true);

            // Name of the named types cannot be variable
            if (!name.empty() && mson::checkVariable(signature.identifier)) {
                return;
            }

            if (!name.empty()) {
                identifier = name;
            } else {
                identifier = signature.identifier;
            }

            if (typeDefinition.typeSpecification.name.name != mson::UndefinedTypeName) {
                pd.namedTypeBaseTable[identifier] = mson::parseBaseType(typeDefinition.typeSpecification.name.name);
            }
            else if (!typeDefinition.typeSpecification.name.symbol.literal.empty() &&
                     !typeDefinition.typeSpecification.name.symbol.variable) {

                pd.namedTypeInheritanceTable[identifier] = typeDefinition.typeSpecification.name.symbol.literal;
            }
            else if (typeDefinition.typeSpecification.name.empty()) {
                pd.namedTypeBaseTable[identifier] = mson::ImplicitPropertyBaseType;
            }
        }

        /**
         * \brief Resolve named type base table entries from the named type inheritance table
         *
         * \param pd Section parser data
         */
        static void resolveNamedTypeTables(SectionParserData& pd) {

            mson::NamedTypeInheritanceTable::iterator it;
            mson::NamedTypeBaseTable::iterator baseIt;

            for (it = pd.namedTypeInheritanceTable.begin();
                 it != pd.namedTypeInheritanceTable.end();
                 it++) {

                baseIt = pd.namedTypeBaseTable.find(it->second);

                if (baseIt != pd.namedTypeBaseTable.end()) {
                    pd.namedTypeBaseTable[it->first] = baseIt->second;
                }
            }
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

            Collection<SourceMap<ResourceGroup> >::iterator resourceGroupSourceMapIt;

            if (pd.exportSourceMap()) {
                resourceGroupSourceMapIt = out.sourceMap.resourceGroups.collection.begin();
            }

            for (ResourceGroups::iterator resourceGroupIt = out.node.resourceGroups.begin();
                 resourceGroupIt != out.node.resourceGroups.end();
                 ++resourceGroupIt) {

                checkResourceLazyReferencing(*resourceGroupIt, resourceGroupSourceMapIt, pd, out);

                if (pd.exportSourceMap()) {
                    resourceGroupSourceMapIt++;
                }
            }
        }

        /** Traverses Resource Collection to resolve references with `Pending` state (Lazy referencing) */
        static void checkResourceLazyReferencing(ResourceGroup& resourceGroup,
                                                 Collection<SourceMap<ResourceGroup> >::iterator resourceGroupSourceMapIt,
                                                 SectionParserData& pd,
                                                 const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<Resource> >::iterator resourceSourceMapIt;

            if (pd.exportSourceMap()) {
                resourceSourceMapIt = resourceGroupSourceMapIt->resources.collection.begin();
            }

            for (Resources::iterator resourceIt = resourceGroup.resources.begin();
                 resourceIt != resourceGroup.resources.end();
                 ++resourceIt) {

                checkActionLazyReferencing(*resourceIt, resourceSourceMapIt, pd, out);

                if (pd.exportSourceMap()) {
                    resourceSourceMapIt++;
                }
            }
        }

        /** Traverses Action Collection to resolve references with `Pending` state (Lazy referencing) */
        static void checkActionLazyReferencing(Resource& resource,
                                               Collection<SourceMap<Resource> >::iterator resourceSourceMapIt,
                                               SectionParserData& pd,
                                               const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<Action> >::iterator actionSourceMapIt;

            if (pd.exportSourceMap()) {
                actionSourceMapIt = resourceSourceMapIt->actions.collection.begin();
            }

            for (Actions::iterator actionIt = resource.actions.begin();
                 actionIt != resource.actions.end();
                 ++actionIt) {

                checkExampleLazyReferencing(*actionIt, actionSourceMapIt, pd, out);

                if (pd.exportSourceMap()) {
                    actionSourceMapIt++;
                }
            }
        }

        /** Traverses Transaction Example Collection AST to resolve references with `Pending` state (Lazy referencing) */
        static void checkExampleLazyReferencing(Action& action,
                                                Collection<SourceMap<Action> >::iterator actionSourceMapIt,
                                                SectionParserData& pd,
                                                const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<TransactionExample> >::iterator exampleSourceMapIt;

            if (pd.exportSourceMap()) {
                exampleSourceMapIt = actionSourceMapIt->examples.collection.begin();
            }

            for (TransactionExamples::iterator transactionExampleIt = action.examples.begin();
                 transactionExampleIt != action.examples.end();
                 ++transactionExampleIt) {

                checkRequestLazyReferencing(*transactionExampleIt, exampleSourceMapIt, pd, out);
                checkResponseLazyReferencing(*transactionExampleIt, exampleSourceMapIt, pd, out);

                if (pd.exportSourceMap()) {
                    exampleSourceMapIt++;
                }
            }
        }

        /** Traverses Request Collection to resolve references with `Pending` state (Lazy referencing) */
        static void checkRequestLazyReferencing(TransactionExample& transactionExample,
                                                Collection<SourceMap<TransactionExample> >::iterator transactionExampleSourceMapIt,
                                                SectionParserData& pd,
                                                const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<Request> >::iterator requestSourceMapIt;

            if (pd.exportSourceMap()) {
                requestSourceMapIt = transactionExampleSourceMapIt->requests.collection.begin();
            }

            for (Requests::iterator requestIt = transactionExample.requests.begin();
                 requestIt != transactionExample.requests.end();
                 ++requestIt) {

                if (!requestIt->reference.id.empty() &&
                    requestIt->reference.meta.state == Reference::StatePending) {

                    if (pd.exportSourceMap()) {

                        ParseResultRef<Payload> payload(out.report, *requestIt, *requestSourceMapIt);
                        resolvePendingModels(pd, payload);
                        SectionProcessor<Payload>::checkRequest(requestIt->reference.meta.node, pd, payload);
                    }
                    else {

                        SourceMap<Payload> tempSourceMap;
                        ParseResultRef<Payload> payload(out.report, *requestIt, tempSourceMap);
                        resolvePendingModels(pd, payload);
                        SectionProcessor<Payload>::checkRequest(requestIt->reference.meta.node, pd, payload);
                    }
                }

                if (pd.exportSourceMap()) {
                    requestSourceMapIt++;
                }
            }
        }

        /** Traverses Response Collection to resolve references with `Pending` state (Lazy referencing) */
        static void checkResponseLazyReferencing(TransactionExample& transactionExample,
                                                 Collection<SourceMap<TransactionExample> >::iterator transactionExampleSourceMapIt,
                                                 SectionParserData& pd,
                                                 const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<Response> >::iterator responseSourceMapIt;

            if (pd.exportSourceMap()) {
                responseSourceMapIt = transactionExampleSourceMapIt->responses.collection.begin();
            }

            for (Responses::iterator responseIt = transactionExample.responses.begin();
                 responseIt != transactionExample.responses.end();
                 ++responseIt) {

                if (!responseIt->reference.id.empty() &&
                    responseIt->reference.meta.state == Reference::StatePending) {

                    if (pd.exportSourceMap()) {

                        ParseResultRef<Payload> payload(out.report, *responseIt, *responseSourceMapIt);
                        resolvePendingModels(pd, payload);
                        SectionProcessor<Payload>::checkResponse(responseIt->reference.meta.node, pd, payload);
                    }
                    else {

                        SourceMap<Payload> tempSourceMap;
                        ParseResultRef<Payload> payload(out.report, *responseIt, tempSourceMap);
                        resolvePendingModels(pd, payload);
                        SectionProcessor<Payload>::checkResponse(responseIt->reference.meta.node, pd, payload);
                    }
                }

                if (pd.exportSourceMap()) {
                    responseSourceMapIt++;
                }
            }
        }

        /**
         *  \brief  Resolve pending model references
         *  \param  pd       Section parser data
         *  \param  out      Processed output
         */
        static void resolvePendingModels(SectionParserData& pd,
                                          const ParseResultRef<Payload>& out) {

            if (pd.modelTable.find(out.node.reference.id) == pd.modelTable.end()) {

                // ERR: Undefined model reference
                std::stringstream ss;
                ss << "Undefined resource model " << out.node.reference.id;

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(out.node.reference.meta.node->sourceMap, pd.sourceData);
                out.report.error = Error(ss.str(), ModelError, sourceMap);

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
