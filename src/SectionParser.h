//
//  SectionParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SECTIONPARSER_H
#define SNOWCRASH_SECTIONPARSER_H

#include "SectionProcessor.h"

namespace snowcrash {

    /**
     *  Blueprint section parser
     */
    template<typename T, typename Adapter>
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
                                          T& out) {
            
            MarkdownNodeIterator cur = Adapter::startingNode(node);
            const MarkdownNodes& collection = Adapter::startingNodeSiblings(node, siblings);
            
            // Signature node
            MarkdownNodeIterator lastCur = cur;
            cur = SectionProcessor<T>::processSignature(cur, pd, report, out);
            if (lastCur == cur)
                return Adapter::nextStartingNode(node, siblings, cur);
            
            // Description nodes
            while(cur != collection.end() &&
                  SectionProcessor<T>::isDescriptionNode(cur)) {
                
                lastCur = cur;
                cur = SectionProcessor<T>::processDescription(cur, pd, report, out);
                if (lastCur == cur)
                    return Adapter::nextStartingNode(node, siblings, cur);
            }
            
            // Content nodes
            while(cur != collection.end() &&
                  SectionProcessor<T>::isContentNode(cur)) {
                
                lastCur = cur;
                cur = SectionProcessor<T>::processContent(cur, pd, report, out);
                if (lastCur == cur)
                    return Adapter::nextStartingNode(node, siblings, cur);
            }
            
            // Nested sections
            while(cur != collection.end()) {
                
                lastCur = cur;
                SectionType nested_type = SectionProcessor<T>::nestedSectionType(cur);
                
                pd.sectionsContext.push_back(nested_type);
                
                if (nested_type != UndefinedSectionType) {
                    cur = SectionProcessor<T>::processNestedSection(cur, collection, pd, report, out);
                }
                else if (SectionProcessor<T>::isUnexpectedNode(cur)) {
                    cur = SectionProcessor<T>::processUnexpectedNode(cur, collection, pd, report, out);
                }
                
                pd.sectionsContext.pop_back();
                
                if (lastCur == cur)
                    break;
            }
            
            return Adapter::nextStartingNode(node, siblings, cur);
        }
    };
    
    /** Parser Adapter for parsing header-defined sections */
    struct HeaderSectionAdapter {
        
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
    };
    
    /** Parser Adapter for parsing list-defined sections */
    struct ListSectionAdapter {
        
        static const MarkdownNodeIterator startingNode(const MarkdownNodeIterator& seed) {
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
    };
}

#endif
