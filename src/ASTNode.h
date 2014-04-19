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
    
    /** 
     *  AST node
     */
    class ASTNode {
    public:
        typedef std::vector<ASTNode> ChildrenNodes;
        typedef int Data;
        
        /** Node type */
        ASTNodeType type;
        
        /** Textual content, where applicable */
        ByteBuffer text;
        
        /** Additinonal data, if applicable */
        Data data;
        
        /** Children nodes */
        ChildrenNodes children;
        
        /** Source map of the node including any and all children */
        BytesRangeSet sourceMap;
        
        /** Parent node, throws exception if no parent is defined */
        ASTNode& parent() const
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

        ASTNode(ASTNodeType type_ = UndefinedASTNodeType,
                ASTNode *parent_ = NULL,
                const ByteBuffer& text_ = ByteBuffer(),
                const Data& data_ = Data())
        : type(type_), m_parent(parent_), text(text_), data(data_) {}
        
        ASTNode(const ASTNode& rhs)
        {
            this->type = rhs.type;
            this->text = rhs.text;
            this->data = rhs.data;
            this->sourceMap = rhs.sourceMap;
            this->children = rhs.children;
        }
        
        ASTNode& operator=(const ASTNode& rhs)
        {
            this->type = rhs.type;
            this->text = rhs.text;
            this->data = rhs.data;
            this->sourceMap = rhs.sourceMap;
            this->children = rhs.children;
            return *this;
        }

    private:
        ASTNode* m_parent;
    };
}


#endif
