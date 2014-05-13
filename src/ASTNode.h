//
//  ASTNode.h
//  markdownparser
//
//  Created by Zdenek Nemec on 4/16/14.
//  Copyright (c) 2014 Apiary. All rights reserved.
//

#ifndef MARKDOWNPARSER_NODE_H
#define MARKDOWNPARSER_NODE_H

#include <deque>
#include <memory>
#include "ByteBuffer.h"

namespace mdp {
    
    /** 
     *  AST block node types 
     */
    enum ASTNodeType {
        RootASTNodeType = 0,
        CodeASTNodeType,
        QuoteASTNodeType,
        HTMLASTNodeType,
        HeaderASTNodeType,
        HRuleASTNodeType,
        ListItemASTNodeType,
        ParagraphASTNodeType,
        TableASTNodeType,
        TableRowASTNodeType,
        TableCellASTNodeType,
        UndefinedASTNodeType = -1
    };

    /* Forward declaration of AST Node */
    class ASTNode;
    
    /** Collection of children nodes */
    typedef std::deque<ASTNode> ChildrenNodes;
    
    /** 
     *  AST node
     */
    class ASTNode {
    public:
        typedef int Data;
        
        /** Node type */
        ASTNodeType type;
        
        /** Textual content, where applicable */
        ByteBuffer text;
        
        /** Additinonal data, if applicable */
        Data data;
        
        /** Source map of the node including any and all children */
        BytesRangeSet sourceMap;
        
        /** Parent node, throws exception if no parent is defined */
        ASTNode& parent();
        const ASTNode& parent() const;
        
        /** Sets parent node */
        void setParent(ASTNode *parent);
        
        /** True if section's parent is specified, false otherwise */
        bool hasParent() const;

        /** Children nodes */
        ChildrenNodes& children();
        const ChildrenNodes& children() const;

        /** Constructor */
        ASTNode(ASTNodeType type_ = UndefinedASTNodeType,
                ASTNode *parent_ = NULL,
                const ByteBuffer& text_ = ByteBuffer(),
                const Data& data_ = Data());

        /** Copy constructor */
        ASTNode(const ASTNode& rhs);
        
        /** Assignment operator */
        ASTNode& operator=(const ASTNode& rhs);
        
        /** Destructor */
        ~ASTNode();
        
    private:
        ASTNode* m_parent;
        std::auto_ptr<ChildrenNodes> m_children;
    };
}

#endif
