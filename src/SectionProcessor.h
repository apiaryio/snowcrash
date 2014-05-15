//
//  Processor.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/14/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SECTIONPROCESSOR_H
#define SNOWCRASH_SECTIONPROCESSOR_H

#include "MarkdownNode.h"
#include "Section.h"
#include "Signature.h"
#include "SourceAnnotation.h"

namespace snowcrash {
    
    using mdp::MarkdownNode;
    using mdp::MarkdownNodes;
    using mdp::MarkdownNodeIterator;
    
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
         *  \param out      Processed output
         *  \return Result of the process operation
         */
        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                       T& out) {
            return ++MarkdownNodeIterator(node);
        }

        /** Process section description Markdown node */
        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                         T& out) {
            out.description += node->text;
            return ++MarkdownNodeIterator(node);
        }
        
        /** Process section-specific content Markdown node */
        static MarkdownNodeIterator processContent(const MarkdownNodeIterator& node,
                                     T& out) {
            return ++MarkdownNodeIterator(node);
        }
        
        /**
         *  \brief Process nested sections Markdown node(s)
         *  \param node     Node to process
         *  \param siblings Siblings of the node being processed
         *  \param SectionType  Type of the nested section to process
         *  \param out      Processed output
         */
        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                           const MarkdownNodes& siblings,
                                           SectionType& type,
                                           T& out) {
            return node;
        }
        
        /** Process unexpected Markdown node */
        static MarkdownNodeIterator processUnexpectedNode(const MarkdownNodeIterator& node,
                                            const MarkdownNodes& siblings,
                                            T& out) {
            return ++MarkdownNodeIterator(node);
        }
        
        /** \return True if the node is a section description node */
        static bool isDescriptionNode(const MarkdownNodeIterator& node) {
            return  !SectionProcessor<T>::isContentNode(node) &&
                    SectionProcessor<T>::nestedSectionType(node) == UndefinedSectionType &&
                    !RecognizeSection(node);
        }

        /** \return True if the node is a section-specific content node */
        static bool isContentNode(const MarkdownNodeIterator& node) {
            return false;
        }

        /** \return True if the node is unexpected in the current context */
        static bool isUnexpectedNode(const MarkdownNodeIterator& node) {
            return !RecognizeSection(node);
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
