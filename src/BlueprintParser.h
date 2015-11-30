//
//  BlueprintParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_BLUEPRINTPARSER_H
#define SNOWCRASH_BLUEPRINTPARSER_H

#include <iterator>
#include <algorithm>
#include "ResourceParser.h"
#include "ResourceGroupParser.h"
#include "DataStructureGroupParser.h"
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

            MarkdownNodeIterator cur = node;

            if (pd.sectionContext() == ResourceGroupSectionType ||
                pd.sectionContext() == ResourceSectionType) {

                IntermediateParseResult<ResourceGroup> resourceGroup(out.report);
                cur = ResourceGroupParser::parse(node, siblings, pd, resourceGroup);

                if (isResourceGroupDuplicate(out.node, resourceGroup.node.attributes.name)) {

                    // WARN: duplicate resource group
                    std::stringstream ss;

                    if (resourceGroup.node.attributes.name.empty()) {
                        ss << "anonymous group";
                    } else {
                        ss << "group '" << resourceGroup.node.attributes.name << "'";
                    }

                    ss << " is already defined";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          DuplicateWarning,
                                                          sourceMap));
                }

                out.node.content.elements().push_back(resourceGroup.node);

                if (pd.exportSourceMap()) {
                    out.sourceMap.content.elements().collection.push_back(resourceGroup.sourceMap);
                }
            }
            else if (pd.sectionContext() == DataStructureGroupSectionType) {

                IntermediateParseResult<DataStructureGroup> dataStructureGroup(out.report);
                cur = DataStructureGroupParser::parse(node, siblings, pd, dataStructureGroup);

                out.node.content.elements().push_back(dataStructureGroup.node);

                if (pd.exportSourceMap()) {
                    out.sourceMap.content.elements().collection.push_back(dataStructureGroup.sourceMap);
                }
            }

            return cur;
        }

        /**
         * Look ahead through all the nested sections and gather list of all
         * named types along with their base types and the types they are sub-typed from
         */
        static void preprocessNestedSections(const MarkdownNodeIterator& node,
                                             const MarkdownNodes& siblings,
                                             SectionParserData& pd,
                                             const ParseResultRef<Blueprint>& out) {

            MarkdownNodeIterator cur = node, contextCur;
            SectionType sectionType = UndefinedSectionType;
            SectionType contextSectionType = UndefinedSectionType;

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

                    // If the current node is a Resource or DataStructures section, assign it as context
                    // Otherwise, make sure the current context is not DataStructures section and remove the context
                    if (sectionType == ResourceSectionType ||
                        sectionType == DataStructureGroupSectionType) {

                        contextSectionType = sectionType;
                        contextCur = cur;
                    }
                    else if (contextSectionType != DataStructureGroupSectionType) {

                        contextSectionType = UndefinedSectionType;
                    }

                    // If context is DataStructures section, NamedTypes should be filled
                    if (contextSectionType == DataStructureGroupSectionType) {

                        if (sectionType != MSONSampleDefaultSectionType &&
                            sectionType != MSONPropertyMembersSectionType &&
                            sectionType != MSONValueMembersSectionType &&
                            sectionType != UndefinedSectionType &&
                            sectionType != DataStructureGroupSectionType) {

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
            resolveNamedTypeTables(pd, out.report);
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
            nestedType = SectionProcessor<DataStructureGroup>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            return UndefinedSectionType;
        }

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested;

            // Resource Group & descendants
            nested.push_back(ResourceGroupSectionType);
            nested.push_back(DataStructureGroupSectionType);
            SectionTypes types = SectionProcessor<ResourceGroup>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            return nested;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<Blueprint>& out) {

            checkLazyReferencing(pd, out);
            out.node.element = Element::CategoryElement;

            if (pd.exportSourceMap()) {
                out.sourceMap.element = out.node.element;
            }

            if (!out.node.name.empty())
                return;

            if (pd.options & RequireBlueprintNameOption) {

                // ERR: No API name specified
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.error = Error(ExpectedAPINameMessage, BusinessError, sourceMap);

            }
            else if (!out.node.description.empty()) {

                // WARN: No API name specified
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(ExpectedAPINameMessage, APINameWarning, sourceMap));
            }
        }

        static bool isUnexpectedNode(const MarkdownNodeIterator& node,
                                     SectionType sectionType) {

            // Since Blueprint is currently top-level node any unprocessed node should be reported
            return true;
        }

        /**
         * \brief Fill named type table entries from the signature information.
         *        Both base table and inheritance table.
         *
         * \param node Markdown node to process
         * \param pd Section parser data
         * \param subject Signature of the named type
         * \param report Parse report
         * \param name Name of the named type (only given in case of named resource)
         */
        static void fillNamedTypeTables(const MarkdownNodeIterator& node,
                                        SectionParserData& pd,
                                        const mdp::ByteBuffer& subject,
                                        Report& report,
                                        const mdp::ByteBuffer& name = "") {

            mdp::ByteBuffer buffer = subject;
            mson::Literal identifier;
            mson::TypeDefinition typeDefinition;
            Report tmpReport;

            SignatureTraits traits(SignatureTraits::IdentifierTrait |
                                   SignatureTraits::AttributesTrait);

            Signature signature = SignatureSectionProcessorBase<Blueprint>::parseSignature(node, pd, traits, tmpReport, buffer);
            mson::parseTypeDefinition(node, pd, signature.attributes, tmpReport, typeDefinition);

            // Name of the named types cannot be variable
            if (!name.empty() && mson::checkVariable(signature.identifier)) {
                return;
            }

            if (!name.empty()) {
                identifier = name;
            }
            else {
                identifier = signature.identifier;
            }

            // If named type already exists, return error
            if (pd.namedTypeDependencyTable.find(identifier) != pd.namedTypeDependencyTable.end()) {

                // ERR: Named type is defined more than once
                std::stringstream ss;
                ss << "named type '" << identifier << "' is defined more than once";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                report.error = Error(ss.str(), MSONError, sourceMap);
                return;
            }

            mson::BaseTypeName baseTypeName = typeDefinition.typeSpecification.name.base;

            // Initialize an entry in the dependency table
            pd.namedTypeDependencyTable[identifier] = std::set<mson::Literal>();

            // Add the respective entries to the tables
            if (baseTypeName != mson::UndefinedTypeName) {
                pd.namedTypeBaseTable[identifier] = mson::parseBaseType(typeDefinition.typeSpecification.name.base);

                // Add nested types as dependents
                if (baseTypeName == mson::ArrayTypeName || baseTypeName == mson::EnumTypeName) {

                    for (mson::TypeNames::iterator it = typeDefinition.typeSpecification.nestedTypes.begin();
                         it != typeDefinition.typeSpecification.nestedTypes.end();
                         ++it) {

                        if (!it->symbol.literal.empty() && !it->symbol.variable) {
                            pd.namedTypeDependencyTable[identifier].insert(it->symbol.literal);
                        }
                    }
                }
            }
            else if (!typeDefinition.typeSpecification.name.symbol.literal.empty() &&
                     !typeDefinition.typeSpecification.name.symbol.variable) {

                pd.namedTypeInheritanceTable[identifier] = std::make_pair(typeDefinition.typeSpecification.name.symbol.literal, node->sourceMap);

                // Make the sub type dependent on super type
                pd.namedTypeDependencyTable[identifier].insert(typeDefinition.typeSpecification.name.symbol.literal);
            }
            else if (typeDefinition.typeSpecification.name.empty()) {

                // If there is no specification, an object is assumed
                pd.namedTypeBaseTable[identifier] = mson::ImplicitObjectBaseType;
            }
        }

        /**
         * \brief Resolve named type base table entries from the named type inheritance table
         *
         * \param pd Section parser data
         * \param report Parse report
         */
        static void resolveNamedTypeTables(SectionParserData& pd,
                                           Report& report) {

            mson::NamedTypeInheritanceTable::iterator it;
            mson::NamedTypeDependencyTable::iterator depIt;

            // First resolve dependency tables
            for (depIt = pd.namedTypeDependencyTable.begin();
                 depIt != pd.namedTypeDependencyTable.end();
                 depIt++) {

                resolveNamedTypeDependencyTableEntry(pd, depIt->first, report);
            }

            for (it = pd.namedTypeInheritanceTable.begin();
                 it != pd.namedTypeInheritanceTable.end();
                 it++) {

                resolveNamedTypeBaseTableEntry(pd, it->first, it->second.first, it->second.second, report);

                if (report.error.code != Error::OK) {
                    return;
                }
            }
        }

        /**
         * \brief Resolve the inheritance dependencies of the current named type
         *        (Does not include mixin or member dependencies)
         *
         * \param pd Section parser data
         * \param identifier The named type whose dependents need to be resolved
         * \param report Parse report
         */
        static void resolveNamedTypeDependencyTableEntry(SectionParserData& pd,
                                                         const mson::Literal& identifier,
                                                         Report& report) {

            std::set<mson::Literal> diffDeps, finalDeps, initialDeps;

            do {
                initialDeps = pd.namedTypeDependencyTable[identifier];
                diffDeps.clear();

                for (std::set<mson::Literal>::iterator it = initialDeps.begin();
                     it != initialDeps.end();
                     it++) {

                    std::set<mson::Literal> superTypeDeps = pd.namedTypeDependencyTable[*it];
                    pd.namedTypeDependencyTable[identifier].insert(superTypeDeps.begin(), superTypeDeps.end());
                }

                // Check if the list of dependents has grown
                finalDeps = pd.namedTypeDependencyTable[identifier];
                std::set_difference(finalDeps.begin(), finalDeps.end(), initialDeps.begin(), initialDeps.end(),
                                    std::inserter(diffDeps, diffDeps.end()));

            } while (!diffDeps.empty());
        };

        /**
         * \brief For each entry in the named type inheritance table, resolve the sub-type's base type recursively
         *
         * \param pd Section parser data
         * \param subType The sub named type between the two
         * \param superType The super named type between the two
         * \param report Parse report
         */
        static void resolveNamedTypeBaseTableEntry(SectionParserData& pd,
                                                   const mson::Literal& subType,
                                                   const mson::Literal& superType,
                                                   const mdp::BytesRangeSet& nodeSourceMap,
                                                   Report& report) {

            mson::BaseType baseType;
            mson::NamedTypeBaseTable::iterator it = pd.namedTypeBaseTable.find(subType);

            // If the base table entry is already filled, nothing else to do
            if (it != pd.namedTypeBaseTable.end()) {
                return;
            }

            // Check for circular references
            std::set<mson::Literal> deps = pd.namedTypeDependencyTable[subType];

            if (deps.find(subType) != deps.end()) {

                // ERR: A named type is circularly referenced
                std::stringstream ss;
                ss << "base type '" << subType << "' circularly referencing itself";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(nodeSourceMap, pd.sourceCharacterIndex);
                report.error = Error(ss.str(), MSONError, sourceMap);
                return;
            }

            // Otherwise, get the base type from super type
            it = pd.namedTypeBaseTable.find(superType);

            // If super type is not already resolved, then it means that it is a sub type of something else
            if (it == pd.namedTypeBaseTable.end()) {

                // Try to get the super type of the current super type
                mson::NamedTypeInheritanceTable::iterator inhIt = pd.namedTypeInheritanceTable.find(superType);

                // Check for recursive MSON definitions
                if (inhIt == pd.namedTypeInheritanceTable.end()) {

                    // ERR: We cannot find the super type in inheritance table at all
                    // and there is not base type table entry for it, so, the blueprint is wrong
                    std::stringstream ss;
                    ss << "base type '" << superType << "' is not defined in the document";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(nodeSourceMap, pd.sourceCharacterIndex);
                    report.error = Error(ss.str(), MSONError, sourceMap);
                    return;
                }

                // Recursively, try to get a base type for the current super type
                resolveNamedTypeBaseTableEntry(pd, superType, inhIt->second.first, inhIt->second.second, report);

                if (report.error.code != Error::OK) {
                    return;
                }

                baseType = pd.namedTypeBaseTable.find(superType)->second;
            }
            else {
                baseType = it->second;
            }

            pd.namedTypeBaseTable[subType] = baseType;
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

                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                        out.report.warnings.push_back(Warning(ss.str(),
                                                              DuplicateWarning,
                                                              sourceMap));
                    }
                }
            }
            else if (!out.node.empty()) {

                // WARN: malformed metadata block
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning("ignoring possible metadata, expected '<key> : <value>', one one per line",
                                                      FormattingWarning,
                                                      sourceMap));
            }
        }

        /**
         * \brief Check if a resource group already exists with the given name
         *
         * \param blueprint The blueprint which is formed until now
         * \param name The resource group name to be checked
         */
        static bool isResourceGroupDuplicate(const Blueprint& blueprint,
                                             mdp::ByteBuffer& name) {

            for (Elements::const_iterator it = blueprint.content.elements().begin();
                 it != blueprint.content.elements().end();
                 ++it) {

                if (it->element == Element::CategoryElement &&
                    it->category == Element::ResourceGroupCategory &&
                    it->attributes.name == name) {

                    return true;
                }
            }

            return false;
        }

        /**
         *  \brief  Checks both blueprint and source map AST to resolve references with `Pending` state (Lazy referencing)
         *  \param  pd       Section parser state
         *  \param  out      Processed output
         */
        static void checkLazyReferencing(SectionParserData& pd,
                                         const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<Element> >::iterator elementSourceMapIt;

            if (pd.exportSourceMap()) {
                elementSourceMapIt = out.sourceMap.content.elements().collection.begin();
            }

            for (Elements::iterator elementIt = out.node.content.elements().begin();
                 elementIt != out.node.content.elements().end();
                 ++elementIt) {

                if (elementIt->element == Element::CategoryElement) {
                    checkResourceLazyReferencing(*elementIt, elementSourceMapIt, pd, out);
                }

                if (pd.exportSourceMap()) {
                    elementSourceMapIt++;
                }
            }
        }

        /** Traverses Resource Collection to resolve references with `Pending` state (Lazy referencing) */
        static void checkResourceLazyReferencing(Element& element,
                                                 Collection<SourceMap<Element> >::iterator& elementSourceMap,
                                                 SectionParserData& pd,
                                                 const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<Element> >::iterator resourceElementSourceMapIt;

            if (pd.exportSourceMap()) {
                resourceElementSourceMapIt = elementSourceMap->content.elements().collection.begin();
            }

            for (Elements::iterator resourceElementIt = element.content.elements().begin();
                 resourceElementIt != element.content.elements().end();
                 ++resourceElementIt) {

                if (resourceElementIt->element == Element::ResourceElement) {
                    if (pd.exportSourceMap()) {
                        checkActionLazyReferencing(resourceElementIt->content.resource, resourceElementSourceMapIt->content.resource, pd, out);
                    } else {
                        SourceMap<Resource> tempSourceMap;
                        checkActionLazyReferencing(resourceElementIt->content.resource, tempSourceMap, pd, out);
                    }
                }

                if (pd.exportSourceMap()) {
                    resourceElementSourceMapIt++;
                }
            }
        }

        /** Traverses Action Collection to resolve references with `Pending` state (Lazy referencing) */
        static void checkActionLazyReferencing(Resource& resource,
                                               SourceMap<Resource>& resourceSourceMap,
                                               SectionParserData& pd,
                                               const ParseResultRef<Blueprint>& out) {

            Collection<SourceMap<Action> >::iterator actionSourceMapIt;

            if (pd.exportSourceMap()) {
                actionSourceMapIt = resourceSourceMap.actions.collection.begin();
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
                                                Collection<SourceMap<Action> >::iterator& actionSourceMapIt,
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
                                                Collection<SourceMap<TransactionExample> >::iterator& transactionExampleSourceMapIt,
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
                                                 Collection<SourceMap<TransactionExample> >::iterator& transactionExampleSourceMapIt,
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

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(out.node.reference.meta.node->sourceMap, pd.sourceCharacterIndex);
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
