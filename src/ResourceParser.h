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
#include "HeadersParser.h"
#include "ParametersParser.h"
#include "UriTemplateParser.h"
#include "RegexMatch.h"

namespace snowcrash {
    
    /** Nameless resource matching regex */
    const char* const ResourceHeaderRegex = "^[[:blank:]]*(" HTTP_REQUEST_METHOD "[[:blank:]]+)?" URI_TEMPLATE "$";
    
    /** Named resource matching regex */
    const char* const NamedResourceHeaderRegex = "^[[:blank:]]*" SYMBOL_IDENTIFIER "[[:blank:]]+\\[" URI_TEMPLATE "]$";

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
                                                     ParseResult<Resource>& out) {

            CaptureGroups captureGroups;

            // If Abbreviated resource section
            if (RegexCapture(node->text, ResourceHeaderRegex, captureGroups, 4)) {

                out.node.uriTemplate = captureGroups[3];

                // Make this section an action
                if (!captureGroups[2].empty()) {

                    ParseResult<Action> action;
                    MarkdownNodeIterator cur = ActionParser::parse(node, node->parent().children(), pd, action);

                    out.report += action.report;
                    out.node.actions.push_back(action.node);
                    layout = RedirectSectionLayout;

                    if (pd.exportSourceMap()) {
                        out.sourceMap.actions.collection.push_back(action.sourceMap);
                        out.sourceMap.uriTemplate.sourceMap = node->sourceMap;
                    }

                    return cur;
                }
            } else if (RegexCapture(node->text, NamedResourceHeaderRegex, captureGroups, 4)) {

                out.node.name = captureGroups[1];
                TrimString(out.node.name);
                out.node.uriTemplate = captureGroups[2];
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
                                                         ParseResult<Resource>& out) {

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
                    ParseResult<Headers> headers;
                    headers.node = out.node.headers;
                    headers.sourceMap = out.sourceMap.headers;

                    MarkdownNodeIterator cur = SectionProcessor<Action>::handleDeprecatedHeaders(node, siblings, pd, headers);

                    out.report += headers.report;
                    out.node.headers = headers.node;
                    out.sourceMap.headers = headers.sourceMap;

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
                                                          ParseResult<Resource>& out) {
            
            if ((node->type == mdp::ParagraphMarkdownNodeType ||
                 node->type == mdp::CodeMarkdownNodeType) &&
                (sectionType == ModelBodySectionType ||
                 sectionType == ModelSectionType)) {
                
                mdp::ByteBuffer content = CodeBlockUtility::addDanglingAsset(node, pd, sectionType, out.report, out.node.model.body);

                if (pd.exportSourceMap() && !content.empty()) {
                    out.sourceMap.model.body.sourceMap.append(node->sourceMap);
                }

                // Update model in the symbol table as well
                ResourceModelSymbolTable::iterator it = pd.symbolTable.resourceModels.find(out.node.model.name);
                
                if (it != pd.symbolTable.resourceModels.end()) {
                    it->second.body = out.node.model.body;

                    if (pd.exportSourceMap()) {
                        pd.symbolSourceMapTable.resourceModels[out.node.model.name].body = out.sourceMap.model.body;
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

            if (nestedType == HeadersSectionType) {
                return nestedType;
            }

            // Check if model section
            nestedType = SectionProcessor<Payload>::sectionType(node);

            if (nestedType == ModelSectionType ||
                nestedType == ModelBodySectionType) {

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
                             ParseResult<Resource>& out) {

            if (!out.node.uriTemplate.empty()) {

                URITemplateParser uriTemplateParser;
                ParsedURITemplate parsedResult;
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);

                uriTemplateParser.parse(out.node.uriTemplate, sourceMap, parsedResult);

                if (!parsedResult.report.warnings.empty()) {
                    out.report += parsedResult.report;
                }
            }

            // Consolidate deprecated headers into subsequent payloads
            if (!out.node.headers.empty()) {

                Collection<Action>::iterator actIt = out.node.actions.begin();
                Collection<SourceMap<Action> >::iterator actSMIt = out.sourceMap.actions.collection.begin();

                for (;
                     actIt != out.node.actions.end();
                     ++actIt, ++actSMIt) {

                    SectionProcessor<Headers>::injectDeprecatedHeaders(pd, out.node.headers, out.sourceMap.headers, actIt->examples, actSMIt->examples);
                }

                out.node.headers.clear();

                if (pd.exportSourceMap()) {
                    out.sourceMap.headers.collection.clear();
                }
            }
        }

        /** Process Action section */
        static MarkdownNodeIterator processAction(const MarkdownNodeIterator& node,
                                                  const MarkdownNodes& siblings,
                                                  SectionParserData& pd,
                                                  ParseResult<Resource>& out) {

            ParseResult<Action> action;
            MarkdownNodeIterator cur = ActionParser::parse(node, siblings, pd, action);

            out.report += action.report;
            ActionIterator duplicate = SectionProcessor<Action>::findAction(out.node.actions, action.node);

            if (duplicate != out.node.actions.end()) {

                // WARN: duplicate method
                std::stringstream ss;
                ss << "action with method '" << action.node.method << "' already defined for resource '";
                ss << out.node.uriTemplate << "'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      DuplicateWarning,
                                                      sourceMap));
            }

            if (!action.node.parameters.empty()) {

                checkParametersEligibility(node, pd, action.node.parameters, out);
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
                                                      ParseResult<Resource>& out) {

            ParseResult<Parameters> parameters;
            MarkdownNodeIterator cur = ParametersParser::parse(node, siblings, pd, parameters);

            out.report += parameters.report;

            if (!parameters.node.empty()) {

                checkParametersEligibility(node, pd, parameters.node, out);
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
                                                 ParseResult<Resource>& out) {

            ParseResult<Payload> model;
            MarkdownNodeIterator cur = PayloadParser::parse(node, siblings, pd, model);

            out.report += model.report;

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

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
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

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.error = Error(ss.str(),
                                             SymbolError,
                                             sourceMap);
                }
            }

            ResourceModelSymbolTable::iterator it = pd.symbolTable.resourceModels.find(model.node.name);

            if (it == pd.symbolTable.resourceModels.end()) {

                pd.symbolTable.resourceModels[model.node.name] = model.node;

                if (pd.exportSourceMap()) {
                    pd.symbolSourceMapTable.resourceModels[model.node.name] = model.sourceMap;
                }
            } else {

                // ERR: Symbol already defined
                std::stringstream ss;
                ss << "symbol '" << model.node.name << "' already defined";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.error = Error(ss.str(),
                                         SymbolError,
                                         sourceMap);
            }

            out.node.model = model.node;

            if (pd.exportSourceMap()) {
                out.sourceMap.model = model.sourceMap;
            }

            return cur;
        }

        /** Check Parameters eligibility in URI template */
        static void checkParametersEligibility(const MarkdownNodeIterator& node,
                                               const SectionParserData& pd,
                                               Parameters& parameters,
                                               ParseResult<Resource>& out) {

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

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          LogicalErrorWarning,
                                                          sourceMap));
                }
            }
        }

        /** Finds a resource inside an resources collection */
        static ResourceIterator findResource(const Resources& resources,
                                             const Resource& resource) {

            return std::find_if(resources.begin(),
                                resources.end(),
                                std::bind2nd(MatchResource(), resource));
        }
    };

    /** Resource Section Parser */
    typedef SectionParser<Resource, HeaderSectionAdapter> ResourceParser;
}

#endif
