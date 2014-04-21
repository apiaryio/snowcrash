//
//  MarkdownParser.cc
//  markdownparser
//
//  Created by Zdenek Nemec on 4/18/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include <cstring>
#include <stdexcept>
#include "MarkdownParser.h"

using namespace mdp;

const size_t MarkdownParser::OutputUnitSize = 64;
const size_t MarkdownParser::MaxNesting = 16;
const int MarkdownParser::ParserExtensions = MKDEXT_FENCED_CODE | MKDEXT_NO_INTRA_EMPHASIS | MKDEXT_LAX_SPACING /*| MKDEXT_TABLES */;

#define NO_WORKING_NODE_ERR std::logic_error("no working node")
#define WORKING_NODE_MISMATCH_ERR std::logic_error("working node mismatch")

/**
 *  \brief  Create a byte buffer from a sundown buffer
 */
static ByteBuffer ByteBufferFromSundown(const struct buf *text)
{
    if (!text || !text->data || !text->size)
        return ByteBuffer();
    
    return ByteBuffer(reinterpret_cast<char *>(text->data), text->size);
}

MarkdownParser::MarkdownParser()
: m_workingNode(NULL)
{
}

void MarkdownParser::parse(const ByteBuffer& source, ASTNode& ast)
{
    ast = ASTNode();
    m_workingNode = &ast;
    m_workingNode->type = RootASTNode;
    m_workingNode->sourceMap.push_back(BytesRange(0, source.length()));
    
    RenderCallbacks callbacks = renderCallbacks();

	::sd_markdown *sundown = ::sd_markdown_new(ParserExtensions, MaxNesting, &callbacks, renderCallbackData());
    ::buf *output = ::bufnew(OutputUnitSize);
    
    ::sd_markdown_render(output, reinterpret_cast<const uint8_t *>(source.c_str()), source.length(), sundown);
    
    ::bufrelease(output);
    ::sd_markdown_free(sundown);
    
    m_workingNode = NULL;
}

MarkdownParser::RenderCallbacks MarkdownParser::renderCallbacks()
{
    RenderCallbacks callbacks;
    ::memset(&callbacks, 0, sizeof(RenderCallbacks));
    
    callbacks.blockcode = &MarkdownParser::renderBlockCode;
    callbacks.blockquote = &MarkdownParser::renderQuote;
    callbacks.blockhtml = &MarkdownParser::renderHTML;
    callbacks.header = &MarkdownParser::renderHeader;
    callbacks.hrule = &MarkdownParser::renderHorizontalRule;
    callbacks.list = &MarkdownParser::renderList;
    callbacks.listitem = &MarkdownParser::renderListItem;
    callbacks.paragraph = &MarkdownParser::renderParagraph;
    
    callbacks.table = NULL;
    callbacks.table_row = NULL;
    callbacks.table_cell = NULL;
    
    callbacks.blockquote_begin = &MarkdownParser::beginQuote;
    callbacks.list_begin = &MarkdownParser::beginList;
    callbacks.listitem_begin = &MarkdownParser::beginListItem;
    
    callbacks.block_did_parse = &MarkdownParser::blockDidParse;
    
    return callbacks;
}

MarkdownParser::RenderCallbackData MarkdownParser::renderCallbackData()
{
    return this;
}

void MarkdownParser::renderHeader(struct buf *ob, const struct buf *text, int level, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderHeader(ByteBufferFromSundown(text), level);
}

void MarkdownParser::renderHeader(const ByteBuffer& text, int level)
{
    if (!m_workingNode)
        throw NO_WORKING_NODE_ERR;
    
    ASTNode node(HeaderASTNodeType, m_workingNode, text, level);
    m_workingNode->children().push_back(node);
}

void MarkdownParser::beginList(int flags, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->beginList(flags);
}

void MarkdownParser::beginList(int flags)
{
    if (!m_workingNode)
        throw NO_WORKING_NODE_ERR;
    
    ASTNode node(ListASTNodeType, m_workingNode, ByteBuffer(), flags);
    m_workingNode->children().push_back(node);
    
    // Push context
    m_workingNode = &m_workingNode->children().back();
}

void MarkdownParser::renderList(struct buf *ob, const struct buf *text, int flags, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderList(ByteBufferFromSundown(text), flags);
}

void MarkdownParser::renderList(const ByteBuffer& text, int flags)
{
    if (!m_workingNode)
        throw NO_WORKING_NODE_ERR;
    
    if (m_workingNode->type != ListASTNodeType)
        throw WORKING_NODE_MISMATCH_ERR;
    
    m_workingNode->text = text;
    m_workingNode->data = flags;
    
    // Pop context
    m_workingNode = &m_workingNode->parent();
}

void MarkdownParser::beginListItem(int flags, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->beginListItem(flags);
}

void MarkdownParser::beginListItem(int flags)
{
    if (!m_workingNode)
        throw NO_WORKING_NODE_ERR;
    
    ASTNode node(ListItemASTNodeType, m_workingNode, ByteBuffer(), flags);
    m_workingNode->children().push_back(node);
    
    // Push context
    m_workingNode = &m_workingNode->children().back();
}

void MarkdownParser::renderListItem(struct buf *ob, const struct buf *text, int flags, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderListItem(ByteBufferFromSundown(text), flags);
}

void MarkdownParser::renderListItem(const ByteBuffer& text, int flags)
{
    if (!m_workingNode)
        throw NO_WORKING_NODE_ERR;

    if (m_workingNode->type != ListItemASTNodeType)
        throw WORKING_NODE_MISMATCH_ERR;
    
    // No "inline" list items:
    // Instead of storing the text on the list item
    // create the artificial paragraph node to store the text.
    if (m_workingNode->children().empty() ||
        m_workingNode->children().front().type != ParagraphASTNodeType) {
        ASTNode textNode(ParagraphASTNodeType, m_workingNode, text);
        m_workingNode->children().push_front(textNode);
    }
    
    m_workingNode->data = flags;
    
    // Pop context
    m_workingNode = &m_workingNode->parent();
}

void MarkdownParser::renderBlockCode(struct buf *ob, const struct buf *text, const struct buf *lang, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderBlockCode(ByteBufferFromSundown(text), ByteBufferFromSundown(lang));
}

void MarkdownParser::renderBlockCode(const ByteBuffer& text, const ByteBuffer& language)
{
    if (!m_workingNode)
        throw NO_WORKING_NODE_ERR;
    
    ASTNode node(CodeASTNodeType, m_workingNode, text);
    m_workingNode->children().push_back(node);
}

void MarkdownParser::renderParagraph(struct buf *ob, const struct buf *text, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderParagraph(ByteBufferFromSundown(text));
}

void MarkdownParser::renderParagraph(const ByteBuffer& text)
{
    if (!m_workingNode)
        throw NO_WORKING_NODE_ERR;
    
    ASTNode node(ParagraphASTNodeType, m_workingNode, text);
    m_workingNode->children().push_back(node);
}

void MarkdownParser::renderHorizontalRule(struct buf *ob, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderHorizontalRule();
}

void MarkdownParser::renderHorizontalRule()
{
    if (!m_workingNode)
        throw NO_WORKING_NODE_ERR;
    
    ASTNode node(HRuleASTNodeType, m_workingNode, ByteBuffer(), ASTNode::Data());
    m_workingNode->children().push_back(node);
}

void MarkdownParser::renderHTML(struct buf *ob, const struct buf *text, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderHTML(ByteBufferFromSundown(text));
}

void MarkdownParser::renderHTML(const ByteBuffer& text)
{
    if (!m_workingNode)
        throw NO_WORKING_NODE_ERR;
    
    ASTNode node(HTMLASTNodeType, m_workingNode, text);
    m_workingNode->children().push_back(node);
}

void MarkdownParser::beginQuote(void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->beginQuote();
}

void MarkdownParser::beginQuote()
{
    // TODO:
    //m_renderStack.push_back(MarkdownBlock(QuoteBlockBeginType, SourceData(), 0));
}

void MarkdownParser::renderQuote(struct buf *ob, const struct buf *text, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderQuote(ByteBufferFromSundown(text));
}

void MarkdownParser::renderQuote(const ByteBuffer& text)
{
    // TODO:
    //m_renderStack.push_back(MarkdownBlock(QuoteBlockEndType, text, 0));
}

void MarkdownParser::blockDidParse(const src_map* map, const uint8_t *txt_data, size_t size, void *opaque)
{
    if (!opaque || !map)
        return;
    
    BytesRangeSet sourceMap;
    for (size_t i = 0; i < map->size; ++i) {
        BytesRange byteRange(((range *)map->item[i])->loc, ((range *)map->item[i])->len);
        sourceMap.push_back(byteRange);
    }
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->blockDidParse(sourceMap);
}

void MarkdownParser::blockDidParse(const BytesRangeSet& sourceMap)
{
    if (!m_workingNode)
        throw NO_WORKING_NODE_ERR;
    
    if (m_workingNode->children().empty())
        throw std::logic_error("no working node children");
    
    ASTNode &lastNode = m_workingNode->children().back();
    lastNode.sourceMap.append(sourceMap);
    
    // No "inline" list items:
    // Share the list item source map with its artifical node, if exists.
    if (lastNode.type == ListItemASTNodeType &&
        !lastNode.children().empty() &&
        lastNode.children().front().sourceMap.empty())
        lastNode.children().front().sourceMap.append(sourceMap);
}
