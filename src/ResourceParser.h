//
//  ResourceParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_RESOURCEPARSER_H
#define SNOWCRASH_RESOURCEPARSER_H

#include "SectionParser.h"
#include "ActionParser.h"
#include "DataStructureGroupParser.h"
#include "HeadersParser.h"
#include "ParametersParser.h"
#include "UriTemplateParser.h"
#include "RegexMatch.h"

namespace snowcrash {

    /** Nameless resource matching regex */
    const char* const ResourceHeaderRegex = "^[[:blank:]]*(" HTTP_REQUEST_METHOD "[[:blank:]]+)?" URI_TEMPLATE "$";

    /** Named resource matching regex */
    const char* const NamedResourceHeaderRegex = "^[[:blank:]]*" SYMBOL_IDENTIFIER "[[:blank:]]+\\[" URI_TEMPLATE "]$";

    /** Named endpoint matching regex */
    const char* const NamedEndpointHeaderRegex = "^[[:blank:]]*" SYMBOL_IDENTIFIER "[[:blank:]]+\\[" HTTP_REQUEST_METHOD "[[:blank:]]+" URI_TEMPLATE "]$";

    /** Internal type alias for Collection iterator of Resource */
    typedef Collection<Resource>::const_iterator ResourceIterator;

    /**
     * Resource Section processor
     */
    template<>
    struct SectionProcessor<Resource> : public SectionProcessorBase<Resource> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     const ParseResultRef<Resource>& out) {

            CaptureGroups captureGroups;

            // If Abbreviated resource section
            if (RegexCapture(node->text, ResourceHeaderRegex, captureGroups, 4)) {

                out.node.uriTemplate = captureGroups[3];

                // Make this section an action
                if (!captureGroups[2].empty()) {
                    return processNestedAction(node, node->parent().children(), pd, layout, out);
                }
            } else if (RegexCapture(node->text, NamedEndpointHeaderRegex, captureGroups, 5)) {

                out.node.name = captureGroups[1];
                TrimString(out.node.name);
                out.node.uriTemplate = captureGroups[3];

                return processNestedAction(node, node->parent().children(), pd, layout, out);
            } else {
                matchNamedResourceHeader(node, out.node);
            }

            if (pd.exportSourceMap()) {
                if (!out.node.uriTemplate.empty()) {
                    out.sourceMap.uriTemplate.sourceMap = node->sourceMap;
                }

                if (!out.node.name.empty()) {
                    out.sourceMap.name.sourceMap = node->sourceMap;
                }
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<Resource>& out) {

            switch (pd.sectionContext()) {
                case ActionSectionType:
                    return processAction(node, siblings, pd, out);

                case ParametersSectionType:
                    return processParameters(node, siblings, pd, out);

                case ModelSectionType:
                case ModelBodySectionType:
                    return processModel(node, siblings, pd, out);

                case HeadersSectionType:
                {
                    ParseResultRef<Headers> headers(out.report, out.node.headers, out.sourceMap.headers);
                    return SectionProcessor<Action>::handleDeprecatedHeaders(node, siblings, pd, headers);
                }

                case AttributesSectionType:
                {
                    // Set up named type context
                    if (!out.node.name.empty()) {
                        pd.namedTypeContext = out.node.name;
                    }

                    ParseResultRef<Attributes> attributes(out.report, out.node.attributes, out.sourceMap.attributes);
                    MarkdownNodeIterator cur = AttributesParser::parse(node, siblings, pd, attributes);

                    // Clear named type context
                    pd.namedTypeContext.clear();

                    if (!out.node.name.empty()) {

                        if (SectionProcessor<DataStructureGroup>::isNamedTypeDuplicate(pd.blueprint, out.node.name)) {

                            // WARN: duplicate named type
                            std::stringstream ss;
                            ss << "named type with name '" << out.node.name << "' already exists";

                            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                            out.report.warnings.push_back(Warning(ss.str(),
                                                                  DuplicateWarning,
                                                                  sourceMap));

                            // Remove the attributes data from the AST since we are ignoring this
                            out.node.attributes = mson::NamedType();

                            return cur;
                        }

                        attributes.node.name.symbol.literal = out.node.name;

                        if (pd.exportSourceMap()) {
                            attributes.sourceMap.name.sourceMap = out.sourceMap.name.sourceMap;
                        }
                    }

                    return cur;
                }

                default:
                    break;
            }

            return node;
        }

        static MarkdownNodeIterator processUnexpectedNode(const MarkdownNodeIterator& node,
                                                          const MarkdownNodes& siblings,
                                                          SectionParserData& pd,
                                                          SectionType& sectionType,
                                                          const ParseResultRef<Resource>& out) {

            if ((node->type == mdp::ParagraphMarkdownNodeType ||
                 node->type == mdp::CodeMarkdownNodeType) &&
                (sectionType == ModelBodySectionType ||
                 sectionType == ModelSectionType)) {

                mdp::ByteBuffer content = CodeBlockUtility::addDanglingAsset(node, pd, sectionType, out.report, out.node.model.body);

                if (pd.exportSourceMap() && !content.empty()) {
                    out.sourceMap.model.body.sourceMap.append(node->sourceMap);
                }

                // Update model in the model table as well
                ModelTable::iterator it = pd.modelTable.find(out.node.model.name);

                if (it != pd.modelTable.end()) {
                    it->second.body = out.node.model.body;

                    if (pd.exportSourceMap()) {
                        pd.modelSourceMapTable[out.node.model.name].body = out.sourceMap.model.body;
                    }
                }

                return ++MarkdownNodeIterator(node);
            }

            return SectionProcessorBase<Resource>::processUnexpectedNode(node, siblings, pd, sectionType, out);
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            if (SectionProcessor<Action>::actionType(node) == CompleteActionType) {
                return false;
            }

            return SectionProcessorBase<Resource>::isDescriptionNode(node, sectionType);
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::HeaderMarkdownNodeType
                && !node->text.empty()) {

                CaptureGroups captureGroups;
                mdp::ByteBuffer subject = node->text;

                TrimString(subject);

                if (RegexMatch(subject, NamedResourceHeaderRegex) ||
                    RegexMatch(subject, NamedEndpointHeaderRegex) ||
                    RegexMatch(subject, ResourceHeaderRegex)) {
                    return ResourceSectionType;
                }
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            SectionType nestedType = UndefinedSectionType;

            // Check if parameters section
            nestedType = SectionProcessor<Parameters>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            // Check if headers section
            nestedType = SectionProcessor<Headers>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            // Check if model section
            nestedType = SectionProcessor<Payload>::sectionType(node);

            if (nestedType == ModelSectionType ||
                nestedType == ModelBodySectionType) {

                return nestedType;
            }

            // Check if attributes section
            nestedType = SectionProcessor<Attributes>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            // Check if action section
            nestedType = SectionProcessor<Action>::sectionType(node);

            if (nestedType == ActionSectionType) {

                // Do not consider complete actions as nested
                mdp::ByteBuffer method;
                if (SectionProcessor<Action>::actionType(node) == CompleteActionType)
                    return UndefinedSectionType;

                return nestedType;
            }

            return UndefinedSectionType;
        }

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested;

            // Action & descendants
            nested.push_back(ActionSectionType);
            SectionTypes types = SectionProcessor<Action>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            nested.push_back(ModelSectionType);
            nested.push_back(ModelBodySectionType);

            return nested;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<Resource>& out) {

            if (!out.node.uriTemplate.empty()) {

                URITemplateParser uriTemplateParser;
                ParsedURITemplate parsedResult;
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);

                uriTemplateParser.parse(out.node.uriTemplate, sourceMap, parsedResult);

                if (!parsedResult.report.warnings.empty()) {
                    out.report += parsedResult.report;
                }
            }

            // Consolidate deprecated headers into subsequent payloads
            if (!out.node.headers.empty()) {

                Collection<Action>::iterator actionIt = out.node.actions.begin();
                Collection<SourceMap<Action> >::iterator actionSMIt = out.sourceMap.actions.collection.begin();

                for (;
                     actionIt != out.node.actions.end();
                     ++actionIt, ++actionSMIt) {

                    SectionProcessor<Headers>::injectDeprecatedHeaders(pd, out.node.headers, out.sourceMap.headers, actionIt->examples, actionSMIt->examples);
                }

                out.node.headers.clear();

                if (pd.exportSourceMap()) {
                    out.sourceMap.headers.collection.clear();
                }
            }
        }

        /**
         * \brief Given a named resource header, retrieve the name and uriTemplate
         *
         * \param node Markdown node to process
         * \param resource Resource data structure
         */
        static void matchNamedResourceHeader(const MarkdownNodeIterator& node,
                                             Resource& resource) {

            CaptureGroups captureGroups;

            if (RegexCapture(node->text, NamedResourceHeaderRegex, captureGroups, 4)) {

                resource.name = captureGroups[1];
                TrimString(resource.name);
                resource.uriTemplate = captureGroups[2];
            }
        }

        /**
         * \brief Parse the current node as an action
         */
        static MarkdownNodeIterator processNestedAction(const MarkdownNodeIterator& node,
                                                        const MarkdownNodes& siblings,
                                                        SectionParserData& pd,
                                                        SectionLayout& layout,
                                                        const ParseResultRef<Resource>& out) {

            IntermediateParseResult<Action> action(out.report);
            MarkdownNodeIterator cur = ActionParser::parse(node, siblings, pd, action);

            out.node.actions.push_back(action.node);
            layout = RedirectSectionLayout;

            if (pd.exportSourceMap()) {
                out.sourceMap.actions.collection.push_back(action.sourceMap);
                out.sourceMap.uriTemplate.sourceMap = node->sourceMap;
            }

            return cur;
        }

        /** Process Action section */
        static MarkdownNodeIterator processAction(const MarkdownNodeIterator& node,
                                                  const MarkdownNodes& siblings,
                                                  SectionParserData& pd,
                                                  const ParseResultRef<Resource>& out) {

            IntermediateParseResult<Action> action(out.report);
            MarkdownNodeIterator cur = ActionParser::parse(node, siblings, pd, action);

            ActionIterator duplicate = SectionProcessor<Action>::findAction(out.node.actions, action.node);

            if (duplicate != out.node.actions.end()) {

                // WARN: duplicate method
                std::stringstream ss;
                ss << "action with method '" << action.node.method << "' already defined for resource '";
                ss << out.node.uriTemplate << "'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      DuplicateWarning,
                                                      sourceMap));
            }

            ActionIterator relationDuplicate = SectionProcessor<Action>::findRelation(out.node.actions, action.node.relation);

            if (relationDuplicate != out.node.actions.end()) {

                // WARN: duplicate relation identifier
                std::stringstream ss;
                ss << "relation identifier '" << action.node.relation.str << "' already defined for resource '" << out.node.uriTemplate << "'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      DuplicateWarning,
                                                      sourceMap));
            }

            if (!action.node.parameters.empty()) {

                if (!action.node.uriTemplate.empty()) {
                    checkParametersEligibility<Action>(node, pd, action.node.parameters, action);
                }
                else {
                    checkParametersEligibility<Resource>(node, pd, action.node.parameters, out);
                }
            }

            out.node.actions.push_back(action.node);

            if (pd.exportSourceMap()) {
                out.sourceMap.actions.collection.push_back(action.sourceMap);
            }

            return cur;
        }

        /** Process Parameters section */
        static MarkdownNodeIterator processParameters(const MarkdownNodeIterator& node,
                                                      const MarkdownNodes& siblings,
                                                      SectionParserData& pd,
                                                      const ParseResultRef<Resource>& out) {

            IntermediateParseResult<Parameters> parameters(out.report);

            MarkdownNodeIterator cur = ParametersParser::parse(node, siblings, pd, parameters);

            if (!parameters.node.empty()) {

                checkParametersEligibility<Resource>(node, pd, parameters.node, out);
                out.node.parameters.insert(out.node.parameters.end(), parameters.node.begin(), parameters.node.end());

                if (pd.exportSourceMap()) {
                    out.sourceMap.parameters.collection.insert(out.sourceMap.parameters.collection.end(),
                                                               parameters.sourceMap.collection.begin(),
                                                               parameters.sourceMap.collection.end());
                }
            }

            return cur;
        }

        /** Process Model section */
        static MarkdownNodeIterator processModel(const MarkdownNodeIterator& node,
                                                 const MarkdownNodes& siblings,
                                                 SectionParserData& pd,
                                                 const ParseResultRef<Resource>& out) {

            IntermediateParseResult<Payload> model(out.report);

            MarkdownNodeIterator cur = PayloadParser::parse(node, siblings, pd, model);

            // Check whether there isn't a model already
            if (!out.node.model.name.empty()) {

                // WARN: Model already defined
                std::stringstream ss;
                ss << "overshadowing previous model definition for '";

                if (!out.node.name.empty()) {
                    ss << out.node.name << "(" << out.node.uriTemplate << ")";
                } else {
                    ss << out.node.uriTemplate;
                }

                ss << "' resource, a resource can be represented by a single model only";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      DuplicateWarning,
                                                      sourceMap));
            }

            if (model.node.name.empty()) {

                if (!out.node.name.empty()) {
                    model.node.name = out.node.name;

                    if (pd.exportSourceMap()) {
                        model.sourceMap.name = out.sourceMap.name;
                    }
                } else {

                    // ERR: No name specified for resource model
                    std::stringstream ss;
                    ss << "resource model can be specified only for a named resource";
                    ss << ", name your resource, e.g. '# <resource name> [" << out.node.uriTemplate << "]'";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                    out.report.error = Error(ss.str(),
                                             ModelError,
                                             sourceMap);
                }
            }

            ModelTable::iterator it = pd.modelTable.find(model.node.name);

            if (it == pd.modelTable.end()) {

                pd.modelTable[model.node.name] = model.node;

                if (pd.exportSourceMap()) {
                    pd.modelSourceMapTable[model.node.name] = model.sourceMap;
                }
            } else {

                // ERR: Model already defined
                std::stringstream ss;
                ss << "symbol '" << model.node.name << "' already defined";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.error = Error(ss.str(),
                                         ModelError,
                                         sourceMap);
            }

            out.node.model = model.node;

            if (pd.exportSourceMap()) {
                out.sourceMap.model = model.sourceMap;
            }

            return cur;
        }

        /**
         * \brief Check Parameters eligibility in URI template
         *
         * \warning Do not specialise this.
         */
        template<typename T>
        static void checkParametersEligibility(const MarkdownNodeIterator& node,
                                               const SectionParserData& pd,
                                               Parameters& parameters,
                                               const ParseResultRef<T>& out) {

            for (ParameterIterator it = parameters.begin();
                 it != parameters.end();
                 ++it) {

                // Naive check whether parameter is present in URI Template
                if (out.node.uriTemplate.find(it->name) == std::string::npos) {

                    // WARN: parameter name not present
                    std::stringstream ss;
                    ss << "parameter '" << it->name << "' not specified in ";

                    if (!out.node.name.empty()) {
                        ss << "'" << out.node.name << "' ";
                    }

                    ss << "its '" << out.node.uriTemplate << "' URI template";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          LogicalErrorWarning,
                                                          sourceMap));
                }
            }
        }

        /**
         * \brief Given a list of elements, Check if a resource already exists with the given uri template
         *
         * \param elements Collection of elements
         * \param uri The resource uri template to be checked
         */
        static bool isResourceDuplicate(const Elements& elements,
                                        const URITemplate& uri) {

            for (Elements::const_iterator it = elements.begin(); it != elements.end(); ++it) {

                if (it->element == Element::ResourceElement &&
                    it->content.resource.uriTemplate == uri) {

                    return true;
                }
            }

            return false;
        }
    };

    /** Resource Section Parser */
    typedef SectionParser<Resource, HeaderSectionAdapter> ResourceParser;
}

#endif
