//
//  MarkdownParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/15/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include <cstring>
#include "MarkdownParser.h"

using namespace snowcrash;

// Returns text from sundown buffer
static std::string BufText(const struct buf *text)
{
    if (!text || !text->data || !text->size)
        return std::string();
    
    return std::string(reinterpret_cast<char *>(text->data), text->size);
}

const size_t MarkdownParser::OutputUnitSize = 64;
const size_t MarkdownParser::MaxNesting = 16;
const int MarkdownParser::ParserExtensions = MKDEXT_FENCED_CODE | MKDEXT_NO_INTRA_EMPHASIS | MKDEXT_LAX_SPACING /*| MKDEXT_TABLES */;

void MarkdownParser::parse(const SourceData& source, Result& result, MarkdownBlock::Stack& markdown)
{
    // Push default render stack
    m_renderStack.clear();
    
    // Build render callbacks & setup parser
    RenderCallbacks callbacks = renderCallbacks();
	sd_markdown *sundown = sd_markdown_new(ParserExtensions, MaxNesting, &callbacks, renderCallbackData());
    
    buf *output = bufnew(OutputUnitSize);

    // Parse & build AST
    sd_markdown_render(output, reinterpret_cast<const uint8_t *>(source.c_str()), source.length(), sundown);

    bufrelease(output);
    sd_markdown_free(sundown);

    // Compose final Markdown object
    markdown = m_renderStack; // FIXME: C++11 move

#ifdef DEBUG
    printMarkdown(markdown, 0);
#endif

}

MarkdownParser::RenderCallbacks MarkdownParser::renderCallbacks()
{
    // Custom callbacks
    sd_callbacks callbacks;
    ::memset(&callbacks, 0, sizeof(sd_callbacks));

    callbacks.blockcode = &MarkdownParser::renderBlockCode;
    callbacks.blockquote = &MarkdownParser::renderQuote;
    callbacks.blockhtml = &MarkdownParser::renderHTML;
    callbacks.header = &MarkdownParser::renderHeader;
    callbacks.hrule = &MarkdownParser::renderHorizontalRule;
    callbacks.list = &MarkdownParser::renderList;
    callbacks.listitem = &MarkdownParser::renderListItem;
    callbacks.paragraph = &MarkdownParser::renderParagraph;
    
    // FUTURE: Table support
    callbacks.table = NULL;
    callbacks.table_row = NULL;
    callbacks.table_cell = NULL;
    
    // AST building extensions
    callbacks.blockquote_begin = &MarkdownParser::beginQuote;
    callbacks.list_begin = &MarkdownParser::beginList;
    callbacks.listitem_begin = &MarkdownParser::beginListItem;
    
    // Source maps
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
    p->renderHeader(BufText(text), level);
}

void MarkdownParser::renderHeader(const std::string& text, int level)
{
    m_renderStack.push_back(MarkdownBlock(HeaderBlockType, text, level));
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
    m_renderStack.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), flags));
}

void MarkdownParser::renderList(struct buf *ob, const struct buf *text, int flags, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderList(BufText(text), flags);
}

void MarkdownParser::renderList(const std::string& text, int flags)
{
    m_renderStack.push_back(MarkdownBlock(ListBlockEndType, text, flags));
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
    m_renderStack.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), flags));
}

void MarkdownParser::renderListItem(struct buf *ob, const struct buf *text, int flags, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderListItem(BufText(text), flags);
}

void MarkdownParser::renderListItem(const std::string& text, int flags)
{
    m_renderStack.push_back(MarkdownBlock(ListItemBlockEndType, text, flags));
}

void MarkdownParser::renderBlockCode(struct buf *ob, const struct buf *text, const struct buf *lang, void *opaque)
{    
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderBlockCode(BufText(text), BufText(lang));
}

void MarkdownParser::renderBlockCode(const std::string& text, const std::string& language)
{
    m_renderStack.push_back(MarkdownBlock(CodeBlockType, text));
}

void MarkdownParser::renderParagraph(struct buf *ob, const struct buf *text, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderParagraph(BufText(text));
}

void MarkdownParser::renderParagraph(const std::string& text)
{
    m_renderStack.push_back(MarkdownBlock(ParagraphBlockType, text));
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
    m_renderStack.push_back(MarkdownBlock(HRuleBlockType));
}

void MarkdownParser::renderHTML(struct buf *ob, const struct buf *text, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderHTML(BufText(text));
}

void MarkdownParser::renderHTML(const std::string& text)
{
    m_renderStack.push_back(MarkdownBlock(HTMLBlockType, text));
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
    m_renderStack.push_back(MarkdownBlock(QuoteBlockBeginType, SourceData(), 0));
}

void MarkdownParser::renderQuote(struct buf *ob, const struct buf *text, void *opaque)
{
    if (!opaque)
        return;
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->renderQuote(BufText(text));
}

void MarkdownParser::renderQuote(const std::string& text)
{
    m_renderStack.push_back(MarkdownBlock(QuoteBlockEndType, text, 0));
}

void MarkdownParser::blockDidParse(const src_map* map, const uint8_t *txt_data, size_t size, void *opaque)
{
    if (!opaque || !map)
        return;
    
    SourceDataBlock sourceMap;
    for (size_t i = 0; i < map->size; ++i) {
        SourceDataRange sourceRange = {((range *)map->item[i])->loc, ((range *)map->item[i])->len};
        sourceMap.push_back(sourceRange);
    }
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->blockDidParse(sourceMap);
}

void MarkdownParser::blockDidParse(const SourceDataBlock& sourceMap)
{
    if (m_renderStack.empty()) {
        return;
    }
    
    MarkdownBlock& lastBlock = m_renderStack.back();
    AppendSourceDataBlock(lastBlock.sourceMap, sourceMap);
}
