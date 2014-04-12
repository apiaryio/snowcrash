//
//  ActionParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_ACTIONPARSER_H
#define SNOWCRASH_ACTIONPARSER_H

#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "RegexMatch.h"
#include "PayloadParser.h"
#include "HeaderParser.h"
#include "ParametersParser.h"
#include "HTTP.h"
#include "DescriptionSectionUtility.h"

namespace snowcrashconst {
    
    /** Nameless action matching regex */
    const char* const ActionHeaderRegex = "^[[:blank:]]*" HTTP_REQUEST_METHOD "[[:blank:]]*" URI_TEMPLATE "?$";
    
    /** Named action matching regex */
    const char* const NamedActionHeaderRegex = "^[[:blank:]]*" SYMBOL_IDENTIFIER "\\[" HTTP_REQUEST_METHOD "]$";
}

namespace snowcrash {
    
    // Method signature
    enum ActionSignature {
        UndefinedActionSignature,
        NoActionSignature,
        MethodActionSignature,      // # GET
        MethodURIActionSignature,   // # GET /uri
        NamedActionSignature        // # My Method [GET]
    };
    
    // Query method signature
    FORCEINLINE ActionSignature GetActionSignature(const MarkdownBlock& block,
                                                   Name& name,
                                                   HTTPMethod& method) {
        if (block.type != HeaderBlockType ||
            block.content.empty())
            return NoActionSignature;
        
        CaptureGroups captureGroups;
        if (RegexCapture(block.content, snowcrashconst::ActionHeaderRegex, captureGroups, 3)) {
            // Nameless action
            method = captureGroups[1];
            URITemplate uri = captureGroups[2];
            return (uri.empty()) ? MethodActionSignature : MethodURIActionSignature;
        }
        else if (RegexCapture(block.content, snowcrashconst::NamedActionHeaderRegex, captureGroups, 3)) {
            // Named action
            name = captureGroups[1];
            TrimString(name);
            method = captureGroups[2];
            return NamedActionSignature;
        }
        
        return NoActionSignature;
    }

    // Returns true if block has HTTP Method signature, false otherwise
    FORCEINLINE bool HasActionSignature(const MarkdownBlock& block) {
        
        if (block.type != HeaderBlockType ||
            block.content.empty())
            return false;
        
        Name name;
        HTTPMethod method;
        return GetActionSignature(block, name, method) != NoActionSignature;
    }
    
    // Finds an action inside resource
    FORCEINLINE Collection<Action>::iterator FindAction(Resource& resource,
                                                        const Action& action) {
        return std::find_if(resource.actions.begin(),
                            resource.actions.end(),
                            std::bind2nd(MatchAction<Action>(), action));
    }
    
    //
    // Classifier of internal list items, Payload context
    //
    template <>
    FORCEINLINE SectionType ClassifyInternaListBlock<Action>(const BlockIterator& begin,
                                                             const BlockIterator& end) {
        if (HasHeaderSignature(begin, end))
            return HeadersSectionType;
        
        if (HasParametersSignature(begin, end))
            return ParametersSectionType;
        
        Name name;
        SourceData mediaType;
        PayloadSignature payload = GetPayloadSignature(begin, end, name, mediaType);
        if (payload == RequestPayloadSignature)
            return RequestSectionType;
        else if (payload == ResponsePayloadSignature)
            return ResponseSectionType;
        else if (payload == ObjectPayloadSignature)
            return ObjectSectionType;
        else if (payload == ModelPayloadSignature)
            return ModelSectionType;
        
        return UndefinedSectionType;
    }
    
    /** Children blocks classifier */
    template <>
    FORCEINLINE SectionType ClassifyChildrenListBlock<Action>(const BlockIterator& begin,
                                                              const BlockIterator& end) {
        
        SectionType type = ClassifyInternaListBlock<Action>(begin, end);
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
    // Block Classifier, Action Context
    //
    template <>
    FORCEINLINE SectionType ClassifyBlock<Action>(const BlockIterator& begin,
                                              const BlockIterator& end,
                                              const SectionType& context) {

        if (HasActionSignature(*begin))
            return (context == UndefinedSectionType) ? ActionSectionType : UndefinedSectionType;
        
        if (HasResourceSignature(*begin) ||
            HasResourceGroupSignature(*begin))
            return UndefinedSectionType;
        
        SectionType listSection = ClassifyInternaListBlock<Action>(begin, end);
        if (listSection != UndefinedSectionType)
            return listSection;
        
        // Unrecognized list item at this level
        if (begin->type == ListItemBlockBeginType)
            return ForeignSectionType;
        
        return (context == ActionSectionType) ? ActionSectionType : UndefinedSectionType;
    }
    
    //
    // Action SectionType Parser
    //
    template<>
    struct SectionParser<Action> {
        
        static ParseSectionResult ParseSection(const BlueprintSection& section,
                                               const BlockIterator& cur,
                                               BlueprintParserCore& parser,
                                               Action& action) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            
            switch (section.type) {
                case ActionSectionType:
                    result = HandleActionDescriptionBlock(section, cur, parser, action);
                    break;

                case ParametersSectionType:
                    result = HandleParameters(section, cur, parser, action);
                    break;

                case HeadersSectionType:
                    result = HandleDeprecatedHeaders(section, cur, parser, action);
                    break;
                    
                case RequestSectionType:
                case ResponseSectionType:
                    result = HandlePayload(section, cur, parser, action);
                    break;
                    
                case ForeignSectionType:
                    result = HandleForeignSection<Action>(section, cur, parser.sourceData);
                    break;
                    
                case UndefinedSectionType:
                    result.second = CloseList(cur, section.bounds.second);
                    break;
                
                case ModelSectionType:
                case ObjectSectionType:
                    {
                        // ERR: Unexpected model definition
                        SourceCharactersBlock sourceBlock = CharacterMapForBlock(cur,
                                                                                 section.bounds.second,
                                                                                 section.bounds,                                                                                 
                                                                                 parser.sourceData);
                        result.first.error = Error("unexpected model definiton, a model can be only defined in the resource section",
                                                   SymbolError,
                                                   sourceBlock);
                    }
                    break;
                    
                default:
                    result.first.error = UnexpectedBlockError(section, cur, parser.sourceData);
                    break;
            }
            
            return result;
        }
        
        static void Finalize(const SectionBounds& bounds,
                             BlueprintParserCore& parser,
                             Action& action,
                             Result& result)
        {
            // Consolidate deprecated headers into subsequent payloads
            if (!action.headers.empty()) {
                InjectDeprecatedHeaders(action.headers, action.examples);
                action.headers.clear();
            }
            
            // Check whether transaction example request is followed by a response
            if (action.examples.size() > 1 &&
                !action.examples.back().requests.empty() &&
                action.examples.back().responses.empty()) {
                // WARN: No response for request
                
                std::stringstream ss;
                ss << "action is missing a response for ";
                if (action.examples.back().requests.back().name.empty())
                    ss << "a request";
                else
                    ss << "the '" << action.examples.back().requests.back().name << "' request";
                
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(bounds.first, bounds.first, bounds, parser.sourceData);
                result.warnings.push_back(Warning(ss.str(),
                                                  EmptyDefinitionWarning,
                                                  sourceBlock));
            }
        }
                
        static ParseSectionResult HandleActionDescriptionBlock(const BlueprintSection& section,
                                                               const BlockIterator& cur,
                                                               BlueprintParserCore& parser,
                                                               Action& action) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur(cur);
            
            if (cur->type == HeaderBlockType &&
                cur == section.bounds.first) {
                
                GetActionSignature(*cur, action.name, action.method);
                result.second = ++sectionCur;
                return result;
            }
            
            result = ParseDescriptionBlock<Action>(section,
                                                    sectionCur,
                                                    parser.sourceData,
                                                    action);
            return result;
        }
        
        /** Parse Parameters section */
        static ParseSectionResult HandleParameters(const BlueprintSection& section,
                                                   const BlockIterator& cur,
                                                   BlueprintParserCore& parser,
                                                   Action& action) {
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
                action.parameters.insert(action.parameters.end(), parameters.begin(), parameters.end());
            }
            
            return result;
        }
        
        /**
         *  \brief  Parse action payload
         *  \param  section Actual section being parsed.
         *  \param  cur     Cursor within the section boundaries.
         *  \param  parser  A parser's instance.
         *  \param  action  An output buffer to store parsed payload into.
         *  \return A block parser section result.
         */
        static ParseSectionResult HandlePayload(const BlueprintSection& section,
                                                const BlockIterator& cur,
                                                BlueprintParserCore& parser,
                                                Action& action)
        {
            Payload payload;
            ParseSectionResult result = PayloadParser::Parse(cur,
                                                             section.bounds.second,
                                                             section,
                                                             parser,
                                                             payload);
            if (result.first.error.code != Error::OK)
                return result;
            
            // Create transaction example if needed
            if (action.examples.empty()) {
                action.examples.push_back(TransactionExample());
            }
            else if (section.type == RequestSectionType) {
                // Automatic request response pairing:
                // If a request follows a response create a new transaction example
                if (!action.examples.back().responses.empty()) {
                    action.examples.push_back(TransactionExample());
                }
            }
            
            BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
            
            // Check for duplicate
            if (IsPayloadDuplicate(section.type, payload, action.examples.back())) {
                // WARN: duplicate payload
                std::stringstream ss;
                ss << SectionName(section.type) << " payload `" << payload.name << "`";
                ss << " already defined for `" << action.method << "` method";

                SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, parser.sourceData);
                result.first.warnings.push_back(Warning(ss.str(),
                                                        DuplicateWarning,
                                                        sourceBlock));
            }

            // Check payload integrity
            CheckPayload(section.type, payload, action.method, nameBlock->sourceMap, parser.sourceData, result.first);
            
            // Inject parsed payload into the action
            if (section.type == RequestSectionType) {
                action.examples.back().requests.push_back(payload);
            }
            else if (section.type == ResponseSectionType) {
                action.examples.back().responses.push_back(payload);
            }
            
            // Check header duplicates
            CheckHeaderDuplicates(action, payload, nameBlock->sourceMap, parser.sourceData, result.first);
            
            return result;
        }
        
        
        /**
         *  \brief  Check & report payload validity.
         *  \param  section     A section of the payload.
         *  \param  sourceMap   Payload signature source map.
         *  \param  payload     The payload to be checked.
         */
        static void CheckPayload(const SectionType& section,
                                 const Payload& payload,
                                 const HTTPMethod method,
                                 const SourceDataBlock& sourceMap,
                                 const SourceData& sourceData,
                                 Result& result) {
            
            bool warnEmptyBody = false;

            std::string contentLength;
            std::string transferEncoding;

            for (Collection<Header>::const_iterator it = payload.headers.begin();
                 it != payload.headers.end();
                 ++it) {

                if (it->first == HTTPHeaderName::ContentLength) {
                    contentLength = it->second;
                }

                if (it->first == HTTPHeaderName::TransferEncoding) {
                    transferEncoding = it->second;
                }
            }
            
            if (section == RequestSectionType) {
                
                if (payload.body.empty()) {
                    
                    // Warn when content-length or transfer-encoding is specified or both headers and body are empty
                    if (payload.headers.empty()) {
                        warnEmptyBody = true;
                    }
                    else {
                        warnEmptyBody = !contentLength.empty() ||
                                        !transferEncoding.empty();
                    }

                    if (warnEmptyBody) {
                        // WARN: empty body
                        std::stringstream ss;
                        ss << "empty " << SectionName(section) << " " << SectionName(BodySectionType);

                        if (!contentLength.empty()) {
                            ss << ", expected " << SectionName(BodySectionType) << " for '" << contentLength << "' Content-Length";
                        }
                        else if (!transferEncoding.empty()) {
                            ss << ", expected " << SectionName(BodySectionType) << " for '" << transferEncoding << "' Transfer-Encoding";
                        }

                        result.warnings.push_back(Warning(ss.str(),
                                                          EmptyDefinitionWarning,
                                                          MapSourceDataBlock(sourceMap, sourceData)));
                    }

                }
            }
            else if (section == ResponseSectionType) {
                // Check status code
                HTTPStatusCode code = 0;

                if (!payload.name.empty()) {
                    std::stringstream(payload.name) >> code;
                }

                StatusCodeTraits statusCodeTraits = GetStatusCodeTrait(code);
                HTTPMethodTraits methodTraits = GetMethodTrait(method);

                if ((!statusCodeTraits.allowBody || !methodTraits.allowBody) && !payload.body.empty()) {
                    // WARN: not empty body

                    if (!statusCodeTraits.allowBody) {
                        std::stringstream ss;
                        ss << "the " << code << " response MUST NOT include a " << SectionName(BodySectionType);
                        result.warnings.push_back(Warning(ss.str(),
                                                          EmptyDefinitionWarning,
                                                          MapSourceDataBlock(sourceMap, sourceData)));
                    }

                    // WARN: Edge case for 2xx CONNECT
                    if (method == HTTPMethodName::Connect && code/100 == 2) {
                        std::stringstream ss;
                        ss << "the response for " << code << " " << method << " request MUST NOT include a " << SectionName(BodySectionType);
                        result.warnings.push_back(Warning(ss.str(),
                                                          EmptyDefinitionWarning,
                                                          MapSourceDataBlock(sourceMap, sourceData)));

                    }
                    else if (method != HTTPMethodName::Connect && !methodTraits.allowBody) {
                        std::stringstream ss;
                        ss << "the response for " << method << " request MUST NOT include a " << SectionName(BodySectionType);
                        result.warnings.push_back(Warning(ss.str(),
                                                          EmptyDefinitionWarning,
                                                          MapSourceDataBlock(sourceMap, sourceData)));
                    }

                    return;
                }
            }
        }

        /**
         *  Checks whether given section payload has duplicate within its transaction examples
         *  \return True when a duplicate is found, false otherwise.
         */
        static bool IsPayloadDuplicate(const SectionType& section, const Payload& payload, TransactionExample& example) {
            
            if (section == RequestSectionType) {
                Collection<Request>::const_iterator duplicate = FindRequest(example, payload);
                return duplicate != example.requests.end();
            }
            else if (section == ResponseSectionType) {
                Collection<Response>::const_iterator duplicate = FindResponse(example, payload);
                return duplicate != example.responses.end();
            }

            return false;
        }
    };
    
    typedef BlockParser<Action, SectionParser<Action> > ActionParser;
}

#endif
