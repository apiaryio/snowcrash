//
//  SectionParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SECTIONPARSER_H
#define SNOWCRASH_SECTIONPARSER_H

//#include <algorithm>
//#include <sstream>

#include "Blueprint.h"
#include "SectionProcessor.h"
//#include "SymbolTable.h"

namespace snowcrash {

    /**
     *  \brief Blueprint Parser Options.
     *
     *  Controls blueprint parser behavior.
     */
    enum BlueprintParserOption {
        RenderDescriptionsOption = (1 << 0),    /// < Render Markdown in description.
        RequireBlueprintNameOption = (1 << 1)   /// < Treat missing blueprint name as error
    };
    
    typedef unsigned int BlueprintParserOptions;
    
    /**
     *  Parser Data
     */
    struct ParserData {
        ParserData(BlueprintParserOptions opts,
                   const mdp::ByteBuffer& src,
                   const Blueprint& bp)
        : options(opts), sourceData(src), blueprint(bp) {}

        /** Parser Options */
        BlueprintParserOptions options;

        /** Symbol Table */
        //SymbolTable symbolTable;

        /** Source Data */
        const mdp::ByteBuffer& sourceData;

        /** AST being parsed **/
        const Blueprint& blueprint;

    private:
        ParserData();
        ParserData(const ParserData&);
        ParserData& operator=(const ParserData&);
    };
    
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
                                          const ParserData& pd,
                                          Report& report,
                                          T& out) {
            
            MarkdownNodeIterator cur = Adapter::startingNode(node);
            const MarkdownNodes& collection = Adapter::startingNodeSiblings(node, siblings);
            
            // Signature node
            MarkdownNodeIterator lastCur = cur;
            cur = SectionProcessor<T>::processSignature(cur, out);
            if (lastCur == cur)
                return Adapter::nextStartingNode(node, siblings, cur);
            
            // Description nodes
            while(cur != collection.end() &&
                  SectionProcessor<T>::isDescriptionNode(cur)) {
                
                lastCur = cur;
                cur = SectionProcessor<T>::processDescription(cur, out);
                if (lastCur == cur)
                    return Adapter::nextStartingNode(node, siblings, cur);
            }
            
            // Content nodes
            while(cur != collection.end() &&
                  SectionProcessor<T>::isContentNode(cur)) {
                
                lastCur = cur;
                cur = SectionProcessor<T>::processContent(cur, out);
                if (lastCur == cur)
                    return Adapter::nextStartingNode(node, siblings, cur);
            }
            
            // Nested sections
            while(cur != collection.end()) {
                
                lastCur = cur;
                SectionType nested_type = SectionProcessor<T>::nestedSectionType(cur);
                if (nested_type != UndefinedSectionType) {
                    cur = SectionProcessor<T>::processNestedSection(cur, collection, nested_type, out);
                }
                else if (SectionProcessor<T>::isUnexpectedNode(cur)) {
                    cur = SectionProcessor<T>::processUnexpectedNode(cur, collection, out);
                }
                
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
