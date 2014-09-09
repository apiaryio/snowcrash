//
//  Processor.h
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
     * Compound product of parsing a node
     */
    template<typename T>
    struct ParseResult {
        Report report;           /// Parser's report
        T node;                  /// Parsed AST node
        SourceMap<T> sourceMap;  /// Parsed AST node source map
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
                                                     ParseResult<T>& out) {

            return ++MarkdownNodeIterator(node);
        }

        /** Process section description Markdown node */
        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       ParseResult<T>& out) {

            if (!out.node.description.empty()) {
                TwoNewLines(out.node.description);
            }

            mdp::ByteBuffer content = mdp::MapBytesRangeSet(node->sourceMap, pd.sourceData);

            if (pd.exportSM() && !content.empty()) {
                out.sourceMap.description.sourceMap.append(node->sourceMap);
            }

            out.node.description += content;

            return ++MarkdownNodeIterator(node);
        }
        
        /** Process section-specific content Markdown node */
        static MarkdownNodeIterator processContent(const MarkdownNodeIterator& node,
                                                   const MarkdownNodes& siblings,
                                                   SectionParserData& pd,
                                                   ParseResult<T>& out) {

            return ++MarkdownNodeIterator(node);
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
                                                         ParseResult<T>& out) {

            return node;
        }
        
        /** Process unexpected Markdown node */
        static MarkdownNodeIterator processUnexpectedNode(const MarkdownNodeIterator& node,
                                                          const MarkdownNodes& siblings,
                                                          SectionParserData& pd,
                                                          SectionType& lastSectionType,
                                                          ParseResult<T>& out) {

            // WARN: Ignoring unexpected node
            std::stringstream ss;
            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);

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
                             ParseResult<T>& out) {
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

            SectionTypes nestedTypes = SectionProcessor<T>::nestedSectionTypes();

            if (std::find(nestedTypes.begin(), nestedTypes.end(), keywordSectionType) != nestedTypes.end()) {
                // Node is a keyword defined section defined in one of the nested sections
                // Treat it as a description
                return true;
            }

            return false;
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
            SectionTypes nestedTypes = SectionProcessor<T, TSM>::nestedSectionTypes();

            if (std::find(nestedTypes.begin(), nestedTypes.end(), keywordSectionType) != nestedTypes.end()) {
                return true;
            }
            
            return (keywordSectionType == UndefinedSectionType);
        }

        /** \return Nested sections of the section */
        static SectionTypes nestedSectionTypes() {
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
