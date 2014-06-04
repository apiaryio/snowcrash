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
    const char* const  ModelRegex = "^[[:blank:]]*" SYMBOL_IDENTIFIER "?[Mm]odel" MEDIA_TYPE "?[[:blank:]]*";

    /**
     * Payload Section Processor
     */
    template<>
    struct SectionProcessor<Payload> : public SectionProcessorBase<Payload> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     Report& report,
                                                     Payload& out) {

            mdp::ByteBuffer signature, remainingContent;
            signature = GetFirstLine(node->text, remainingContent);

            parseSignature(node, pd, signature, report, out);

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
                } else {
                    CodeBlockUtility::signatureContentAsCodeBlock(node, pd, report, out.body);
                }
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       SectionParserData& pd,
                                                       Report& report,
                                                       Payload& out) {

            out.description += (out.description.empty() ? "" : "\n\n") + mdp::MapBytesRangeSet(node->sourceMap, pd.sourceData);
            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processContent(const MarkdownNodeIterator& node,
                                                   SectionParserData& pd,
                                                   Report& report,
                                                   Payload& out) {

            mdp::ByteBuffer content;
            CodeBlockUtility::contentAsCodeBlock(node, pd, report, content);

            out.body += content;
            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         Payload& out) {

            switch (pd.sectionContext()) {
                case HeadersSectionType:
                    return HeadersParser::parse(node, siblings, pd, report, out.headers);

                case BodySectionType:
                    if (!out.body.empty()) {
                        // WARN: Multiple body section
                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        report.warnings.push_back(Warning("ignoring additional 'body' content, it is already defined",
                                                           RedefinitionWarning,
                                                           sourceMap));
                    }

                    return AssetParser::parse(node, siblings, pd, report, out.body);

                case SchemaSectionType:
                    if (!out.schema.empty()) {
                        // WARN: Multiple schema section
                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        report.warnings.push_back(Warning("ignoring additional 'schema' content, it is already defined",
                                                           RedefinitionWarning,
                                                           sourceMap));
                    }

                    return AssetParser::parse(node, siblings, pd, report, out.schema);

                default:
                    break;
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processUnexpectedNode(const MarkdownNodeIterator& node,
                                                          const MarkdownNodes& siblings,
                                                          SectionParserData& pd,
                                                          SectionType& sectionType,
                                                          Report& report,
                                                          Payload& out) {

            std::stringstream ss;

            ss << "found dangling " << SectionName(sectionType) << " block";

            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
            report.warnings.push_back(Warning(ss.str(),
                                              IndentationWarning,
                                              sourceMap));

            return ++MarkdownNodeIterator(node);
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            if (!RecognizeSection(node) &&
                !isAbbreviated(sectionType)) {
                return true;
            }

            return false;
        }

        static bool isContentNode(const MarkdownNodeIterator& node,
                                  SectionType sectionType) {

            if (!RecognizeSection(node) &&
                isAbbreviated(sectionType)) {
                return true;
            }

            return false;
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {

                SectionType nestedType;
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

            // Check if headers section
            nestedType = SectionProcessor<Asset>::sectionType(node);

            if (nestedType != UndefinedSectionType) {
                return nestedType;
            }

            return UndefinedSectionType;
        }

        /** Resolve payload signature */
        static PayloadSignature payloadSignature(const MarkdownNodeIterator& node) {

            mdp::ByteBuffer subject = node->children().front().text;

            if (RegexMatch(subject, RequestRegex))
                return RequestPayloadSignature;

            if (RegexMatch(subject, ResponseRegex))
                return ResponsePayloadSignature;

            if (RegexMatch(subject, ModelRegex))
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
                                   Payload& out) {

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

                out.name = captureGroups[1];
                TrimString(out.name);
                mediaType = captureGroups[3];
                TrimString(mediaType);

                if (!mediaType.empty()) {
                    Header header = std::make_pair(HTTPHeaderName::ContentType, mediaType);
                    out.headers.push_back(header);
                }
            }

            return true;
        }
    };

    /** Payload Section Parser */
    typedef SectionParser<Payload, ListSectionAdapter> PayloadParser;
}

#endif
