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
<<<<<<< HEAD
    struct SectionParser<Resource> {
        
        static ParseSectionResult ParseSection(const BlueprintSection& section,
                                               const BlockIterator& cur,
                                               BlueprintParserCore& parser,
                                               Resource& resource) {
            ParseSectionResult result = std::make_pair(Result(), cur);
           
            switch (section.type) {
                case ResourceSectionType:
                    result = HandleResourceDescriptionBlock(section, cur, parser, resource);
                    break;
                    
                case ResourceMethodSectionType:
                    result = HandleResourceMethod(section, cur, parser, resource);
                    break;
                
                case ModelSectionType:
                case ObjectSectionType:
                    result = HandleModel(section, cur, parser, resource);
                    break;
                    
                case ParametersSectionType:
                    result = HandleParameters(section, cur, parser, resource);
                    break;
                    
                case HeadersSectionType:
                    result = HandleDeprecatedHeaders(section, cur, parser, resource);
                    break;
                    
                case ActionSectionType:
                    result = HandleAction(section, cur, parser, resource);
                    break;
                    
                case UndefinedSectionType:
                    CheckAmbiguousMethod(section, cur, resource, parser.sourceData, result.first);
                    result.second = CloseList(cur, section.bounds.second);
                    break;
                    
                case ForeignSectionType:
                    result = HandleForeignSection<Resource>(section, cur, parser.sourceData);
                    break;
                    
                default:
                    result.first.error = UnexpectedBlockError(section, cur, parser.sourceData);
                    break;
            }

            
            return result;
        }
        
        static void Finalize(const SectionBounds& bounds,
                             BlueprintParserCore& parser,
                             Resource& resource,
                             Result& result)
        {

            if (!resource.uriTemplate.empty()) {
                URITemplateParser uriTemplateParser;
                ParsedURITemplate parsedResult;
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(bounds.first, bounds.first, bounds, parser.sourceData);

                uriTemplateParser.parse(resource.uriTemplate, sourceBlock, parsedResult);
                if (parsedResult.result.warnings.size() > 0) {
                    result += parsedResult.result;
                }
            }

            // Consolidate depraceted headers into subsequent payloads
            if (!resource.headers.empty()) {
                for (Collection<Action>::iterator it = resource.actions.begin();
                     it != resource.actions.end();
                     ++it) {
                  InjectDeprecatedHeaders(resource.headers, it->examples);
                }
                
                resource.headers.clear();
            }
=======
    struct SectionProcessor<Resource> : public SectionProcessorBase<Resource> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     Report& report,
                                                     Resource& out) {
>>>>>>> cb318f2... Started on ResourceParser

            CaptureGroups captureGroups;

            if (RegexCapture(node->text, ResourceHeaderRegex, captureGroups, 4)) {

                out.uriTemplate = captureGroups[3];

                if (!captureGroups[2].empty()) {

                    Action action;

                    action.method = captureGroups[2];
                    ActionParser::parse(node, node->parent().children(), pd, report, action);
                    out.actions.push_back(action);

                    return node;
                }
            } else if (RegexCapture(node->text, NamedResourceHeaderRegex, captureGroups, 4)) {

                out.name = captureGroups[1];
                TrimString(out.name);
                out.uriTemplate = captureGroups[2];
            }

            return ++MarkdownNodeIterator(node);
        }
<<<<<<< HEAD
        
        static ParseSectionResult HandleResourceDescriptionBlock(const BlueprintSection& section,
                                                                 const BlockIterator& cur,
                                                                 BlueprintParserCore& parser,
                                                                 Resource& resource) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur(cur);            
            
            // Retrieve URI            
            if (cur->type == HeaderBlockType &&
                cur == section.bounds.first) {
                
                HTTPMethod method;
                GetResourceSignature(*cur, resource.name, resource.uriTemplate, method);
                result.second = ++sectionCur;
                return result;
            }
=======
>>>>>>> cb318f2... Started on ResourceParser

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         Resource& out) {

            switch (pd.sectionContext()) {
                case ActionSectionType:
                    return processAction(node, siblings, pd, report, out);

                case ParametersSectionType:
                    return ParametersParser::parse(node, siblings, pd, report, out.parameters);

                case ModelSectionType:
                case ModelBodySectionType:
                    return processModel(node, siblings, pd, report, out);

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

        static MarkdownNodeIterator processAction(const MarkdownNodeIterator& node,
                                                  const MarkdownNodes& siblings,
                                                  SectionParserData& pd,
                                                  Report& report,
                                                  Resource& out) {

            Action action;
            MarkdownNodeIterator cur = ActionParser::parse(node, siblings, pd, report, action);

            out.actions.push_back(action);

            return cur;
        }

        static MarkdownNodeIterator processModel(const MarkdownNodeIterator& node,
                                                  const MarkdownNodes& siblings,
                                                  SectionParserData& pd,
                                                  Report& report,
                                                  Resource& out) {

            Payload model;
            MarkdownNodeIterator cur = PayloadParser::parse(node, siblings, pd, report, model);

            out.model = model;

            // TODO: Feel like I have to do something here

            return cur;
        }
    };

    /** Resource Section Parser */
    typedef SectionParser<Resource, HeaderSectionAdapter> ResourceParser;
}

#endif
