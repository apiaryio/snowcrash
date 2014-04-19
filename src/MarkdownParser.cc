//
//  MarkdownParser.cc
//  markdownparser
//
//  Created by Zdenek Nemec on 4/18/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include <cstring>
#include "MarkdownParser.h"

using namespace mdp;

const size_t MarkdownParser::OutputUnitSize = 64;
const size_t MarkdownParser::MaxNesting = 16;
const int MarkdownParser::ParserExtensions = MKDEXT_FENCED_CODE | MKDEXT_NO_INTRA_EMPHASIS | MKDEXT_LAX_SPACING /*| MKDEXT_TABLES */;

/**
 *  \brief  Create a byte buffer from a sundown buffer
 */
static ByteBuffer ByteBufferFromSunndown(const struct buf *text)
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
    p->renderHeader(ByteBufferFromSunndown(text), level);
}

void MarkdownParser::renderHeader(const ByteBuffer& text, int level)
{
    //m_renderStack.push_back(MarkdownBlock(HeaderBlockType, text, level));
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
    //m_renderStack.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), flags));
}

void MarkdownParser::renderList(struct buf *ob, const struct buf *text, int flags, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderList(ByteBufferFromSunndown(text), flags);
}

void MarkdownParser::renderList(const ByteBuffer& text, int flags)
{
   //m_renderStack.push_back(MarkdownBlock(ListBlockEndType, text, flags));
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
    //m_renderStack.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), flags));
}

void MarkdownParser::renderListItem(struct buf *ob, const struct buf *text, int flags, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderListItem(ByteBufferFromSunndown(text), flags);
}

void MarkdownParser::renderListItem(const ByteBuffer& text, int flags)
{
    //m_renderStack.push_back(MarkdownBlock(ListItemBlockEndType, text, flags));
}

void MarkdownParser::renderBlockCode(struct buf *ob, const struct buf *text, const struct buf *lang, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderBlockCode(ByteBufferFromSunndown(text), ByteBufferFromSunndown(lang));
}

void MarkdownParser::renderBlockCode(const ByteBuffer& text, const ByteBuffer& language)
{
    //m_renderStack.push_back(MarkdownBlock(CodeBlockType, text));
}

void MarkdownParser::renderParagraph(struct buf *ob, const struct buf *text, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderParagraph(ByteBufferFromSunndown(text));
}

void MarkdownParser::renderParagraph(const ByteBuffer& text)
{
    ASTNode node(ParagraphASTNodeType, m_workingNode, text);
    m_workingNode->children.push_back(node);
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
    //m_renderStack.push_back(MarkdownBlock(HRuleBlockType));
}

void MarkdownParser::renderHTML(struct buf *ob, const struct buf *text, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderHTML(ByteBufferFromSunndown(text));
}

void MarkdownParser::renderHTML(const ByteBuffer& text)
{
    //m_renderStack.push_back(MarkdownBlock(HTMLBlockType, text));
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
    //m_renderStack.push_back(MarkdownBlock(QuoteBlockBeginType, SourceData(), 0));
}

void MarkdownParser::renderQuote(struct buf *ob, const struct buf *text, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderQuote(ByteBufferFromSunndown(text));
}

void MarkdownParser::renderQuote(const ByteBuffer& text)
{
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
    ASTNode &lastNode = m_workingNode->children.back();
    lastNode.sourceMap.append(sourceMap);
    
//    MarkdownBlock& lastBlock = m_renderStack.back();
//    AppendSourceDataBlock(lastBlock.sourceMap, sourceMap);
}
