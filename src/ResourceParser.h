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

    // Resource signature
    enum ResourceSignature {
        NoResourceSignature = 0,
        URIResourceSignature,
        MethodURIResourceSignature,
        NamedResourceSignature,
        UndefinedResourceSignature = -1
    };

    /**
     * Resource Section processor
     */
    template<>
    struct SectionProcessor<Resource> : public SectionProcessorBase<Resource> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     bool& parsingRedirect,
                                                     Report& report,
                                                     Resource& out) {

            CaptureGroups captureGroups;

            // If Abbreviated resource section
            if (RegexCapture(node->text, ResourceHeaderRegex, captureGroups, 4)) {

                out.uriTemplate = captureGroups[3];

                // Make this section an action
                if (!captureGroups[2].empty()) {

                    Action action;
                    MarkdownNodeIterator cur = ActionParser::parse(node, node->parent().children(), pd, report, action);

                    out.actions.push_back(action);
                    parsingRedirect = true;

                    return cur;
                }
            } else if (RegexCapture(node->text, NamedResourceHeaderRegex, captureGroups, 4)) {

                out.name = captureGroups[1];
                TrimString(out.name);
                out.uriTemplate = captureGroups[2];
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         Resource& out) {

            switch (pd.sectionContext()) {
                case ActionSectionType:
                    return processAction(node, siblings, pd, report, out);

                case ParametersSectionType:
                    return processParameters(node, siblings, pd, report, out);

                case ModelSectionType:
                case ModelBodySectionType:
                    return processModel(node, siblings, pd, report, out);

                case HeadersSectionType:
                    return SectionProcessor<Action>::handleDeprecatedHeaders(node, siblings, pd, report, out.headers);

                default:
                    break;
            }

            return node;
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::HeaderMarkdownNodeType
                && !node->text.empty()) {

                CaptureGroups captureGroups;
                mdp::ByteBuffer subject = node->text;

                TrimString(subject);

                if (RegexMatch(subject, NamedResourceHeaderRegex)) {
                    return ResourceSectionType;
                }

                if (RegexCapture(subject, ResourceHeaderRegex, captureGroups, 4)) {
                    return (captureGroups[2].empty()) ? ResourceSectionType : ResourceMethodSectionType;
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
                return nestedType;
            }

            return UndefinedSectionType;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             Report& report,
                             Resource& out) {

            if (!out.uriTemplate.empty()) {

                URITemplateParser uriTemplateParser;
                ParsedURITemplate parsedResult;
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);

                uriTemplateParser.parse(out.uriTemplate, sourceMap, parsedResult);

                if (!parsedResult.report.warnings.empty()) {
                    report += parsedResult.report;
                }
            }

            // Consolidate depraceted headers into subsequent payloads
            if (!out.headers.empty()) {

                for (Collection<Action>::iterator it = out.actions.begin();
                     it != out.actions.end();
                     ++it) {

                    SectionProcessor<Headers>::injectDeprecatedHeaders(out.headers, it->examples);
                }

                out.headers.clear();
            }
        }

        /** Process Action section */
        static MarkdownNodeIterator processAction(const MarkdownNodeIterator& node,
                                                  const MarkdownNodes& siblings,
                                                  SectionParserData& pd,
                                                  Report& report,
                                                  Resource& out) {

            Action action;
            MarkdownNodeIterator cur = ActionParser::parse(node, siblings, pd, report, action);

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

            return cur;
        }

        /** Process Parameters section */
        static MarkdownNodeIterator processParameters(const MarkdownNodeIterator& node,
                                                      const MarkdownNodes& siblings,
                                                      SectionParserData& pd,
                                                      Report& report,
                                                      Resource& out) {

            Parameters parameters;
            MarkdownNodeIterator cur = ParametersParser::parse(node, siblings, pd, report, parameters);

            if (!parameters.empty()) {

                checkParametersEligibility(node, pd, out, parameters, report);
                out.parameters.insert(out.parameters.end(), parameters.begin(), parameters.end());
            }

            return cur;
        }

        /** Process Model section */
        static MarkdownNodeIterator processModel(const MarkdownNodeIterator& node,
                                                 const MarkdownNodes& siblings,
                                                 SectionParserData& pd,
                                                 Report& report,
                                                 Resource& out) {

            Payload model;
            MarkdownNodeIterator cur = PayloadParser::parse(node, siblings, pd, report, model);

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
    typedef SectionParser<Resource, HeaderSectionAdapter> ResourceParser;
}

#endif
