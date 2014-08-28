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

    /** Internal type alias for Collection of Resource */
    typedef Collection<Resource>::type Resources;

    typedef Collection<Resource>::const_iterator ResourceIterator;

    /**
     * Resource Section processor
     */
    template<>
    struct SectionProcessor<Resource, ResourceSM> : public SectionProcessorBase<Resource, ResourceSM> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     Report& report,
                                                     Resource& out,
                                                     ResourceSM& outSM) {

            CaptureGroups captureGroups;

            // If Abbreviated resource section
            if (RegexCapture(node->text, ResourceHeaderRegex, captureGroups, 4)) {

                out.uriTemplate = captureGroups[3];

                // Make this section an action
                if (!captureGroups[2].empty()) {

                    Action action;
                    ActionSM actionSM;

                    MarkdownNodeIterator cur = ActionParser::parse(node, node->parent().children(), pd, report, action, actionSM);

                    if (pd.exportSM()) {
                        outSM.actions.push_back(actionSM);
                    }

                    out.actions.push_back(action);
                    layout = RedirectSectionLayout;

                    return cur;
                }
            } else if (RegexCapture(node->text, NamedResourceHeaderRegex, captureGroups, 4)) {

                out.name = captureGroups[1];
                TrimString(out.name);
                out.uriTemplate = captureGroups[2];
            }

            if (pd.exportSM()) {
                if (!out.uriTemplate.empty()) {
                    outSM.uriTemplate = node->sourceMap;
                }

                if (!out.name.empty()) {
                    outSM.name = node->sourceMap;
                }
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         Resource& out,
                                                         ResourceSM& outSM) {

            switch (pd.sectionContext()) {
                case ActionSectionType:
                    return processAction(node, siblings, pd, report, out, outSM);

                case ParametersSectionType:
                    return processParameters(node, siblings, pd, report, out, outSM);

                case ModelSectionType:
                case ModelBodySectionType:
                    return processModel(node, siblings, pd, report, out, outSM);

                case HeadersSectionType:
                    return SectionProcessor<Action, ActionSM>::handleDeprecatedHeaders(node, siblings, pd, report, out.headers, outSM.headers);

                default:
                    break;
            }

            return node;
        }
        
        static MarkdownNodeIterator processUnexpectedNode(const MarkdownNodeIterator& node,
                                                          const MarkdownNodes& siblings,
                                                          SectionParserData& pd,
                                                          SectionType& sectionType,
                                                          Report& report,
                                                          Resource& out,
                                                          ResourceSM& outSM) {
            
            if ((node->type == mdp::ParagraphMarkdownNodeType ||
                 node->type == mdp::CodeMarkdownNodeType) &&
                (sectionType == ModelBodySectionType ||
                 sectionType == ModelSectionType)) {
                
                mdp::ByteBuffer content = CodeBlockUtility::addDanglingAsset(node, pd, sectionType, report, out.model.body);

                // Update model in the symbol table as well
                ResourceModelSymbolTable::iterator it = pd.symbolTable.resourceModels.find(out.model.name);
                
                if (it != pd.symbolTable.resourceModels.end()) {
                    it->second.body = out.model.body;
                }
                
                return ++MarkdownNodeIterator(node);
            }
            
            return SectionProcessorBase<Resource, ResourceSM>::processUnexpectedNode(node, siblings, pd, sectionType, report, out, outSM);
        }
        
        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {
            
            if (SectionProcessor<Action, ActionSM>::actionType(node) == CompleteActionType) {
                return false;
            }
                
            return SectionProcessorBase<Resource, ResourceSM>::isDescriptionNode(node, sectionType);
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
            nestedType = SectionProcessor<Parameters, ParametersSM>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            // Check if headers section
            nestedType = SectionProcessor<Headers, HeadersSM>::sectionType(node);

            if (nestedType == HeadersSectionType) {
                return nestedType;
            }

            // Check if model section
            nestedType = SectionProcessor<Payload, PayloadSM>::sectionType(node);

            if (nestedType == ModelSectionType ||
                nestedType == ModelBodySectionType) {

                return nestedType;
            }

            // Check if action section
            nestedType = SectionProcessor<Action, ActionSM>::sectionType(node);

            if (nestedType == ActionSectionType) {
                
                // Do not consider complete actions as nested
                mdp::ByteBuffer method;
                if (SectionProcessor<Action, ActionSM>::actionType(node) == CompleteActionType)
                    return UndefinedSectionType;
                
                return nestedType;
            }

            return UndefinedSectionType;
        }

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested;

            // Action & descendants
            nested.push_back(ActionSectionType);
            SectionTypes types = SectionProcessor<Action, ActionSM>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            nested.push_back(ModelSectionType);
            nested.push_back(ModelBodySectionType);

            return nested;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             Report& report,
                             Resource& out,
                             ResourceSM& outSM) {

            if (!out.uriTemplate.empty()) {

                URITemplateParser uriTemplateParser;
                ParsedURITemplate parsedResult;
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);

                uriTemplateParser.parse(out.uriTemplate, sourceMap, parsedResult);

                if (!parsedResult.report.warnings.empty()) {
                    report += parsedResult.report;
                }
            }

            // Consolidate deprecated headers into subsequent payloads
            if (!out.headers.empty()) {

                Collection<Action>::iterator actIt = out.actions.begin();
                Collection<ActionSM>::iterator actSMIt = outSM.actions.begin();

                for (;
                     actIt != out.actions.end();
                     ++actIt, ++actSMIt) {

                    SectionProcessor<Headers, HeadersSM>::injectDeprecatedHeaders(pd, out.headers, outSM.headers, actIt->examples, actSMIt->examples);
                }

                out.headers.clear();

                if (pd.exportSM()) {
                    outSM.headers.clear();
                }
            }
        }

        /** Process Action section */
        static MarkdownNodeIterator processAction(const MarkdownNodeIterator& node,
                                                  const MarkdownNodes& siblings,
                                                  SectionParserData& pd,
                                                  Report& report,
                                                  Resource& out,
                                                  ResourceSM& outSM) {

            Action action;
            ActionSM actionSM;

            MarkdownNodeIterator cur = ActionParser::parse(node, siblings, pd, report, action, actionSM);
            ActionIterator duplicate = findAction(out.actions, action);

            if (duplicate != out.actions.end()) {

                // WARN: duplicate method
                std::stringstream ss;
                ss << "action with method '" << action.method << "' already defined for resource '";
                ss << out.uriTemplate << "'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning(ss.str(),
                                                  DuplicateWarning,
                                                  sourceMap));
            }

            if (!action.parameters.empty()) {

                checkParametersEligibility(node, pd, out, action.parameters, report);
            }

            out.actions.push_back(action);

            if (pd.exportSM()) {
                outSM.actions.push_back(actionSM);
            }

            return cur;
        }

        /** Process Parameters section */
        static MarkdownNodeIterator processParameters(const MarkdownNodeIterator& node,
                                                      const MarkdownNodes& siblings,
                                                      SectionParserData& pd,
                                                      Report& report,
                                                      Resource& out,
                                                      ResourceSM& outSM) {

            Parameters parameters;
            ParametersSM parametersSM;

            MarkdownNodeIterator cur = ParametersParser::parse(node, siblings, pd, report, parameters, parametersSM);

            if (!parameters.empty()) {

                checkParametersEligibility(node, pd, out, parameters, report);
                out.parameters.insert(out.parameters.end(), parameters.begin(), parameters.end());

                if (pd.exportSM()) {
                    outSM.parameters.insert(outSM.parameters.end(), parametersSM.begin(), parametersSM.end());
                }
            }

            return cur;
        }

        /** Process Model section */
        static MarkdownNodeIterator processModel(const MarkdownNodeIterator& node,
                                                 const MarkdownNodes& siblings,
                                                 SectionParserData& pd,
                                                 Report& report,
                                                 Resource& out,
                                                 ResourceSM& outSM) {

            Payload model;
            PayloadSM modelSM;

            MarkdownNodeIterator cur = PayloadParser::parse(node, siblings, pd, report, model, modelSM);

            // Check whether there isn't a model already
            if (!out.model.name.empty()) {

                // WARN: Model already defined
                std::stringstream ss;
                ss << "overshadowing previous model definition for '";

                if (!out.name.empty()) {
                    ss << out.name << "(" << out.uriTemplate << ")";
                } else {
                    ss << out.uriTemplate;
                }

                ss << "' resource, a resource can be represented by a single model only";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning(ss.str(),
                                                  DuplicateWarning,
                                                  sourceMap));
            }

            if (model.name.empty()) {

                if (!out.name.empty()) {
                    model.name = out.name;
                } else {

                    // ERR: No name specified for resource model
                    std::stringstream ss;
                    ss << "resource model can be specified only for a named resource";
                    ss << ", name your resource, e.g. '# <resource name> [" << out.uriTemplate << "]'";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.error = Error(ss.str(),
                                         SymbolError,
                                         sourceMap);
                }
            }

            ResourceModelSymbolTable::iterator it = pd.symbolTable.resourceModels.find(model.name);

            if (it == pd.symbolTable.resourceModels.end()) {

                pd.symbolTable.resourceModels[model.name] = model;
            } else {

                // ERR: Symbol already defined
                std::stringstream ss;
                ss << "symbol '" << model.name << "' already defined";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.error = Error(ss.str(),
                                     SymbolError,
                                     sourceMap);
            }

            out.model = model;

            return cur;
        }

        /** Check Parameters eligibility in URI template */
        static void checkParametersEligibility(const MarkdownNodeIterator& node,
                                               const SectionParserData& pd,
                                               Resource& resource,
                                               Parameters& parameters,
                                               Report& report) {

            for (ParameterIterator it = parameters.begin();
                 it != parameters.end();
                 ++it) {

                // Naive check whether parameter is present in URI Template
                if (resource.uriTemplate.find(it->name) == std::string::npos) {

                    // WARN: parameter name not present
                    std::stringstream ss;
                    ss << "parameter '" << it->name << "' not specified in ";

                    if (!resource.name.empty()) {
                        ss << "'" << resource.name << "' ";
                    }

                    ss << "its '" << resource.uriTemplate << "' URI template";

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(Warning(ss.str(),
                                                      LogicalErrorWarning,
                                                      sourceMap));
                }
            }
        }

        /** Finds an action inside an actions collection */
        static ActionIterator findAction(const Actions& actions,
                                         const Action& action) {

            return std::find_if(actions.begin(),
                                actions.end(),
                                std::bind2nd(MatchAction<Action>(), action));
        }
    };

    /** Resource Section Parser */
    typedef SectionParser<Resource, ResourceSM, HeaderSectionAdapter> ResourceParser;
}

#endif
