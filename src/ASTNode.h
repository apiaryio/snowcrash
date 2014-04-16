//
//  ASTNode.h
//  markdownparser
//
//  Created by Zdenek Nemec on 4/16/14.
//  Copyright (c) 2014 Apiary. All rights reserved.
//

#ifndef MARKDOWNPARSER_NODE_H
#define MARKDOWNPARSER_NODE_H

#include <vector>

namespace mdp {
    
    /** 
     *  AST block node types 
     */
    enum ASTNodeType {
        CodeASTNodeType = 0,
        QuoteASTNodeType,
        HTMLASTNodeType,
        HeaderASTNodeType,
        HRuleASTNodeType,
        ListASTNodeType,
        ListItemASTNodeType,
        ParagraphASTNodeType,
        TableASTNodeType,
        TableRowASTNodeType,
        TableCellASTNodeType,
        UndefinedASTNodeType = -1
    };
    
    /** Generic container for collections */
    template<typename T>
    struct Collection {
        typedef T value_type;
        typedef std::vector<T> type;
        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;
    };
    
    /** 
     *  \brief Source data byte buffer
     *  
     *  Note this is a byte buffer, a sequence of
     *  UTF8 bytes note necessarily characters.
     */
    typedef std::string ByteBuffer;
    
    /** A generic continuous range */
    struct Range {
        size_t location;
        size_t length;
        
        Range(size_t loc = 0, size_t len = 0)
        : location(loc), length(len) {}
    };
    
    /** Range of bytes */
    typedef BytesRange Range;
    
    /** Range of characters */
    typedef CharactersRange Range;
    
    /** A generic set of non-continuous of ranges */
    template<typename T>
    class RangeSet : public std::vector<T> {
    public:
        // TODO:
        // void append(const value_type& val);
    };
    
    /** Set of non-continuous byte ranges */
    typedef BytesRangeSet : RangeSet<BytesRange>;
    
    /** Set of non-continuous character ranges */
    typedef CharactersRangeSet : RangeSet<CharactersRange>;
    
    /** Map Ranges of bytes to ranges of characters */
    CharactersRangeSet MapBytesToCharacters(const BytesRangeSet& rangeSet, const ByteBuffer& byteBuffer);
    
    /** 
     *  AST node
     */
    class ASTNode {
    public:
        /** Node type */
        ASTNodeType type;
        
        /** Textual content, where applicable */
        ByteBuffer text;
        
        /** Children nodes */
        std::vector<ASTNode> children;
        
        /** Source map of the node including any and all children */
        BytesRangeSet sourceMap;
        
        /** Parent node, throws exception if no parent is defined */
        const ASTNode& parent() const
        {
            if (!hasParent())
                throw "no parent set";
            return *m_parent;
        }
        
        /** True if section's parent is specified, false otherwise */
        bool hasParent() const
        {
            return (m_parent != NULL);
        }
        
        ASTNode()
        : type(UndefinedASTNodeType), m_parent(nullptr) {}
        
    private:
        ASTNode* m_parent;
    };
}


#endif
