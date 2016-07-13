//
//  SectionProcessor.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/14/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SECTIONPROCESSOR_H
#define SNOWCRASH_SECTIONPROCESSOR_H

#include "SectionParserData.h"
#include "SourceAnnotation.h"
#include "Signature.h"

// Use the following macro whenever a section doesn't have description
#define NO_SECTION_DESCRIPTION(T)\
static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,\
                                               const MarkdownNodes& siblings,\
                                               SectionParserData& pd,\
                                               const ParseResultRef<T>& out) {\
    return node;\
}\
\
static bool isDescriptionNode(const MarkdownNodeIterator& node,\
                              SectionType sectionType) {\
    return false;\
}\

namespace snowcrash {

    using mdp::MarkdownNode;
    using mdp::MarkdownNodes;
    using mdp::MarkdownNodeIterator;

    typedef Collection<SectionType>::type SectionTypes;

    /**
     *  Layout of the section being parsed
     */
    enum SectionLayout {
        DefaultSectionLayout,          /// Default Section Layout: Signature > Description > Content > Nested
        ExclusiveNestedSectionLayout,  /// Section is composed of nested sections only
        RedirectSectionLayout          /// Section should be parsed by another parser as whole
    };

    /**
     *  \brief Complete compound product of parsing
     *
     *  This structure owns all of the product data. As such
     *  it shouldn't be used internally unless there is a need
     *  for storing complete parser data. See %IntermediateParseResult.
     */
    template<typename T>
    struct ParseResult {

        ParseResult(const Report& report_ = Report())
        : report(report_) {}

        Report report;           /// Parser's report
        T node;                  /// Parsed AST node
        SourceMap<T> sourceMap;  /// Parsed AST node source map
    };

    /**
     *  \brief Partial product of parsing.
     *
     *  This structure owns the node being parsed and its source map.
     *  Unlike %ParseResult it relies on shared parser report data making it
     *  ideal for holding temporary results while parsing items of a collection.
     */
    template<typename T>
    struct IntermediateParseResult {

        explicit IntermediateParseResult(Report& report_)
        : report(report_) {}

        Report& report;
        T node;
        SourceMap<T> sourceMap;
    };

    /**
     *  \brief Reference wrapper for parsing data product
     *
     *  Reference wrapper for %ParseResult and %IntermediateParseResult.
     */
    template<typename T>
    struct ParseResultRef {

        ParseResultRef(ParseResult<T>& parseResult)
        : report(parseResult.report), node(parseResult.node), sourceMap(parseResult.sourceMap) {}

        ParseResultRef(IntermediateParseResult<T>& parseResult)
        : report(parseResult.report), node(parseResult.node), sourceMap(parseResult.sourceMap) {}

        ParseResultRef(Report& report_, T& node_, SourceMap<T>& sourceMap_)
        : report(report_), node(node_), sourceMap(sourceMap_) {}

        Report& report;
        T& node;
        SourceMap<T>& sourceMap;

    private:
        ParseResultRef();
    };

    /*
     * Forward Declarations
     */
    template<typename T>
    struct SectionProcessor;

    /**
     *  \brief  Section Processor Base
     *
     *  Defines section processor interface alongised with its default
     *  behavior.
     */
    template<typename T>
    struct SectionProcessorBase {

        /**
         *  \brief Process section signature Markdown node
         *  \param node     Node to process
         *  \param siblings Siblings of the node being processed
         *  \param pd       Section parser state
         *  \param report   Process log report
         *  \param out      Processed output
         *  \return Result of the process operation
         */
        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     const ParseResultRef<T>& out) {

            return ++MarkdownNodeIterator(node);
        }

        /** Process section description Markdown node */
        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       const ParseResultRef<T>& out) {

            if (!out.node.description.empty()) {
                TwoNewLines(out.node.description);
            }

            mdp::ByteBuffer content = mdp::MapBytesRangeSet(node->sourceMap, pd.sourceData);

            if (pd.exportSourceMap() && !content.empty()) {
                out.sourceMap.description.sourceMap.append(node->sourceMap);
            }

            out.node.description += content;

            return ++MarkdownNodeIterator(node);
        }

        /** Process section-specific content Markdown node */
        static MarkdownNodeIterator processContent(const MarkdownNodeIterator& node,
                                                   const MarkdownNodes& siblings,
                                                   SectionParserData& pd,
                                                   const ParseResultRef<T>& out) {

            return ++MarkdownNodeIterator(node);
        }

        /**
         * Pre-process for nested sections, basically a look-ahead before processing nested sections
         * Only used by BlueprintParser for now
         */
        static void preprocessNestedSections(const MarkdownNodeIterator& node,
                                             const MarkdownNodes& siblings,
                                             SectionParserData& pd,
                                             const ParseResultRef<T>& out) {
        }

        /**
         *  \brief Process nested sections Markdown node(s)
         *  \param node     Node to process
         *  \param siblings Siblings of the node being processed
         *  \param pd       Section parser state
         *  \param report   Process log report
         *  \param out      Processed output
         */
        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<T>& out) {

            return node;
        }

        /** Process unexpected Markdown node */
        static MarkdownNodeIterator processUnexpectedNode(const MarkdownNodeIterator& node,
                                                          const MarkdownNodes& siblings,
                                                          SectionParserData& pd,
                                                          SectionType& lastSectionType,
                                                          const ParseResultRef<T>& out) {

            // WARN: Ignoring unexpected node
            std::stringstream ss;
            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);

            if (node->type == mdp::HeaderMarkdownNodeType) {
                ss << "unexpected header block, expected a group, resource or an action definition";
                ss << ", e.g. '# Group <name>', '# <resource name> [<URI>]' or '# <HTTP method> <URI>'";
            } else {
                ss << "ignoring unrecognized block";
            }

            out.report.warnings.push_back(Warning(ss.str(),
                                                  IgnoringWarning,
                                                  sourceMap));

            return ++MarkdownNodeIterator(node);
        }

        /** Final validation after processing */
        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<T>& out) {
        }

        /** \return True if the node is a section description node */
        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            if (SectionProcessor<T>::isContentNode(node, sectionType) ||
                SectionProcessor<T>::nestedSectionType(node) != UndefinedSectionType) {
                return false;
            }

            SectionType keywordSectionType = SectionKeywordSignature(node);

            if (keywordSectionType == UndefinedSectionType) {
                return true;
            }

            SectionTypes upperTypes = SectionProcessor<T>::upperSectionTypes();

            if (std::find(upperTypes.begin(), upperTypes.end(), keywordSectionType) != upperTypes.end()) {
                // Node is a keyword defined section defined in an upper level section
                return false;
            }

            return true;
        }

        /** \return True if the node is a section-specific content node */
        static bool isContentNode(const MarkdownNodeIterator& node,
                                  SectionType sectionType) {
            return false;
        }

        /** \return True if the node is unexpected in the current context */
        static bool isUnexpectedNode(const MarkdownNodeIterator& node,
                                     SectionType sectionType) {

            SectionType keywordSectionType = SectionKeywordSignature(node);
            SectionTypes upperTypes = SectionProcessor<T>::upperSectionTypes();

            if (std::find(upperTypes.begin(), upperTypes.end(), keywordSectionType) == upperTypes.end()) {
                // Node is not a section that is upper level
                return true;
            }

            return (keywordSectionType == UndefinedSectionType);
        }

        /** \return All upper level sections of the section */
        static SectionTypes upperSectionTypes() {
            return SectionTypes();
        }

        /** \return %SectionType of the node */
        static SectionType sectionType(const MarkdownNodeIterator& node) {
            return UndefinedSectionType;
        }

        /** \return Nested %SectionType of the node */
        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {
            return UndefinedSectionType;
        }
    };

    /**
     *  Default Section Processor
     */
    template<typename T>
    struct SectionProcessor : public SectionProcessorBase<T> {
    };
}

#endif
