//
//  HeaderParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/22/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_HEADERPARSER_H
#define SNOWCRASH_HEADERPARSER_H

#include "SectionParser.h"
#include "RegexMatch.h"
#include "CodeBlockUtility.h"
#include "StringUtility.h"
#include "BlueprintUtility.h"
#include "RegexMatch.h"

#include <string.h>

namespace snowcrash {

/** Headers matching regex */
const char *const HeadersRegex = "^[[:blank:]]*[Hh]eaders?[[:blank:]]*$";

/** Header Iterator in its containment group */
typedef Collection<Header>::const_iterator HeaderIterator;

/** Base class for functor to check validity of parsed header */
struct ValidateFunctorBase
{

    /** intended to generate warning mesage */
    virtual std::string getMessage() const = 0;

    /**
     * intended to invoke validation
     * \return true if validation is ok
     *
     * data for validation you can inject into functor via c-tor
     */
    virtual bool operator()() const = 0;
};

/** Functor implementation for check header name is valid token according to
 * specification \see http://tools.ietf.org/html/rfc7230#section-3.2.6 */
struct HeaderNameTokenChecker : public ValidateFunctorBase
{

    const std::string &headerName;

    explicit HeaderNameTokenChecker(const std::string &headerName)
        : headerName(headerName) {}

    virtual bool operator()() const;
    virtual std::string getMessage() const;
};

/** Functor implementation for check header contains colon character between
 * name and value */
struct ColonPresentedChecker : public ValidateFunctorBase
{

    const CaptureGroups &captures;

    explicit ColonPresentedChecker(const CaptureGroups &captures)
        : captures(captures) {}

    virtual bool operator()() const;
    virtual std::string getMessage() const;
};

/** Functor implementation to check Headers duplicity */
struct HeadersDuplicateChecker : public ValidateFunctorBase
{

    const Header &header;
    const Headers &headers;

    explicit HeadersDuplicateChecker(
        const Header &header, const Headers &headers)
        : header(header), headers(headers) {}

    virtual bool operator()() const;
    virtual std::string getMessage() const;
};

/** Functor implementation to check Headers duplicity */
struct HeaderValuePresentedChecker : public ValidateFunctorBase
{

    const Header &header;

    explicit HeaderValuePresentedChecker(const Header &header)
        : header(header) {}

    virtual bool operator()() const;
    virtual std::string getMessage() const;
};

/** Functor receive and invoke individual Validators and conditionaly push
 * reports  */
struct HeaderParserValidator
{

    const ParseResultRef<Headers> &out;
    mdp::CharactersRangeSet sourceMap;

    HeaderParserValidator(
        const ParseResultRef<Headers> &out, mdp::CharactersRangeSet sourceMap)
        : out(out), sourceMap(sourceMap) {}

    bool operator()(const ValidateFunctorBase &rule);
};

/**
 *  Headers Section Processor
 */
template <>
struct SectionProcessor<Headers> : public SectionProcessorBase<Headers>
{

    static MarkdownNodeIterator processSignature(
        const MarkdownNodeIterator &node,
        const MarkdownNodes &siblings,
        SectionParserData &pd,
        SectionLayout &layout,
        const ParseResultRef<Headers> &out) {

        mdp::ByteBuffer content;
        CodeBlockUtility::signatureContentAsCodeBlock(
            node, pd, out.report, content);

        // drop first line (it contain " + Headers")
        // we need just "content"
        mdp::BytesRangeSet::const_iterator begin = node->sourceMap.begin();
        begin++;

        headersFromContent(node, begin, node->sourceMap.end(), pd, out);

        return ++MarkdownNodeIterator(node);
    }

    NO_SECTION_DESCRIPTION(Headers)

    static MarkdownNodeIterator processContent(const MarkdownNodeIterator &node,
        const MarkdownNodes &siblings,
        SectionParserData &pd,
        const ParseResultRef<Headers> &out) {

        mdp::ByteBuffer content;
        CodeBlockUtility::contentAsCodeBlock(node, pd, out.report, content);

        headersFromContent(
            node, node->sourceMap.begin(), node->sourceMap.end(), pd, out);

        return ++MarkdownNodeIterator(node);
    }

    static bool isContentNode(
        const MarkdownNodeIterator &node, SectionType sectionType) {

        return (SectionKeywordSignature(node) == UndefinedSectionType);
    }

    static SectionType sectionType(const MarkdownNodeIterator &node) {

        if (node->type == mdp::ListItemMarkdownNodeType &&
            !node->children().empty()) {

            mdp::ByteBuffer subject = node->children().front().text;
            mdp::ByteBuffer signature;
            mdp::ByteBuffer remainingContent;

            signature = GetFirstLine(subject, remainingContent);
            TrimString(signature);

            if (RegexMatch(signature, HeadersRegex))
                return HeadersSectionType;
        }

        return UndefinedSectionType;
    }

    static void finalize(const MarkdownNodeIterator &node,
        SectionParserData &pd,
        const ParseResultRef<Headers> &out) {

        if (out.node.empty()) {

            // WARN: No valid headers defined
            mdp::CharactersRangeSet sourceMap =
                mdp::BytesRangeSetToCharactersRangeSet(
                    node->sourceMap, pd.sourceCharacterIndex);
            out.report.warnings.push_back(Warning(
                "no valid headers specified", FormattingWarning, sourceMap));
        }
    }

    /**
     * Parse individual line of header
     * \return true if valid header definition
     *
     * Header name is checked against token definition
     *
     * \param line - contains individual line with header definition
     * \param header - is filled by name and value if definition is valid
     * \param out - "report" member can receive warning while checking validity
     * \param sourceMap - just contain source mapping for warning report
     */
    static bool parseHeaderLine(const mdp::ByteBuffer &line,
        Header &header,
        const ParseResultRef<Headers> &out,
        const mdp::CharactersRangeSet sourceMap) {

        std::string re = "^ *([^:[:blank:]]+)(( *:? *)(.*)?)$";

        CaptureGroups parts;
        bool matched = RegexCapture(line, re, parts, 5);

        if (!matched) {
            // WARN: unable to parse header
            out.report.warnings.push_back(
                Warning("unable to parse HTTP header, expected '<header name> "
                        ": <header value>', one header per line",
                    FormattingWarning,
                    sourceMap));
            return false;
        }

        header = std::make_pair(parts[1], parts[4]);
        TrimString(header.second);

        HeaderParserValidator validate(out, sourceMap);

        if (!validate(HeaderNameTokenChecker(header.first))) {
            return false;
        }

        validate(ColonPresentedChecker(parts));
        validate(HeadersDuplicateChecker(header, out.node));
        validate(HeaderValuePresentedChecker(header));

        return !header.first.empty();
    }

    static bool fetchLine(
        const std::string &input, mdp::BytesRange &map, std::string &line) {

        if (input.length() < (map.location + map.length)) {
            return false;
        }

        TrimRange trim = GetTrimInfo(input.begin() + map.location,
            input.begin() + map.location + map.length);

        map.length = std::get<1>(trim);

        if (map.length <= 0) {
            return false;
        }

        map.location += std::get<0>(trim);

        line = input.substr(map.location, map.length);

        return true;
    }

    static bool isCodeFence(const std::string &line) {
        return (
            !memcmp(line.c_str(), "```", 3) || !memcmp(line.c_str(), "~~~", 3));
    }

    /** Retrieve headers from content */
    static void headersFromContent(const MarkdownNodeIterator &node,
        mdp::BytesRangeSet::const_iterator from,
        mdp::BytesRangeSet::const_iterator to,
        const SectionParserData &pd,
        const ParseResultRef<Headers> &out) {

        bool inCodeFence = false;

        for (mdp::BytesRangeSet::const_iterator it = from; it != to; it++) {

            mdp::BytesRange map(*it);
            std::string line;

            if (!fetchLine(pd.sourceData, map, line)) {
                continue;
            }

            if (it == from) {
                inCodeFence = isCodeFence(line);
            };

            if (inCodeFence && isCodeFence(line)) {
                continue;
            }

            Header header;

            mdp::BytesRangeSet byteMap;
            byteMap.push_back(map);
            mdp::CharactersRangeSet sourceMap =
                mdp::BytesRangeSetToCharactersRangeSet(
                    byteMap, pd.sourceCharacterIndex);

            if (parseHeaderLine(line, header, out, sourceMap)) {
                out.node.push_back(header);

                if (pd.exportSourceMap()) {
                    SourceMap<Header> headerSM;
                    headerSM.sourceMap = sourceMap;
                    out.sourceMap.collection.push_back(headerSM);
                }
            }
        }
    }

    /** Inject headers into transaction examples requests and responses */
    static void injectDeprecatedHeaders(SectionParserData &pd,
        const Headers &headers,
        const SourceMap<Headers> &headersSM,
        TransactionExamples &examples,
        SourceMap<TransactionExamples> &examplesSM) {

        Collection<TransactionExample>::iterator exampleIt = examples.begin();
        Collection<SourceMap<TransactionExample>>::iterator exampleSourceMapIt;

        if (pd.exportSourceMap()) {
            exampleSourceMapIt = examplesSM.collection.begin();
        }

        while (exampleIt != examples.end()) {

            Collection<Request>::iterator requestIt =
                exampleIt->requests.begin();
            Collection<SourceMap<Request>>::iterator requestSourceMapIt;

            if (pd.exportSourceMap()) {
                requestSourceMapIt =
                    exampleSourceMapIt->requests.collection.begin();
            }

            // Requests
            while (requestIt != exampleIt->requests.end()) {

                requestIt->headers.insert(
                    requestIt->headers.begin(), headers.begin(), headers.end());
                ++requestIt;

                if (pd.exportSourceMap()) {
                    requestSourceMapIt->headers.collection.insert(
                        requestSourceMapIt->headers.collection.begin(),
                        headersSM.collection.begin(),
                        headersSM.collection.end());
                    ++requestSourceMapIt;
                }
            }

            Collection<Response>::iterator responseIt =
                exampleIt->responses.begin();
            Collection<SourceMap<Response>>::iterator responseSourceMapIt;

            if (pd.exportSourceMap()) {
                responseSourceMapIt =
                    exampleSourceMapIt->responses.collection.begin();
            }

            // Responses
            while (responseIt != exampleIt->responses.end()) {

                responseIt->headers.insert(responseIt->headers.begin(),
                    headers.begin(),
                    headers.end());
                ++responseIt;

                if (pd.exportSourceMap()) {
                    responseSourceMapIt->headers.collection.insert(
                        responseSourceMapIt->headers.collection.begin(),
                        headersSM.collection.begin(),
                        headersSM.collection.end());
                    ++responseSourceMapIt;
                }
            }

            ++exampleIt;

            if (pd.exportSourceMap()) {
                ++exampleSourceMapIt;
            }
        }
    }
};

/** Headers Section Parser */
typedef SectionParser<Headers, ListSectionAdapter> HeadersParser;
}

#endif
