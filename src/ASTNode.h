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
#include "ByteBuffer.h"

namespace mdp {
    
    /** 
     *  AST block node types 
     */
    enum ASTNodeType {
        RootASTNode = 0,
        CodeASTNodeType,
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
    
    // TODO: Remove
//    /** Generic container for collections */
//    template<typename T>
//    struct Collection {
//        typedef T value_type;
//        typedef std::vector<T> type;
//        typedef typename std::vector<T>::iterator iterator;
//        typedef typename std::vector<T>::const_iterator const_iterator;
//    };
    
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
        
        /** Sets parent node */
        void setParent(ASTNode *parent)
        {
            m_parent = parent;
        }
        
        /** True if section's parent is specified, false otherwise */
        bool hasParent() const
        {
            return (m_parent != NULL);
        }
        
        ASTNode()
        : type(UndefinedASTNodeType), m_parent(NULL) {}

        ASTNode(ASTNodeType t, ASTNode *p, const ByteBuffer& txt = ByteBuffer())
        : type(t), m_parent(p), text(txt) {}

    private:
        ASTNode* m_parent;
    };
}


#endif
