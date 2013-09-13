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

static const std::string ActionHeaderRegex("^(" HTTP_METHODS ")[ \\t]*(" URI_TEMPLATE ")?$");
static const std::string NamedActionHeaderRegex("^([^\\[]*)\\[(" HTTP_METHODS ")]$");

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
        if (RegexCapture(block.content, ActionHeaderRegex, captureGroups, 3)) {
            // Nameless action
            method = captureGroups[1];
            URITemplate uri = captureGroups[2];
            return (uri.empty()) ? MethodActionSignature : MethodURIActionSignature;
        }
        else if (RegexCapture(block.content, NamedActionHeaderRegex, captureGroups, 3)) {
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
    FORCEINLINE Section ClassifyInternaListBlock<Action>(const BlockIterator& begin,
                                                         const BlockIterator& end) {        
        if (HasHeaderSignature(begin, end))
            return HeadersSection;
        
        if (HasParametersSignature(begin, end))
            return ParametersSection;
        
        Name name;
        SourceData mediaType;
        PayloadSignature payload = GetPayloadSignature(begin, end, name, mediaType);
        if (payload == RequestPayloadSignature)
            return RequestSection;
        else if (payload == ResponsePayloadSignature)
            return ResponseSection;
        else if (payload == ObjectPayloadSignature)
            return ObjectSection;
        else if (payload == ModelPayloadSignature)
            return ModelSection;
        
        return UndefinedSection;
    }
        
    //
    // Block Classifier, Method Context
    //
    template <>
    FORCEINLINE Section ClassifyBlock<Action>(const BlockIterator& begin,
                                              const BlockIterator& end,
                                              const Section& context) {

        if (HasActionSignature(*begin))
            return (context == UndefinedSection) ? ActionSection : UndefinedSection;
        
        if (HasResourceSignature(*begin) ||
            HasResourceGroupSignature(*begin))
            return UndefinedSection;
        
        Section listSection = ClassifyInternaListBlock<Action>(begin, end);
        if (listSection != UndefinedSection)
            return listSection;
        
        // Unrecognized list item at this level
        if (begin->type == ListItemBlockBeginType)
            return ForeignSection;
        
        return (context == ActionSection) ? ActionSection : UndefinedSection;
    }
    
    //
    // Method Section Parser
    //
    template<>
    struct SectionParser<Action> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               BlueprintParserCore& parser,
                                               Action& action) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            
            switch (section) {                    
                case ActionSection:
                    result = HandleActionOverviewBlock(cur, bounds, parser, action);
                    break;

                case ParametersSection:
                    result = HandleParameters(cur, bounds, parser, action);
                    break;

                case HeadersSection:
                    result = HandleHeaders(cur, bounds.second, parser, action);
                    break;
                    
                case RequestSection:
                case ResponseSection:
                    result = HandlePayload(section, cur, bounds.second, parser, action);
                    break;
                    
                case ForeignSection:
                    result = HandleForeignSection(cur, bounds);
                    break;
                    
                case UndefinedSection:
                    result.second = CloseListItemBlock(cur, bounds.second);
                    break;
                
                case ModelSection:
                case ObjectSection:
                    // ERR: Unexpected model definition
                    result.first.error = Error("unexpected model definiton, a model can be only defined in the resource section",
                                               SymbolError,
                                               cur->sourceMap);
                    break;
                    
                default:
                    result.first.error = UnexpectedBlockError(*cur);
                    break;
            }
            
            return result;
        }
        
        static ParseSectionResult HandleActionOverviewBlock(const BlockIterator& cur,
                                                            const SectionBounds& bounds,
                                                            BlueprintParserCore& parser,
                                                            Action& action) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur(cur);
            if (cur->type == HeaderBlockType &&
                cur == bounds.first) {
                
                GetActionSignature(*cur, action.name, action.method);
            }
            else {
                
                if (sectionCur->type == QuoteBlockBeginType) {
                    sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
                }
                else if (sectionCur->type == ListBlockBeginType) {
                    
                    SourceDataBlock descriptionMap;
                    sectionCur = SkipToDescriptionListEnd<Action>(sectionCur, bounds.second, descriptionMap);
                    
                    if (sectionCur->type != ListBlockEndType) {
                        if (!descriptionMap.empty())
                            action.description += MapSourceData(parser.sourceData, descriptionMap);
                        
                        result.second = sectionCur;
                        return result;
                    }
                }
                
                if (!CheckCursor(sectionCur, bounds, cur, result.first))
                    return result;
                action.description += MapSourceData(parser.sourceData, sectionCur->sourceMap);
            }
            
            result.second = ++sectionCur;
            return result;
        }
        
        /** Parse Parameters section */
        static ParseSectionResult HandleParameters(const BlockIterator& cur,
                                                   const SectionBounds& bounds,
                                                   BlueprintParserCore& parser,
                                                   Action& action) {
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
                action.parameters.insert(action.parameters.end(), parameters.begin(), parameters.end());
            }
            
            return result;
        }
        
        /**
         *  \brief  Parse action payload
         *  \param  begin   The begin of the block to be parsed.
         *  \param  end     The end of the markdown block buffer.
         *  \param  parser  A parser's instance.
         *  \param  method  An output buffer to store parsed payload into.
         *  \return A block parser section result.
         */
        static ParseSectionResult HandlePayload(const Section &section,
                                                const BlockIterator& begin,
                                                const BlockIterator& end,
                                                BlueprintParserCore& parser,
                                                Action& action)
        {
            Payload payload;
            ParseSectionResult result = PayloadParser::Parse(begin, end, parser, payload);
            if (result.first.error.code != Error::OK)
                return result;
            
            // Make sure a transaction is defined for the Action
            if (action.examples.empty()) {
                action.examples.push_back(TransactionExample());
            }
            
            // Check for duplicate
            if (IsPayloadDuplicate(section, payload, action)) {
                // WARN: duplicate payload
                std::stringstream ss;
                ss << SectionName(section) << " payload `" << payload.name << "`";
                ss << " already defined for `" << action.method << "` method";
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                result.first.warnings.push_back(Warning(ss.str(),
                                                        DuplicateWarning,
                                                        nameBlock->sourceMap));
            }
            
            BlockIterator nameBlock = ListItemNameBlock(begin, end);

            // Check payload integrity
            CheckPayload(section, payload, nameBlock->sourceMap, result.first);
            
            // Inject parsed payload into the action
            if (section == RequestSection) {
                action.examples.front().requests.push_back(payload);
            }
            else if (section == ResponseSection) {
                action.examples.front().responses.push_back(payload);
            }
            
            // Check header duplicates
            CheckHeaderDuplicates(action, payload, nameBlock->sourceMap, result.first);
            
            return result;
        }
        
        
        /**
         *  \brief  Check & report payload validity.
         *  \param  section     A section of the payload.
         *  \param  sourceMap   Payload signature source map.
         *  \param  payload     The payload to be checked.
         */
        static void CheckPayload(const Section& section,
                                 const Payload& payload,
                                 const SourceDataBlock& sourceMap,
                                 Result& result) {
            
            bool warnEmptyBody = false;
            if (section == RequestSection) {
                
                if (payload.body.empty()) {
                    
                    // Warn when content type is specified or both headers and body are empty
                    if (payload.headers.empty()) {
                        warnEmptyBody = true;
                    }
                    else {
                        for (Collection<Header>::const_iterator it = payload.headers.begin();
                             it != payload.headers.end();
                             ++it) {

                            if (it->first == HTTPHeaderName::ContentType)
                                warnEmptyBody = true;
                        }
                    }
                }
            }
            else if (section == ResponseSection) {
                // Check status code
                HTTPStatusCode code = 0;
                if (!payload.name.empty()) {
                    std::stringstream(payload.name) >> code;
                }
                StatusCodeTraits traits = GetStatusCodeTrait(code);
                if (traits.allowBody) {
                    warnEmptyBody = payload.body.empty();
                }
                else if (!payload.body.empty()) {
                    // WARN: not empty body
                    std::stringstream ss;
                    ss << "the " << code << " response MUST NOT include a " << SectionName(BodySection);
                    result.warnings.push_back(Warning(ss.str(),
                                                      EmptyDefinitionWarning,
                                                      sourceMap));
                    return;
                }
            }
            
            // Issue the warning
            if (warnEmptyBody) {
                // WARN: empty body
                std::stringstream ss;
                ss << "empty " << SectionName(section) << " " << SectionName(BodySection);
                result.warnings.push_back(Warning(ss.str(),
                                                  EmptyDefinitionWarning,
                                                  sourceMap));
            }
        }

        /**
         *  Checks whether given section payload has duplicate.
         *  \return True when a duplicate is found, false otherwise.
         */
        static bool IsPayloadDuplicate(const Section& section, const Payload& payload, Action& action) {
            
            if (action.examples.empty())
                return false;
            
            if (section == RequestSection) {
                Collection<Request>::const_iterator duplicate = FindRequest(action.examples.front(), payload);
                return duplicate != action.examples.front().requests.end();
            }
            else if (section == ResponseSection) {
                Collection<Response>::const_iterator duplicate = FindResponse(action.examples.front(), payload);
                return duplicate != action.examples.front().responses.end();
            }

            return false;
        }
    };
    
    typedef BlockParser<Action, SectionParser<Action> > ActionParser;
}

#endif
