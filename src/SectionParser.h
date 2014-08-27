//
//  SectionParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SECTIONPARSER_H
#define SNOWCRASH_SECTIONPARSER_H

#include <stdexcept>
#include "SectionProcessor.h"

#define ADAPTER_MISMATCH_ERR std::logic_error("mismatched adapter and node type")

namespace snowcrash {

    /**
     *  Blueprint section parser
     */
    template<typename T, typename TSM, typename Adapter>
    struct SectionParser {

        
        /**
         *  \brief  Parse a section of blueprint
         *  \param  node    Initial node to start parsing at
         *  \param  siblings    Siblings of the initial node
         *  \param  pd      Parser data
         *  \param  T       Parsed output
         *  \return Iterator to the first unparsed block
         */
        static MarkdownNodeIterator parse(const MarkdownNodeIterator& node,
                                          const MarkdownNodes& siblings,
                                          SectionParserData& pd,
                                          Report& report,
                                          T& out,
                                          TSM& outSM) {
            
            SectionLayout layout = DefaultSectionLayout;
            MarkdownNodeIterator cur = Adapter::startingNode(node);
            const MarkdownNodes& collection = Adapter::startingNodeSiblings(node, siblings);
            
            // Signature node
            MarkdownNodeIterator lastCur = cur;
            cur = SectionProcessor<T, TSM>::processSignature(cur, collection, pd, layout, report, out, outSM);

            // Exclusive Nested Sections Layout
            if (layout == ExclusiveNestedSectionLayout) {

                cur = parseNestedSections(cur, collection, pd, report, out, outSM);
                
                SectionProcessor<T, TSM>::finalize(node, pd, report, out, outSM);
                
                return Adapter::nextStartingNode(node, siblings, cur);
            }
            
            // Parser redirect layout
            if (layout == RedirectSectionLayout) {
                SectionProcessor<T, TSM>::finalize(node, pd, report, out, outSM);
                
                return Adapter::nextStartingNode(node, siblings, cur);
            }

            // Default layout
            if (lastCur == cur)
                return Adapter::nextStartingNode(node, siblings, cur);
            
            // Description nodes
            while(cur != collection.end() &&
                  SectionProcessor<T, TSM>::isDescriptionNode(cur, pd.sectionContext())) {
                
                lastCur = cur;
                cur = SectionProcessor<T, TSM>::processDescription(cur, collection, pd, report, out, outSM);
                
                if (lastCur == cur)
                    return Adapter::nextStartingNode(node, siblings, cur);
            }
            
            // Content nodes
            while(cur != collection.end() &&
                  SectionProcessor<T, TSM>::isContentNode(cur, pd.sectionContext())) {
                
                lastCur = cur;
                cur = SectionProcessor<T, TSM>::processContent(cur, collection, pd, report, out, outSM);
                
                if (lastCur == cur)
                    return Adapter::nextStartingNode(node, siblings, cur);
            }
            
            // Nested Sections
            cur = parseNestedSections(cur, collection, pd, report, out, outSM);

            SectionProcessor<T, TSM>::finalize(node, pd, report, out, outSM);

            return Adapter::nextStartingNode(node, siblings, cur);
        }
        
        
        /** Parse nested sections */
        static MarkdownNodeIterator parseNestedSections(const MarkdownNodeIterator& node,
                                                        const MarkdownNodes& collection,
                                                        SectionParserData& pd,
                                                        Report& report,
                                                        T& out,
                                                        TSM& outSM) {

            MarkdownNodeIterator cur = node;
            MarkdownNodeIterator lastCur = cur;
            
            SectionType lastSectionType = UndefinedSectionType;
            
            // Nested sections
            while(cur != collection.end()) {
                
                lastCur = cur;
                SectionType nestedType = SectionProcessor<T, TSM>::nestedSectionType(cur);
                
                pd.sectionsContext.push_back(nestedType);
                
                if (nestedType != UndefinedSectionType) {
                    cur = SectionProcessor<T, TSM>::processNestedSection(cur, collection, pd, report, out, outSM);
                }
                else if (Adapter::nextSkipsUnexpected ||
                         SectionProcessor<T, TSM>::isUnexpectedNode(cur, pd.sectionContext())) {
                    cur = SectionProcessor<T, TSM>::processUnexpectedNode(cur, collection, pd, lastSectionType, report, out, outSM);
                }
                
                if (cur != collection.end() &&
                    (pd.sectionContext() != UndefinedSectionType ||
                     (cur->type != mdp::ParagraphMarkdownNodeType &&
                      cur->type != mdp::CodeMarkdownNodeType))) {
                         
                    lastSectionType = pd.sectionContext();
                }
                
                pd.sectionsContext.pop_back();
                
                if (lastCur == cur)
                    break;
            }
            
            return cur;
        }
    };
    
    /** Parser Adapter for parsing header-defined sections */
    struct HeaderSectionAdapter {
        
        /** \return Node to start parsing with */
        static const MarkdownNodeIterator startingNode(const MarkdownNodeIterator& seed) {
            if (seed->type != mdp::HeaderMarkdownNodeType)
                throw ADAPTER_MISMATCH_ERR;
            
            return seed;
        }
        
        /** \return Collection of siblings to starting Node */
        static const MarkdownNodes& startingNodeSiblings(const MarkdownNodeIterator& seed,
                                                         const MarkdownNodes& siblings) {
            return siblings;
        }
        
        /** \return Starting node for next parsing */
        static const MarkdownNodeIterator nextStartingNode(const MarkdownNodeIterator& seed,
                                                           const MarkdownNodes& siblings,
                                                           const MarkdownNodeIterator& cur) {
            return cur;
        }
        
        /** 
         *  \brief Adapter Markdown node skipping behavior trait.
         *
         *  Adapter trait signalizing that the adapter can possibly skip some Markdown nodes on a nextStartingNode() call.
         *  If set to true, a call to nextStartingNode() can skip some nodes causing some information loss. False otherwise.
         */
        static const bool nextSkipsUnexpected = false;
    };
    
    /** Parser Adapter for parsing list-defined sections */
    struct ListSectionAdapter {
        
        static const MarkdownNodeIterator startingNode(const MarkdownNodeIterator& seed) {
            if (seed->type != mdp::ListItemMarkdownNodeType)
                throw ADAPTER_MISMATCH_ERR;
            
            return seed->children().begin();
        }
        
        static const MarkdownNodes& startingNodeSiblings(const MarkdownNodeIterator& seed,
                                                         const MarkdownNodes& siblings) {
            return seed->children();
        }
        
        static const MarkdownNodeIterator nextStartingNode(const MarkdownNodeIterator& seed,
                                                           const MarkdownNodes& siblings,
                                                           const MarkdownNodeIterator& cur) {
            if (seed == siblings.end())
                return seed;
            
            return ++MarkdownNodeIterator(seed);
        }
        
        static const bool nextSkipsUnexpected = true;
    };

    /** Parser Adapter for parsing blueprint sections */
    struct BlueprintSectionAdapter {

        /** \return Node to start parsing with */
        static const MarkdownNodeIterator startingNode(const MarkdownNodeIterator& seed) {
            return seed;
        }

        /** \return Collection of siblings to starting Node */
        static const MarkdownNodes& startingNodeSiblings(const MarkdownNodeIterator& seed,
                                                         const MarkdownNodes& siblings) {
            return siblings;
        }

        /** \return Starting node for next parsing */
        static const MarkdownNodeIterator nextStartingNode(const MarkdownNodeIterator& seed,
                                                           const MarkdownNodes& siblings,
                                                           const MarkdownNodeIterator& cur) {
            return cur;
        }
        
        static const bool nextSkipsUnexpected = false;
    };
}

#endif
