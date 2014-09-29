//
//  PayloadParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/7/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_PARSEPAYLOAD_H
#define SNOWCRASH_PARSEPAYLOAD_H

#include "SectionParser.h"
#include "RegexMatch.h"
#include "AssetParser.h"
#include "HeadersParser.h"
#include "ParametersParser.h"

/** Media type in brackets regex */
#define MEDIA_TYPE "([[:blank:]]*\\(([^\\)]*)\\))"

namespace snowcrash {

    /** Internal type alias for Collection iterator of Response */
    typedef Collection<Response>::const_iterator ResponseIterator;
    typedef Collection<Request>::const_iterator RequestIterator;

    /// Payload signature
    enum PayloadSignature {
        NoPayloadSignature = 0,
        RequestPayloadSignature,    /// < Request payload.
        ResponsePayloadSignature,   /// < Response payload.
        ModelPayloadSignature,      /// < Resource Model payload.
        UndefinedPayloadSignature = -1
    };

    /** Request matching regex */
    const char* const RequestRegex = "^[[:blank:]]*[Rr]equest" SYMBOL_IDENTIFIER "?" MEDIA_TYPE "?[[:blank:]]*";

    /** Response matching regex */
    const char* const ResponseRegex = "^[[:blank:]]*[Rr]esponse([[:blank:][:digit:]]+)?" MEDIA_TYPE "?[[:blank:]]*";

    /** Model matching regex */
    const char* const  ModelRegex = "^[[:blank:]]*(" SYMBOL_IDENTIFIER "[[:blank:]]+)?[Mm]odel" MEDIA_TYPE "?[[:blank:]]*$";

    /**
     * Payload Section Processor
     */
    template<>
    struct SectionProcessor<Payload> : public SectionProcessorBase<Payload> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     ParseResult<Payload>& out) {

            mdp::ByteBuffer signature, remainingContent;
            signature = GetFirstLine(node->text, remainingContent);

            parseSignature(node, pd, signature, out);

            // WARN: missing status code
            if (out.node.name.empty() &&
                (pd.sectionContext() == ResponseSectionType || pd.sectionContext() == ResponseBodySectionType)) {

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.warnings.push_back(Warning("missing response HTTP status code, assuming 'Response 200'",
                                                      EmptyDefinitionWarning,
                                                      sourceMap));
                out.node.name = "200";
            }

            if (!remainingContent.empty()) {
                if (!isAbbreviated(pd.sectionContext())) {
                    out.node.description = remainingContent;

                    if (pd.exportSourceMap() && !out.node.description.empty()) {
                        out.sourceMap.description.sourceMap.append(node->sourceMap);
                    }
                } else if (!parseSymbolReference(node, pd, remainingContent, out)) {

                    // NOTE: NOT THE CORRECT WAY TO DO THIS
                    // https://github.com/apiaryio/snowcrash/commit/a7c5868e62df0048a85e2f9aeeb42c3b3e0a2f07#commitcomment-7322085
                    pd.sectionsContext.push_back(BodySectionType);
                    CodeBlockUtility::signatureContentAsCodeBlock(node, pd, out.report, out.node.body);
                    pd.sectionsContext.pop_back();

                    if (pd.exportSourceMap() && !out.node.body.empty()) {
                        out.sourceMap.body.sourceMap.append(node->sourceMap);
                    }
                }
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processContent(const MarkdownNodeIterator& node,
                                                   const MarkdownNodes& siblings,
                                                   SectionParserData& pd,
                                                   ParseResult<Payload>& out) {

            mdp::ByteBuffer content;

            if (!out.node.symbol.empty()) {
                //WARN: ignoring extraneous content after symbol reference
                std::stringstream ss;

                ss << "ignoring extraneous content after symbol reference";
                ss << ", expected symbol reference only e.g. '[" << out.node.symbol << "][]'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                out.report.warnings.push_back(Warning(ss.str(),
                                                      IgnoringWarning,
                                                      sourceMap));
            } else {

                if (!out.node.body.empty() ||
                    node->type != mdp::ParagraphMarkdownNodeType ||
                    !parseSymbolReference(node, pd, node->text, out)) {

                    // NOTE: NOT THE CORRECT WAY TO DO THIS
                    // https://github.com/apiaryio/snowcrash/commit/a7c5868e62df0048a85e2f9aeeb42c3b3e0a2f07#commitcomment-7322085
                    pd.sectionsContext.push_back(BodySectionType);
                    CodeBlockUtility::contentAsCodeBlock(node, pd, out.report, content);
                    pd.sectionsContext.pop_back();

                    out.node.body += content;

                    if (pd.exportSourceMap() && !content.empty()) {
                        out.sourceMap.body.sourceMap.append(node->sourceMap);
                    }
                }
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         ParseResult<Payload>& out) {

            switch (pd.sectionContext()) {
                case HeadersSectionType:
                {
                    ParseResult<Headers> headers;
                    headers.node = out.node.headers;
                    headers.sourceMap = out.sourceMap.headers;

                    MarkdownNodeIterator cur = HeadersParser::parse(node, siblings, pd, headers);

                    out.report += headers.report;
                    out.node.headers = headers.node;
                    out.sourceMap.headers = headers.sourceMap;

                    return cur;
                }

                case BodySectionType:
                {
                    if (!out.node.body.empty()) {
                        // WARN: Multiple body section
                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        out.report.warnings.push_back(Warning("ignoring additional 'body' content, it is already defined",
                                                              RedefinitionWarning,
                                                              sourceMap));
                    }

                    ParseResult<Asset> asset;
                    MarkdownNodeIterator cur = AssetParser::parse(node, siblings, pd, asset);

                    out.report += asset.report;
                    out.node.body = asset.node;
                    out.sourceMap.body = asset.sourceMap;

                    return cur;
                }

                case SchemaSectionType:
                {
                    if (!out.node.schema.empty()) {
                        // WARN: Multiple schema section
                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        out.report.warnings.push_back(Warning("ignoring additional 'schema' content, it is already defined",
                                                              RedefinitionWarning,
                                                              sourceMap));
                    }

                    ParseResult<Asset> asset;
                    MarkdownNodeIterator cur = AssetParser::parse(node, siblings, pd, asset);

                    out.report += asset.report;
                    out.node.schema = asset.node;
                    out.sourceMap.schema = asset.sourceMap;

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
                                                          ParseResult<Payload>& out) {

            if ((node->type == mdp::ParagraphMarkdownNodeType ||
                 node->type == mdp::CodeMarkdownNodeType) &&
                sectionType == BodySectionType) {

                mdp::ByteBuffer content = CodeBlockUtility::addDanglingAsset(node, pd, sectionType, out.report, out.node.body);

                if (pd.exportSourceMap() && !content.empty()) {
                    out.sourceMap.body.sourceMap.append(node->sourceMap);
                }

                return ++MarkdownNodeIterator(node);
            }
            
            return SectionProcessorBase<Payload>::processUnexpectedNode(node, siblings, pd, sectionType, out);
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            if (!isAbbreviated(sectionType) &&
                SectionProcessorBase<Payload>::isDescriptionNode(node, sectionType)) {

                return true;
            }

            return false;
        }

        static bool isContentNode(const MarkdownNodeIterator& node,
                                  SectionType sectionType) {

            if (isAbbreviated(sectionType) &&
                (SectionKeywordSignature(node) == UndefinedSectionType)) {

                return true;
            }

            return false;
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {

                SectionType nestedType = UndefinedSectionType;
                PayloadSignature signature = payloadSignature(node);

                if (signature == NoPayloadSignature) {
                    return UndefinedSectionType;
                }

                for (MarkdownNodeIterator child = node->children().begin();
                     child != node->children().end();
                     ++child) {

                    nestedType = nestedSectionType(child);

                    if (nestedType != UndefinedSectionType) {
                        return getSectionType(signature, nestedType);
                    }
                }

                // Return abbreviated signature
                return getSectionType(signature, nestedType);
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            SectionType nestedType = UndefinedSectionType;

            // Check if headers section
            nestedType = SectionProcessor<Headers>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            // Check if asset section
            nestedType = SectionProcessor<Asset>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            return UndefinedSectionType;
        }

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested, types;

            nested.push_back(HeadersSectionType);
            nested.push_back(BodySectionType);
            nested.push_back(SchemaSectionType);

            // Parameters & descendants
            nested.push_back(ParametersSectionType);
            types = SectionProcessor<Parameters>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            return nested;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             ParseResult<Payload>& out) {

            bool warnEmptyBody = false;

            mdp::ByteBuffer contentLength;
            mdp::ByteBuffer transferEncoding;

            SectionType sectionType = pd.sectionContext();

            for (HeaderIterator it = out.node.headers.begin();
                 it != out.node.headers.end();
                 ++it) {

                if (it->first == HTTPHeaderName::ContentLength) {
                    contentLength = it->second;
                }

                if (it->first == HTTPHeaderName::TransferEncoding) {
                    transferEncoding = it->second;
                }
            }

            if ((sectionType == RequestSectionType || sectionType == RequestBodySectionType) && out.node.body.empty()) {

                // Warn when content-length or transfer-encoding is specified or both headers and body are empty
                if (out.node.headers.empty()) {
                    warnEmptyBody = true;
                } else {
                    warnEmptyBody = !contentLength.empty() || !transferEncoding.empty();
                }

                if (warnEmptyBody) {
                    // WARN: empty body
                    std::stringstream ss;
                    ss << "empty " << SectionName(sectionType) << " " << SectionName(BodySectionType);

                    if (!contentLength.empty()) {
                        ss << ", expected " << SectionName(BodySectionType) << " for '" << contentLength << "' Content-Length";
                    } else if (!transferEncoding.empty()) {
                        ss << ", expected " << SectionName(BodySectionType) << " for '" << transferEncoding << "' Transfer-Encoding";
                    }

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          EmptyDefinitionWarning,
                                                          sourceMap));
                }
            } else if ((sectionType == ResponseSectionType || sectionType == ResponseBodySectionType)) {

                HTTPStatusCode code = 200;

                if (!out.node.name.empty()) {
                    std::stringstream(out.node.name) >> code;
                }

                StatusCodeTraits statusCodeTraits = GetStatusCodeTrait(code);

                if (!statusCodeTraits.allowBody && !out.node.body.empty()) {
                    // WARN: not empty body
                    std::stringstream ss;
                    ss << "the " << code << " response MUST NOT include a " << SectionName(BodySectionType);

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          EmptyDefinitionWarning,
                                                          sourceMap));
                }
            }
        }

        /** Resolve payload signature */
        static PayloadSignature payloadSignature(const MarkdownNodeIterator& node) {

            mdp::ByteBuffer subject = node->children().front().text;
            mdp::ByteBuffer signature;
            mdp::ByteBuffer remainingContent;

            signature = GetFirstLine(subject, remainingContent);
            TrimString(signature);

            if (RegexMatch(signature, RequestRegex))
                return RequestPayloadSignature;

            if (RegexMatch(signature, ResponseRegex))
                return ResponsePayloadSignature;

            if (RegexMatch(signature, ModelRegex))
                return ModelPayloadSignature;

            return NoPayloadSignature;
        }

        /** Get SectionType from PayloadSignature and nestedSectionType */
        static SectionType getSectionType(PayloadSignature signature,
                                          SectionType nestedType) {

            switch (signature) {
                case RequestPayloadSignature:
                    return (nestedType != UndefinedSectionType) ? RequestSectionType : RequestBodySectionType;

                case ResponsePayloadSignature:
                    return (nestedType != UndefinedSectionType) ? ResponseSectionType : ResponseBodySectionType;

                case ModelPayloadSignature:
                    return (nestedType != UndefinedSectionType) ? ModelSectionType : ModelBodySectionType;

                default:
                    break;
            }

            return UndefinedSectionType;
        }

        /** True if abbreviated section type */
        static bool isAbbreviated(SectionType sectionType) {

            return (sectionType == RequestBodySectionType ||
                    sectionType == ResponseBodySectionType ||
                    sectionType == ModelBodySectionType);
        }

        /** Given the signature, parse it */
        static bool parseSignature(const MarkdownNodeIterator& node,
                                   SectionParserData& pd,
                                   const mdp::ByteBuffer& signature,
                                   ParseResult<Payload>& out) {

            const char* regex;
            mdp::ByteBuffer mediaType;
            CaptureGroups captureGroups;

            switch (pd.sectionContext()) {
                case RequestSectionType:
                case RequestBodySectionType:
                    regex = RequestRegex;
                    break;

                case ResponseSectionType:
                case ResponseBodySectionType:
                    regex = ResponseRegex;
                    break;

                case ModelSectionType:
                case ModelBodySectionType:
                    regex = ModelRegex;
                    break;

                default:
                    return true;
            }

            if (RegexCapture(signature, regex, captureGroups, 5) &&
                !captureGroups.empty()) {

                mdp::ByteBuffer target = signature;
                mdp::ByteBuffer::size_type pos = target.find(captureGroups[0]);

                if (pos != mdp::ByteBuffer::npos) {
                    target.replace(pos, captureGroups[0].length(), mdp::ByteBuffer());
                }

                TrimString(target);

                if (!target.empty()) {
                    // WARN: unable to parse payload signature
                    std::stringstream ss;
                    ss << "unable to parse " << SectionName(pd.sectionContext()) << " signature, expected ";

                    switch (pd.sectionContext()) {
                        case RequestSectionType:
                        case RequestBodySectionType:
                            ss << "'request [<identifier>] [(<media type>)]'";
                            break;

                        case ResponseBodySectionType:
                        case ResponseSectionType:
                            ss << "'response [<HTTP status code>] [(<media type>)]'";
                            break;

                        case ModelSectionType:
                        case ModelBodySectionType:
                            ss << "'model [(<media type>)]'";
                            break;

                        default:
                            return false;
                    }

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          FormattingWarning,
                                                          sourceMap));

                    return false;
                }

                if (pd.sectionContext() == ModelSectionType ||
                    pd.sectionContext() == ModelBodySectionType) {

                    out.node.name = captureGroups[2];
                    mediaType = captureGroups[4];
                } else {
                    out.node.name = captureGroups[1];
                    mediaType = captureGroups[3];
                }

                TrimString(out.node.name);
                TrimString(mediaType);

                if (pd.exportSourceMap() && !out.node.name.empty()) {
                    out.sourceMap.name.sourceMap = node->sourceMap;
                }

                if (!mediaType.empty()) {
                    Header header = std::make_pair(HTTPHeaderName::ContentType, mediaType);
                    out.node.headers.push_back(header);

                    if (pd.exportSourceMap()) {
                        SourceMap<Header> headerSM;
                        headerSM.sourceMap = node->sourceMap;
                        out.sourceMap.headers.collection.push_back(headerSM);
                    }
                }
            }

            return true;
        }

        static bool parseSymbolReference(const MarkdownNodeIterator& node,
                                         SectionParserData& pd,
                                         mdp::ByteBuffer& source,
                                         ParseResult<Payload>& out) {

            SymbolName symbol;
            ResourceModel model;
            SourceMap<ResourceModel> modelSM;

            TrimString(source);

            if (GetSymbolReference(source, symbol)) {
                out.node.symbol = symbol;

                if (pd.exportSourceMap() && !symbol.empty()) {
                    out.sourceMap.symbol.sourceMap = node->sourceMap;
                }

                // If symbol doesn't exist
                if (pd.symbolTable.resourceModels.find(symbol) == pd.symbolTable.resourceModels.end()) {

                    // ERR: Undefined symbol
                    std::stringstream ss;
                    ss << "Undefined symbol " << symbol;

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.error = Error(ss.str(), SymbolError, sourceMap);

                    return true;
                }

                model = pd.symbolTable.resourceModels.at(symbol);
                
                out.node.description = model.description;
                out.node.parameters = model.parameters;
                
                HeaderIterator modelContentType = std::find_if(model.headers.begin(),
                                                               model.headers.end(),
                                                               std::bind2nd(MatchFirstWith<Header, std::string>(),
                                                                            HTTPHeaderName::ContentType));
                
                bool isPayloadContentType = !out.node.headers.empty();
                bool isModelContentType = modelContentType != model.headers.end();
                
                if (isPayloadContentType && isModelContentType) {
                    
                    // WARN: Ignoring payload content-type, when referencing a model with headers
                    std::stringstream ss;
                    
                    ss << "ignoring additional " << SectionName(pd.sectionContext()) << " header(s), ";
                    ss << "specify this header(s) in the referenced model definition instead";
                    
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.warnings.push_back(Warning(ss.str(),
                                                          IgnoringWarning,
                                                          sourceMap));
                }

                if (isPayloadContentType && !isModelContentType) {
                    out.node.headers.insert(out.node.headers.end(), model.headers.begin(), model.headers.end());
                } else {
                    out.node.headers = model.headers;
                }
                
                out.node.body = model.body;
                out.node.schema = model.schema;

                if (pd.exportSourceMap()) {

                    modelSM = pd.symbolSourceMapTable.resourceModels.at(symbol);

                    out.sourceMap.description = modelSM.description;
                    out.sourceMap.parameters = modelSM.parameters;
                    out.sourceMap.body = modelSM.body;
                    out.sourceMap.schema = modelSM.schema;

                    if (isPayloadContentType && !isModelContentType) {
                        out.sourceMap.headers.collection.insert(out.sourceMap.headers.collection.end(), modelSM.headers.collection.begin(), modelSM.headers.collection.end());
                    } else {
                        out.sourceMap.headers = modelSM.headers;
                    }
                }

                return true;
            }

            return false;
        }

        /**
         *  \brief  Find a request within given action.
         *  \param  transaction  A transaction to check.
         *  \param  request A request to look for.
         *  \return Iterator pointing to the matching request within given method requests.
         */
        static RequestIterator findRequest(const TransactionExample& example,
                                           const Request& request) {

            return std::find_if(example.requests.begin(),
                                example.requests.end(),
                                std::bind2nd(MatchPayload(), request));
        }

        /**
         *  \brief  Find a response within responses of a given action.
         *  \param  transaction  A transaction to check.
         *  \param  response A response to look for.
         *  \return Iterator pointing to the matching response within given method requests.
         */
        static ResponseIterator findResponse(const TransactionExample& example,
                                             const Response& response) {

            return std::find_if(example.responses.begin(),
                                example.responses.end(),
                                std::bind2nd(MatchPayload(), response));
        }
    };

    /** Payload Section Parser */
    typedef SectionParser<Payload, ListSectionAdapter> PayloadParser;
}

#endif
