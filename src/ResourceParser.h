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
#include "DescriptionSectionUtility.h"
#include "UriTemplateParser.h"

namespace snowcrashconst {
    
    /** Nameless resource matching regex */
    const char* const ResourceHeaderRegex = "^[[:blank:]]*(" HTTP_REQUEST_METHOD "[[:blank:]]+)?" URI_TEMPLATE "$";
    
    /** Named resource matching regex */
    const char* const NamedResourceHeaderRegex = "^[[:blank:]]*" SYMBOL_IDENTIFIER "[[:blank:]]+\\[" URI_TEMPLATE "]$";
}

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
        if (RegexCapture(block.content, snowcrashconst::ResourceHeaderRegex, captureGroups, 4)) {
            method = captureGroups[2];
            uri = captureGroups[3];
            return (method.empty()) ? URIResourceSignature : MethodURIResourceSignature;
        }
        else if (RegexCapture(block.content, snowcrashconst::NamedResourceHeaderRegex, captureGroups, 4)) {
            method.clear();
            name = captureGroups[1];
            TrimString(name);
            uri = captureGroups[2];
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
    FORCEINLINE SectionType ClassifyInternaListBlock<Resource>(const BlockIterator& begin,
                                                           const BlockIterator& end) {
        if (HasHeaderSignature(begin, end))
            return HeadersSectionType;
        
        if (HasParametersSignature(begin, end))
            return ParametersSectionType;
        
        Name name;
        SourceData mediaType;
        PayloadSignature payloadSignature = GetPayloadSignature(begin, end, name, mediaType);
        if (payloadSignature == ObjectPayloadSignature)
            return ObjectSectionType;
        else if (payloadSignature == ModelPayloadSignature)
            return ModelSectionType;

        return UndefinedSectionType;
    }
    
    /** Children blocks classifier */
    template <>
    FORCEINLINE SectionType ClassifyChildrenListBlock<Resource>(const BlockIterator& begin,
                                                              const BlockIterator& end) {
        
        SectionType type = ClassifyInternaListBlock<Resource>(begin, end);
        if (type != UndefinedSectionType)
            return type;
        
        type = ClassifyChildrenListBlock<HeaderCollection>(begin, end);
        if (type != UndefinedSectionType)
            return type;
        
        type = ClassifyChildrenListBlock<ParameterCollection>(begin, end);
        if (type != UndefinedSectionType)
            return type;
        
        type = ClassifyChildrenListBlock<Payload>(begin, end);
        if (type != UndefinedSectionType)
            return type;
        
        return UndefinedSectionType;
    }
    
    //
    // Block Classifier, Resource Context
    //
    template <>
    FORCEINLINE SectionType ClassifyBlock<Resource>(const BlockIterator& begin,
                                                const BlockIterator& end,
                                                const SectionType& context) {
        if (HasResourceGroupSignature(*begin))
            return UndefinedSectionType;

        Name name;
        URITemplate uri;
        HTTPMethod method;
        ResourceSignature resourceSignature = GetResourceSignature(*begin, name, uri, method);
        if (resourceSignature != NoResourceSignature) {
            return (context == UndefinedSectionType) ?
                    ((resourceSignature == MethodURIResourceSignature) ? ResourceMethodSectionType : ResourceSectionType) :
                    UndefinedSectionType;
        }
        
        if (HasActionSignature(*begin))
            return (context != ResourceMethodSectionType) ? ActionSectionType : UndefinedSectionType;
        
        SectionType listSection = ClassifyInternaListBlock<Resource>(begin, end);
        if (listSection != UndefinedSectionType)
            return listSection;
        
        // Unrecognized list item at this level
        if (begin->type == ListItemBlockBeginType)
            return ForeignSectionType;
        
        return (context == ResourceSectionType) ? context : UndefinedSectionType;
    }
        
    //
    // Resource SectionType Parser
    //
    template<>
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

        }
        
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

            result = ParseDescriptionBlock<Resource>(section,
                                                      sectionCur,
                                                      parser.sourceData,
                                                      resource);
            return result;
        }
        
        static ParseSectionResult HandleModel(const BlueprintSection& section,
                                              const BlockIterator& cur,
                                              BlueprintParserCore& parser,
                                              Resource& resource)
        {
            Payload payload;
            ParseSectionResult result = PayloadParser::Parse(cur,
                                                             section.bounds.second,
                                                             section,
                                                             parser,
                                                             payload);
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
                
                BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, parser.sourceData);
                result.first.warnings.push_back(Warning(ss.str(),
                                                        DuplicateWarning,
                                                        sourceBlock));
                
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

                    BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
                    SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, parser.sourceData);
                    result.first.error = Error(ss.str(),
                                               SymbolError,
                                               sourceBlock);
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

                BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, parser.sourceData);
                result.first.error = Error(ss.str(),
                                           SymbolError,
                                           sourceBlock);
            }
            
            // Assign model
            resource.model = payload;
            
            return result;
        }
        
        /** Parse Parameters section */
        static ParseSectionResult HandleParameters(const BlueprintSection& section,
                                                   const BlockIterator& cur,
                                                   BlueprintParserCore& parser,
                                                   Resource& resource) {
            ParameterCollection parameters;
            ParseSectionResult result = ParametersParser::Parse(cur,
                                                                section.bounds.second,
                                                                section,
                                                                parser,
                                                                parameters);
            if (result.first.error.code != Error::OK)
                return result;
            
            if (parameters.empty()) {
                BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, parser.sourceData);
                result.first.warnings.push_back(Warning(snowcrashconst::NoParametersMessage,
                                                        FormattingWarning,
                                                        sourceBlock));
            }
            else {
                // Check Eligibility
                BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
                CheckParametersEligibility(resource, parameters, nameBlock->sourceMap, parser.sourceData, result.first);
                
                // Insert
                resource.parameters.insert(resource.parameters.end(), parameters.begin(), parameters.end());
            }
            
            return result;
        }
        
        /** Check parameter eligibility, reporting if a parameter is not specified in URI template */
        static void CheckParametersEligibility(const Resource& resource,
                                               const ParameterCollection& parameters,
                                               const SourceDataBlock& location,
                                               const SourceData& sourceData,
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

                    result.warnings.push_back(Warning(ss.str(),
                                                      LogicalErrorWarning,
                                                      MapSourceDataBlock(location, sourceData)));
                }
            }
        }
        
        static ParseSectionResult HandleResourceMethod(const BlueprintSection& section,
                                                       const BlockIterator& cur,
                                                       BlueprintParserCore& parser,
                                                       Resource& resource) {
            
            // Retrieve URI template
            HTTPMethod method;
            GetResourceSignature(*cur, resource.name, resource.uriTemplate, method);
            
            // Parse as a resource action abbreviation
            return HandleAction(section, cur, parser, resource, true);
        }
        
        static ParseSectionResult HandleAction(const BlueprintSection& section,
                                               const BlockIterator& cur,
                                               BlueprintParserCore& parser,
                                               Resource& resource,
                                               bool abbrev = false)
        {
            Action action;
            ParseSectionResult result = ActionParser::Parse(cur,
                                                            section.bounds.second,
                                                            section,
                                                            parser,
                                                            action);
            if (result.first.error.code != Error::OK)
                return result;

            if (!abbrev) {
                Name name;
                HTTPMethod httpMethod;
                ActionSignature actionSignature = GetActionSignature(*cur, name, httpMethod);

                if (actionSignature == MethodURIActionSignature) {
                    // WARN: ignoring extraneous content in action header
                    std::stringstream ss;
                    ss << "ignoring additional content in method header '" << cur->content << "'";
                    ss << ", expected method-only e.g. '# " << action.method << "'";
                    
                    SourceCharactersBlock sourceBlock = CharacterMapForBlock(cur, section.bounds.second, section.bounds, parser.sourceData);
                    result.first.warnings.push_back(Warning(ss.str(),
                                                            IgnoringWarning,
                                                            sourceBlock));
                }
            }
            
            Collection<Action>::iterator duplicate = FindAction(resource, action);
            if (duplicate != resource.actions.end()) {
                
                // WARN: duplicate method
                std::stringstream ss;
                ss << "action with method '" << action.method << "' already defined for resource '";
                ss << resource.uriTemplate << "'";
                
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(cur, section.bounds.second, section.bounds, parser.sourceData);
                result.first.warnings.push_back(Warning(ss.str(),
                                                        DuplicateWarning,
                                                        sourceBlock));
            }
            
            // Check Eligibility
            if (!action.parameters.empty())
                CheckParametersEligibility(resource, action.parameters, cur->sourceMap, parser.sourceData, result.first);
            
            
            // Check for header duplictes
            DeepCheckHeaderDuplicates(resource, action, cur->sourceMap, parser.sourceData, result.first);
            
            if (action.examples.empty() ||
                action.examples.front().responses.empty()) {
                // WARN: method has no response
                std::stringstream ss;
                ss << "no response defined for '" << action.method << " " << resource.uriTemplate << "'";
                
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(cur, section.bounds.second, section.bounds, parser.sourceData);
                result.first.warnings.push_back(Warning(ss.str(),
                                                        EmptyDefinitionWarning,
                                                        sourceBlock));
            }
            
            resource.actions.push_back(action);
            return result;
        }
        
        static void DeepCheckHeaderDuplicates(const Resource& resource,
                                              const Action& action,
                                              const SourceDataBlock& sourceMap,
                                              const SourceData& sourceData,
                                              Result& result) {
            
            if (action.examples.empty())
                return;
            
            CheckHeaderDuplicates(resource, action, sourceMap, sourceData, result);
            for (Collection<Request>::const_iterator it = action.examples.front().requests.begin();
                 it != action.examples.front().requests.end();
                 ++it) {
                
                CheckHeaderDuplicates(resource, *it, sourceMap, sourceData, result);
            }
            for (Collection<Response>::const_iterator it = action.examples.front().responses.begin();
                 it != action.examples.front().responses.end();
                 ++it) {
                
                CheckHeaderDuplicates(resource, *it, sourceMap, sourceData, result);
            }
        }
        
        // Check whether abbreviated resource action isn't followed by an
        // action header which would imply possible additional method intended.
        static void CheckAmbiguousMethod(const BlueprintSection& section,
                                         const BlockIterator& cur,
                                         const Resource& resource,
                                         const SourceData& sourceData,
                                         Result& result) {
            
            if (cur == section.bounds.second ||
                cur->type != HeaderBlockType)
                return;
            
            Name name;
            HTTPMethod method;
            ActionSignature actionSignature = GetActionSignature(*cur, name, method);
            if (actionSignature == MethodActionSignature ||
                actionSignature == NamedActionSignature) {
                // WARN: ignoring possible method header
                std::stringstream ss;
                ss << "unexpected action '" << cur->content << "', ";
                ss << "to the define muliple actions for the '" << resource.uriTemplate << "' resource omit the HTTP method in its definition, ";
                ss << "e.g. '# " << resource.uriTemplate << "'";
                
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(cur, section.bounds.second, section.bounds, sourceData);
                result.warnings.push_back(Warning(ss.str(),
                                                  IgnoringWarning,
                                                  sourceBlock));
            }
        }
    };
    
    typedef BlockParser<Resource, SectionParser<Resource> > ResourceParser;
}

#endif
