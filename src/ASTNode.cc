//
//  ASTNode.cc
//  markdownparser
//
//  Created by Zdenek Nemec on 4/19/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "ASTNode.h"

using namespace mdp;

ASTNode::ASTNode(ASTNodeType type_, ASTNode *parent_, const ByteBuffer& text_, const Data& data_)
: type(type_), m_parent(parent_), text(text_), data(data_)
{
    m_children.reset(::new ChildrenNodes);
}
                     
ASTNode::ASTNode(const ASTNode& rhs)
{
    this->type = rhs.type;
    this->text = rhs.text;
    this->data = rhs.data;
    this->sourceMap = rhs.sourceMap;
    this->m_children.reset(::new ChildrenNodes(*rhs.m_children.get()));
    this->m_parent = rhs.m_parent;
}

ASTNode& ASTNode::operator=(const ASTNode& rhs)
{
    this->type = rhs.type;
    this->text = rhs.text;
    this->data = rhs.data;
    this->sourceMap = rhs.sourceMap;
    this->m_children.reset(::new ChildrenNodes(*rhs.m_children.get()));
    this->m_parent = rhs.m_parent;
    return *this;
}

ASTNode::~ASTNode()
{
}

ASTNode& ASTNode::parent()
{
    if (!hasParent())
        throw "no parent set";
    return *m_parent;
}

const ASTNode& ASTNode::parent() const
{
    if (!hasParent())
        throw "no parent set";
    return *m_parent;
}

void ASTNode::setParent(ASTNode *parent)
{
    m_parent = parent;
}

bool ASTNode::hasParent() const
{
    return (m_parent != NULL);
}

ChildrenNodes& ASTNode::children()
{
    if (!m_children.get())
        throw "no children set";
    
    return *m_children;
}

const ChildrenNodes& ASTNode::children() const
{
    if (!m_children.get())
        throw "no children set";
    
    return *m_children;
}
