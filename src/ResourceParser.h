//
//  ResourceParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_RESOURCEPARSER_H
#define SNOWCRASH_RESOURCEPARSER_H

#include <sstream>
#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "ActionParser.h"
#include "RegexMatch.h"
#include "StringUtility.h"
#include "ParametersParser.h"

static const std::string ResourceHeaderRegex("^[ \\t]*((" HTTP_METHODS ")[ \\t]+)?(" URI_TEMPLATE ")$");
static const std::string NamedResourceHeaderRegex("^[ \\t]*(" SYMBOL_IDENTIFIER ")[ \\t]+\\[(" URI_TEMPLATE ")]$");

namespace snowcrash {
    
    // Resource signature
    enum ResourceSignature {
        UndefinedResourceSignature,
        NoResourceSignature,
        URIResourceSignature,
        MethodURIResourceSignature,
        NamedResourceSignature
    };
    
    // Query resource signature
    FORCEINLINE ResourceSignature GetResourceSignature(const MarkdownBlock& block,
                                                       Name& name,
                                                       URITemplate& uri,
                                                       HTTPMethod& method) {
        if (block.type != HeaderBlockType ||
            block.content.empty())
            return NoResourceSignature;
        
        CaptureGroups captureGroups;
        // Nameless resource
        if (RegexCapture(block.content, ResourceHeaderRegex, captureGroups, 4)) {
            method = captureGroups[2];
            uri = captureGroups[3];
            return (method.empty()) ? URIResourceSignature : MethodURIResourceSignature;
        }
        else if (RegexCapture(block.content, NamedResourceHeaderRegex, captureGroups, 4)) {
            method.clear();
            name = captureGroups[1];
            TrimString(name);
            uri = captureGroups[3];
            return NamedResourceSignature;
        }

        return NoResourceSignature;
    }
    
    // Returns true if block has resource header signature, false otherwise
    FORCEINLINE bool HasResourceSignature(const MarkdownBlock& block) {

        Name name;
        URITemplate uri;
        HTTPMethod method;
        return GetResourceSignature(block, name, uri, method) != NoResourceSignature;
    }

    // Resource iterator in its containment group
    typedef Collection<Resource>::const_iterator ResourceIterator;
    
    // Finds a resource in resource group by its URI template
    FORCEINLINE ResourceIterator FindResource(const ResourceGroup& group,
                                              const Resource& resource) {
        return std::find_if(group.resources.begin(),
                            group.resources.end(),
                            std::bind2nd(MatchResource(), resource));
    }

    // Resource iterator pair: its containment group and resource iterator itself
    typedef std::pair<Collection<ResourceGroup>::const_iterator, ResourceIterator> ResourceIteratorPair;
    
    // Finds a resource in blueprint by its URI template
    FORCEINLINE ResourceIteratorPair FindResource(const Blueprint& blueprint,
                                                  const Resource& resource) {
        
        for (Collection<ResourceGroup>::const_iterator it = blueprint.resourceGroups.begin();
             it != blueprint.resourceGroups.end();
             ++it) {
            
            Collection<Resource>::const_iterator match = FindResource(*it, resource);
            if (match != it->resources.end())
                return std::make_pair(it, match);
        }
        
        return std::make_pair(blueprint.resourceGroups.end(), Collection<Resource>::iterator());
    }
    
    
    //
    // Classifier of internal list items, Resource context
    //
    template <>
    FORCEINLINE Section ClassifyInternaListBlock<Resource>(const BlockIterator& begin,
                                                           const BlockIterator& end) {
        if (HasHeaderSignature(begin, end))
            return HeadersSection;
        
        if (HasParametersSignature(begin, end))
            return ParametersSection;
        
        Name name;
        SourceData mediaType;
        PayloadSignature payloadSignature = GetPayloadSignature(begin, end, name, mediaType);
        if (payloadSignature == ObjectPayloadSignature)
            return ObjectSection;
        else if (payloadSignature == ModelPayloadSignature)
            return ModelSection;

        return UndefinedSection;
    }
    
    //
    // Block Classifier, Resource Context
    //
    template <>
    FORCEINLINE Section ClassifyBlock<Resource>(const BlockIterator& begin,
                                                const BlockIterator& end,
                                                const Section& context) {
        if (HasResourceGroupSignature(*begin))
            return UndefinedSection;

        Name name;
        URITemplate uri;
        HTTPMethod method;
        ResourceSignature resourceSignature = GetResourceSignature(*begin, name, uri, method);
        if (resourceSignature != NoResourceSignature) {
            return (context == UndefinedSection) ?
                    ((resourceSignature == MethodURIResourceSignature) ? ResourceMethodSection : ResourceSection) :
                    UndefinedSection;
        }
        
        if (HasActionSignature(*begin))
            return (context != ResourceMethodSection) ? ActionSection : UndefinedSection;
        
        Section listSection = ClassifyInternaListBlock<Resource>(begin, end);
        if (listSection != UndefinedSection)
            return listSection;
        
        // Unrecognized list item at this level
        if (begin->type == ListItemBlockBeginType)
            return ForeignSection;
        
        return (context == ResourceSection) ? context : UndefinedSection;
    }
        
    //
    // Resource Section Parser
    //
    template<>
    struct SectionParser<Resource> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               BlueprintParserCore& parser,
                                               Resource& resource) {

            ParseSectionResult result = std::make_pair(Result(), cur);
            switch (section) {                    
                case ResourceSection:
                    result = HandleResourceOverviewBlock(cur, bounds, parser, resource);
                    break;
                    
                case ResourceMethodSection:
                    result = HandleResourceMethod(cur, bounds, parser, resource);
                    break;
                
                case ModelSection:
                case ObjectSection:
                    result = HandleModel(cur, bounds.second, parser, resource);
                    break;
                    
                case ParametersSection:
                    result = HandleParameters(cur, bounds, parser, resource);
                    break;
                    
                case HeadersSection:
                    result = HandleHeaders(cur, bounds.second, parser, resource);
                    break;
                    
                case ActionSection:
                    result = HandleAction(cur, bounds.second, parser, resource);
                    break;
                    
                case UndefinedSection:
                    CheckAmbiguousMethod(cur, bounds.second, resource, result.first);
                    result.second = CloseListItemBlock(cur, bounds.second);
                    break;
                    
                case ForeignSection:
                    result = HandleForeignSection(cur, bounds);
                    break;
                    
                default:
                    result.first.error = UnexpectedBlockError(*cur);
                    break;
            }
            
            return result;
        }
        
        static ParseSectionResult HandleResourceOverviewBlock(const BlockIterator& cur,
                                                              const SectionBounds& bounds,
                                                              BlueprintParserCore& parser,
                                                              Resource& resource) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur(cur);
            if (cur->type == HeaderBlockType &&
                cur == bounds.first) {
                
                // Retrieve URI
                HTTPMethod method;
                GetResourceSignature(*cur, resource.name, resource.uriTemplate, method);
            }
            else {
                
                if (cur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (cur->type == ListBlockBeginType) {
                    
                    SourceDataBlock descriptionMap;
                    sectionCur = SkipToDescriptionListEnd<Resource>(sectionCur, bounds.second, descriptionMap);
                    
                    if (sectionCur->type != ListBlockEndType) {
                        if (!descriptionMap.empty())
                            resource.description += MapSourceData(parser.sourceData, descriptionMap);
                        
                        result.second = sectionCur;
                        return result;
                    }
                }
                
                if (!CheckCursor(sectionCur, bounds, cur, result.first))
                    return result;
                resource.description += MapSourceData(parser.sourceData, sectionCur->sourceMap);
            }
            
            result.second = ++sectionCur;
            return result;
        }
        
        static ParseSectionResult HandleModel(const BlockIterator& begin,
                                              const BlockIterator& end,
                                              BlueprintParserCore& parser,
                                              Resource& resource)
        {
            Payload payload;
            ParseSectionResult result = PayloadParser::Parse(begin, end, parser, payload);
            if (result.first.error.code != Error::OK)
                return result;
            
            // Check whether there isnt a model already
            if (!resource.model.name.empty()) {

                // WARN: Model already defined
                std::stringstream ss;
                ss << "overshadowing previous model definiton for '";

                if (!resource.name.empty())
                    ss << resource.name << "(" << resource.uriTemplate << ")";
                else
                    ss << resource.uriTemplate;

                ss << "' resource, a resource can be represented by a single model only";
                
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                result.first.warnings.push_back(Warning(ss.str(),
                                                        DuplicateWarning,
                                                        nameBlock->sourceMap));
                
                return result;
            }

            // Check symbol name
            if (payload.name.empty()) {
                
                if (!resource.name.empty()) {
                    payload.name = resource.name;
                }
                else {
                    // ERR: No name specified for resource model
                    std::stringstream ss;
                    ss << "resource model can be specified only for a named resource";
                    ss << ", name your resource, e.g. '# <resource name> [" << resource.uriTemplate << "]'";
                    BlockIterator nameBlock = ListItemNameBlock(begin, end);
                    result.first.error = Error(ss.str(),
                                               SymbolError,
                                               nameBlock->sourceMap);
                }
            }
            
            // Check whether symbol isn't already defined
            ResourceModelSymbolTable::const_iterator it = parser.symbolTable.resourceModels.find(payload.name);
            if (it == parser.symbolTable.resourceModels.end()) {

                parser.symbolTable.resourceModels[payload.name] = payload;
            }
            else {
                // ERR: Symbol already defined
                std::stringstream ss;
                ss << "symbol '" << payload.name << "' already defined";
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                result.first.error = Error(ss.str(),
                                           SymbolError,
                                           nameBlock->sourceMap);
            }
            
            // Assign model
            resource.model = payload;
            
            return result;
        }
        
        /** Parse Parameters section */
        static ParseSectionResult HandleParameters(const BlockIterator& cur,
                                                   const SectionBounds& bounds,
                                                   BlueprintParserCore& parser,
                                                   Resource& resource) {
            ParameterCollection parameters;
            ParseSectionResult result = ParametersParser::Parse(cur, bounds.second, parser, parameters);
            if (result.first.error.code != Error::OK)
                return result;
            
            if (parameters.empty()) {
                BlockIterator nameBlock = ListItemNameBlock(cur, bounds.second);
                result.first.warnings.push_back(Warning(NoParametersMessage,
                                                        FormattingWarning,
                                                        nameBlock->sourceMap));
            }
            else {
                // Check Eligibility
                BlockIterator nameBlock = ListItemNameBlock(cur, bounds.second);
                CheckParametersEligibility(resource, parameters, nameBlock->sourceMap, result.first);
                
                // Insert
                resource.parameters.insert(resource.parameters.end(), parameters.begin(), parameters.end());
            }
            
            return result;
        }
        
        /** Check parameter eligibility - report if a parameter is not specified in URI template */
        static void CheckParametersEligibility(const Resource& resource,
                                               const ParameterCollection& parameters,
                                               const SourceDataBlock& location,
                                               Result& result)
        {
            for (ParameterCollection::const_iterator it = parameters.begin();
                 it != parameters.end();
                 ++it) {
                
                // Naive check whether parameter is present in URI Template
                if (resource.uriTemplate.find(it->name) == std::string::npos) {
                    // WARN: parameter name not present
                    std::stringstream ss;
                    ss << "parameter '" << it->name << "' not specified in ";
                    if (!resource.name.empty())
                        ss << "'" << resource.name << "' ";
                    ss << "its '" << resource.uriTemplate << "' URI template";

                    result.warnings.push_back(Warning(ss.str(), LogicalErrorWarning, location));
                }
            }
        }
        
        static ParseSectionResult HandleResourceMethod(const BlockIterator& cur,
                                                       const SectionBounds& bounds,
                                                       BlueprintParserCore& parser,
                                                       Resource& resource) {
            
            // Retrieve URI template
            HTTPMethod method;
            GetResourceSignature(*cur, resource.name, resource.uriTemplate, method);
            
            // Parse as a resource action abbreviation
            return HandleAction(cur, bounds.second, parser, resource, true);
        }
        
        static ParseSectionResult HandleAction(const BlockIterator& begin,
                                               const BlockIterator& end,
                                               BlueprintParserCore& parser,
                                               Resource& resource,
                                               bool abbrev = false)
        {
            Action action;
            ParseSectionResult result = ActionParser::Parse(begin, end, parser, action);
            if (result.first.error.code != Error::OK)
                return result;

            if (!abbrev) {
                Name name;
                HTTPMethod httpMethod;
                ActionSignature actionSignature = GetActionSignature(*begin, name, httpMethod);
                if (actionSignature == MethodURIActionSignature) {
                    // WARN: ignoring extraneous content in action header
                    std::stringstream ss;
                    ss << "ignoring additional content in method header '" << begin->content << "'";
                    ss << ", expected method-only e.g. '# " << action.method << "'";
                    result.first.warnings.push_back(Warning(ss.str(), IgnoringWarning, begin->sourceMap));
                }
            }
            
            Collection<Action>::iterator duplicate = FindAction(resource, action);
            if (duplicate != resource.actions.end()) {
                
                // WARN: duplicate method
                result.first.warnings.push_back(Warning("action with method '" +
                                                        action.method +
                                                        "' already defined for resource '" +
                                                        resource.uriTemplate +
                                                        "'",
                                                        DuplicateWarning,
                                                        begin->sourceMap));
            }
            
            // FIXME: Do we want to check heck for parameters duplicates at this level?
            
            // Check Eligibility
            if (!action.parameters.empty())
                CheckParametersEligibility(resource, action.parameters, begin->sourceMap, result.first);
            
            
            // Check for header duplictes
            DeepCheckHeaderDuplicates(resource, action, begin->sourceMap, result.first);
            
            if (action.examples.empty() ||
                action.examples.front().responses.empty()) {
                // WARN: method has no response
                result.first.warnings.push_back(Warning("no response defined for '" +
                                                        action.method +
                                                        " " +
                                                        resource.uriTemplate +
                                                        "'",
                                                        EmptyDefinitionWarning,
                                                        begin->sourceMap));
            }
            
            resource.actions.push_back(action);
            return result;
        }
        
        static void DeepCheckHeaderDuplicates(const Resource& resource,
                                              const Action& action,
                                              const SourceDataBlock& sourceMap,
                                              Result& result) {
            
            if (action.examples.empty())
                return;
            
            CheckHeaderDuplicates(resource, action, sourceMap, result);
            for (Collection<Request>::const_iterator it = action.examples.front().requests.begin();
                 it != action.examples.front().requests.end();
                 ++it) {
                
                CheckHeaderDuplicates(resource, *it, sourceMap, result);
            }
            for (Collection<Response>::const_iterator it = action.examples.front().responses.begin();
                 it != action.examples.front().responses.end();
                 ++it) {
                
                CheckHeaderDuplicates(resource, *it, sourceMap, result);
            }
        }
        
        // Check whether abbreviated resource action isn't followed by an
        // action header -> implies possible additional method intended
        static void CheckAmbiguousMethod(const BlockIterator& begin,
                                         const BlockIterator& end,
                                         const Resource& resource,
                                         Result& result) {
            
            if (begin == end ||
                begin->type != HeaderBlockType)
                return;
            
            Name name;
            HTTPMethod method;
            ActionSignature actionSignature = GetActionSignature(*begin, name, method);
            if (actionSignature == MethodActionSignature ||
                actionSignature == NamedActionSignature) {
                // WARN: ignoring possible method header
                std::stringstream ss;
                ss << "unexpected action '" << begin->content << "', ";
                ss << "to the define muliple actions for the '" << resource.uriTemplate << "' resource omit the HTTP method in its definition, ";
                ss << "e.g. '# " << resource.uriTemplate << "'";
                result.warnings.push_back(Warning(ss.str(), IgnoringWarning, begin->sourceMap));
            }
        }
    };
    
    typedef BlockParser<Resource, SectionParser<Resource> > ResourceParser;
}

#endif
