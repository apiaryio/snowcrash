//
//  MarkdownNode.cc
//  markdownparser
//
//  Created by Zdenek Nemec on 4/19/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "MarkdownNode.h"

using namespace mdp;

MarkdownNode::MarkdownNode(MarkdownNodeType type_, MarkdownNode *parent_, const ByteBuffer& text_, const Data& data_)
: type(type_), text(text_), data(data_), m_parent(parent_)
{
    m_children.reset(::new MarkdownNodes);
}
                     
MarkdownNode::MarkdownNode(const MarkdownNode& rhs)
{
    this->type = rhs.type;
    this->text = rhs.text;
    this->data = rhs.data;
    this->sourceMap = rhs.sourceMap;
    this->m_children.reset(::new MarkdownNodes(*rhs.m_children.get()));
    this->m_parent = rhs.m_parent;
}

MarkdownNode& MarkdownNode::operator=(const MarkdownNode& rhs)
{
    this->type = rhs.type;
    this->text = rhs.text;
    this->data = rhs.data;
    this->sourceMap = rhs.sourceMap;
    this->m_children.reset(::new MarkdownNodes(*rhs.m_children.get()));
    this->m_parent = rhs.m_parent;
    return *this;
}

MarkdownNode::~MarkdownNode()
{
}

MarkdownNode& MarkdownNode::parent()
{
    if (!hasParent())
        throw "no parent set";
    return *m_parent;
}

const MarkdownNode& MarkdownNode::parent() const
{
    if (!hasParent())
        throw "no parent set";
    return *m_parent;
}

void MarkdownNode::setParent(MarkdownNode *parent)
{
    m_parent = parent;
}

bool MarkdownNode::hasParent() const
{
    return (m_parent != NULL);
}

MarkdownNodes& MarkdownNode::children()
{
    if (!m_children.get())
        throw "no children set";
    
    return *m_children;
}

const MarkdownNodes& MarkdownNode::children() const
{
    if (!m_children.get())
        throw "no children set";
    
    return *m_children;
}

void MarkdownNode::printNode(size_t level) const
{
    using std::cout;
    for (size_t i = 0; i < level; ++i)
        std::cout << "  ";
    
    cout << "+ ";
    switch (type) {
        case RootMarkdownNodeType:
            cout << "root";
            break;

        case CodeMarkdownNodeType:
            cout << "code";
            break;
            
        case QuoteMarkdownNodeType:
            cout << "quote";
            break;

        case HTMLMarkdownNodeType:
            cout << "HTML";
            break;

        case HeaderMarkdownNodeType:
            cout << "header";
            break;

        case HRuleMarkdownNodeType:
            cout << "hrul";
            break;

        case ListItemMarkdownNodeType:
            cout << "list item";
            break;

        case ParagraphMarkdownNodeType:
            cout << "paragraph";
            break;
            
        default:
            cout << "undefined";
            break;
    }
    
    cout << " (type " << type << ", data " << data << ") - ";
    cout << "`" << text << "`";
    
    if (!sourceMap.empty()) {
        for (mdp::BytesRangeSet::const_iterator it = sourceMap.begin();
             it != sourceMap.end();
             ++it) {
            std::cout << ((it == sourceMap.begin()) ? " :" : ";");
            std::cout << it->location << ":" << it->length;
        }
    }
    
    cout << std::endl;
    
    for (MarkdownNodeIterator it = m_children->begin();
         it != m_children->end();
         ++it) {
        it->printNode(level + 1);
    }
    
    if (level == 0)
        cout << std::endl << std::endl;
}

