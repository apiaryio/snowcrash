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
    struct SectionProcessor<Payload, PayloadSM> : public SectionProcessorBase<Payload, PayloadSM> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     Report& report,
                                                     Payload& out,
                                                     PayloadSM& outSM) {

            mdp::ByteBuffer signature, remainingContent;
            signature = GetFirstLine(node->text, remainingContent);

            parseSignature(node, pd, signature, report, out, outSM);

            // WARN: missing status code
            if (out.name.empty() &&
                (pd.sectionContext() == ResponseSectionType || pd.sectionContext() == ResponseBodySectionType)) {

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning("missing response HTTP status code, assuming 'Response 200'",
                                                  EmptyDefinitionWarning,
                                                  sourceMap));
                out.name = "200";
            }

            if (!remainingContent.empty()) {
                if (!isAbbreviated(pd.sectionContext())) {
                    out.description = remainingContent;

                    if (pd.exportSM() && !out.description.empty()) {
                        outSM.description.append(node->sourceMap);
                    }
                } else if (!parseSymbolReference(node, pd, remainingContent, report, out, outSM)) {

                    // NOTE: NOT THE CORRECT WAY TO DO THIS
                    // https://github.com/apiaryio/snowcrash/commit/a7c5868e62df0048a85e2f9aeeb42c3b3e0a2f07#commitcomment-7322085
                    pd.sectionsContext.push_back(BodySectionType);
                    CodeBlockUtility::signatureContentAsCodeBlock(node, pd, report, out.body);
                    pd.sectionsContext.pop_back();

                    if (pd.exportSM() && !out.body.empty()) {
                        outSM.body.append(node->sourceMap);
                    }
                }
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processContent(const MarkdownNodeIterator& node,
                                                   SectionParserData& pd,
                                                   Report& report,
                                                   Payload& out,
                                                   PayloadSM& outSM) {

            mdp::ByteBuffer content;

            if (!out.symbol.empty()) {
                //WARN: ignoring extraneous content after symbol reference
                std::stringstream ss;

                ss << "ignoring extraneous content after symbol reference";
                ss << ", expected symbol reference only e.g. '[" << out.symbol << "][]'";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning(ss.str(),
                                                  IgnoringWarning,
                                                  sourceMap));
            } else {

                if (!out.body.empty() ||
                    node->type != mdp::ParagraphMarkdownNodeType ||
                    !parseSymbolReference(node, pd, node->text, report, out, outSM)) {

                    // NOTE: NOT THE CORRECT WAY TO DO THIS
                    // https://github.com/apiaryio/snowcrash/commit/a7c5868e62df0048a85e2f9aeeb42c3b3e0a2f07#commitcomment-7322085
                    pd.sectionsContext.push_back(BodySectionType);
                    CodeBlockUtility::contentAsCodeBlock(node, pd, report, content);
                    pd.sectionsContext.pop_back();

                    if (pd.exportSM() && !content.empty()) {
                        outSM.body.append(node->sourceMap);
                    }

                    out.body += content;
                }
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         Payload& out,
                                                         PayloadSM& outSM) {

            switch (pd.sectionContext()) {
                case HeadersSectionType:
                    return HeadersParser::parse(node, siblings, pd, report, out.headers, outSM.headers);

                case BodySectionType:
                    if (!out.body.empty()) {
                        // WARN: Multiple body section
                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        report.warnings.push_back(Warning("ignoring additional 'body' content, it is already defined",
                                                           RedefinitionWarning,
                                                           sourceMap));
                    }

                    return AssetParser::parse(node, siblings, pd, report, out.body, outSM.body);

                case SchemaSectionType:
                    if (!out.schema.empty()) {
                        // WARN: Multiple schema section
                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        report.warnings.push_back(Warning("ignoring additional 'schema' content, it is already defined",
                                                           RedefinitionWarning,
                                                           sourceMap));
                    }

                    return AssetParser::parse(node, siblings, pd, report, out.schema, outSM.schema);

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
                                                          Payload& out,
                                                          PayloadSM& outSM) {

            if ((node->type == mdp::ParagraphMarkdownNodeType ||
                 node->type == mdp::CodeMarkdownNodeType) &&
                sectionType == BodySectionType) {

                mdp::ByteBuffer content = CodeBlockUtility::addDanglingAsset(node, pd, sectionType, report, out.body);

                if (pd.exportSM() && !content.empty()) {
                    outSM.body.append(node->sourceMap);
                }
            } else {

                // WARN: Dangling blocks found
                std::stringstream ss;
                ss << "found dangling " << SectionName(sectionType) << " block";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                report.warnings.push_back(Warning(ss.str(),
                                                  IndentationWarning,
                                                  sourceMap));
            }

            return ++MarkdownNodeIterator(node);
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            if (!isAbbreviated(sectionType) &&
                SectionProcessorBase<Payload, PayloadSM>::isDescriptionNode(node, sectionType)) {

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
            nestedType = SectionProcessor<Headers, HeadersSM>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            // Check if asset section
            nestedType = SectionProcessor<Asset, AssetSM>::sectionType(node);

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
            types = SectionProcessor<Parameters, ParametersSM>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            return nested;
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             Report& report,
                             Payload& out,
                             PayloadSM& outSM) {

            bool warnEmptyBody = false;

            mdp::ByteBuffer contentLength;
            mdp::ByteBuffer transferEncoding;

            SectionType sectionType = pd.sectionContext();

            for (Collection<Header>::const_iterator it = out.headers.begin();
                 it != out.headers.end();
                 ++it) {

                if (it->first == HTTPHeaderName::ContentLength) {
                    contentLength = it->second;
                }

                if (it->first == HTTPHeaderName::TransferEncoding) {
                    transferEncoding = it->second;
                }
            }

            if ((sectionType == RequestSectionType || sectionType == RequestBodySectionType) && out.body.empty()) {

                // Warn when content-length or transfer-encoding is specified or both headers and body are empty
                if (out.headers.empty()) {
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
                    report.warnings.push_back(Warning(ss.str(),
                                                      EmptyDefinitionWarning,
                                                      sourceMap));
                }
            } else if ((sectionType == ResponseSectionType || sectionType == ResponseBodySectionType)) {

                HTTPStatusCode code = 200;

                if (!out.name.empty()) {
                    std::stringstream(out.name) >> code;
                }

                StatusCodeTraits statusCodeTraits = GetStatusCodeTrait(code);

                if (!statusCodeTraits.allowBody && !out.body.empty()) {
                    // WARN: not empty body
                    std::stringstream ss;
                    ss << "the " << code << " response MUST NOT include a " << SectionName(BodySectionType);

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(Warning(ss.str(),
                                                      EmptyDefinitionWarning,
                                                      sourceMap));
                }
            }
        }

        /** Resolve payload signature */
        static PayloadSignature payloadSignature(const MarkdownNodeIterator& node) {

            mdp::ByteBuffer subject = node->children().front().text;
            mdp::ByteBuffer subjectFirstLine;
            mdp::ByteBuffer remainingContent;

            subjectFirstLine = GetFirstLine(subject, remainingContent);
            TrimString(subjectFirstLine);

            if (RegexMatch(subjectFirstLine, RequestRegex))
                return RequestPayloadSignature;

            if (RegexMatch(subjectFirstLine, ResponseRegex))
                return ResponsePayloadSignature;

            if (RegexMatch(subjectFirstLine, ModelRegex))
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
                                   Report& report,
                                   Payload& out,
                                   PayloadSM& outSM) {

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
                    report.warnings.push_back(Warning(ss.str(),
                                                      FormattingWarning,
                                                      sourceMap));

                    return false;
                }

                if(pd.sectionContext() == ModelSectionType ||
                   pd.sectionContext() == ModelBodySectionType){
                    out.name = captureGroups[2];
                    mediaType = captureGroups[4];
                }
                else{
                    out.name = captureGroups[1];
                    mediaType = captureGroups[3];
                }

                TrimString(out.name);
                TrimString(mediaType);

                if (pd.exportSM() && !out.name.empty()) {
                    outSM.name = node->sourceMap;
                }

                if (!mediaType.empty()) {
                    Header header = std::make_pair(HTTPHeaderName::ContentType, mediaType);
                    out.headers.push_back(header);

                    if (pd.exportSM()) {
                        outSM.headers.push_back(node->sourceMap);
                    }
                }
            }

            return true;
        }

        static bool parseSymbolReference(const MarkdownNodeIterator& node,
                                         SectionParserData& pd,
                                         mdp::ByteBuffer& source,
                                         Report& report,
                                         Payload& out,
                                         PayloadSM& outSM) {

            SymbolName symbol;
            ResourceModel model;

            TrimString(source);

            if (GetSymbolReference(source, symbol)) {
                out.symbol = symbol;

                if (pd.exportSM() && !symbol.empty()) {
                    outSM.symbol = node->sourceMap;
                }

                // If symbol doesn't exist
                if (pd.symbolTable.resourceModels.find(symbol) == pd.symbolTable.resourceModels.end()) {

                    // ERR: Undefined symbol
                    std::stringstream ss;
                    ss << "Undefined symbol " << symbol;

                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.error = Error(ss.str(), SymbolError, sourceMap);

                    return true;
                }

                model = pd.symbolTable.resourceModels.at(symbol);

                out.description = model.description;
                out.parameters = model.parameters;
                out.headers = model.headers;
                out.body = model.body;
                out.schema = model.schema;

                return true;
            }

            return false;
        }
    };

    /** Payload Section Parser */
    typedef SectionParser<Payload, PayloadSM, ListSectionAdapter> PayloadParser;
}

#endif
