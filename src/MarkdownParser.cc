//
//  MarkdownParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/15/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include "MarkdownParser.h"

using namespace snowcrash;

// Returns text from sundown buffer
static std::string BufText(const struct buf *text)
{
    if (!text || !text->data || !text->size)
        return std::string();
    
    return std::string(reinterpret_cast<char *>(text->data), text->size);
}

void MarkdownParser::parse(const SourceData& source, const MarkdownParser::ParseHandler& callback)
{
    // Push default render stack
    m_renderContext.clear();
    pushRenderContext();
    
    // Build render callbacks & setup parser
    RenderCallbacks callbacks = renderCallbacks();
	sd_markdown *markdown = sd_markdown_new(ParserExtensions, MaxNesting, &callbacks, renderCallbackData());
    
    buf *output = bufnew(OutputUnitSize);

    // Parse & build AST
    sd_markdown_render(output, reinterpret_cast<const uint8_t *>(source.c_str()), source.length(), markdown);

    bufrelease(output);
    sd_markdown_free(markdown);

    // Compose final Markdown object
    MarkdownBlock markdownAst;
    Result result;
    if (m_renderContext.size() == 1) {
        markdownAst.blocks = std::move(m_renderContext.back());
        m_renderContext.clear();
        
//#ifdef DEBUG
//        printMarkdownBlock(markdownAst, 0);
//#endif

    }
    else {
        result.error = { "mismatched markdown block element(s)", 1 };
    }
    
    if (callback)
        callback(result, markdownAst);
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
    
    // TODO: Table support
	// void (*table)(struct buf *ob, const struct buf *header, const struct buf *body, void *opaque);
    callbacks.table = NULL;
	//void (*table_row)(struct buf *ob, const struct buf *text, void *opaque);
    callbacks.table_row = NULL;
	// void (*table_cell)(struct buf *ob, const struct buf *text, int flags, void *opaque);
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

void MarkdownParser::pushRenderContext()
{
    m_renderContext.push_back(MarkdownBlock::Stack());
}

void MarkdownParser::popRenderContext()
{
    m_renderContext.pop_back();
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
    m_renderContext.back().emplace_back(MarkdownBlockType::Header, text, level);
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
    pushRenderContext();
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
    MarkdownBlock::Stack context = std::move(m_renderContext.back());
    
    popRenderContext();
    
    m_renderContext.back().emplace_back(MarkdownBlockType::List, text, 0);
    m_renderContext.back().back().blocks = std::move(context);
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
    pushRenderContext();
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
    MarkdownBlock::Stack context = std::move(m_renderContext.back());
    
    popRenderContext();
    
    m_renderContext.back().emplace_back(MarkdownBlockType::ListItem, text, 0);
    m_renderContext.back().back().blocks = std::move(context);
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
    m_renderContext.back().emplace_back(MarkdownBlockType::Code, text);
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
    m_renderContext.back().emplace_back(MarkdownBlockType::Paragraph, text);
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
    m_renderContext.back().emplace_back(MarkdownBlockType::HRule);
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
    m_renderContext.back().emplace_back(MarkdownBlockType::HTML, text);
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
    pushRenderContext();
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
    MarkdownBlock::Stack context = std::move(m_renderContext.back());
    
    popRenderContext();
    
    m_renderContext.back().emplace_back(MarkdownBlockType::Quote, text, 0);
    m_renderContext.back().back().blocks = std::move(context);
}

void MarkdownParser::blockDidParse(const src_map* map, const uint8_t *txt_data, size_t size, void *opaque)
{
    if (!opaque || !map)
        return;
    
    SourceDataBlock sourceMap;
    for (size_t i = 0; i < map->size; ++i) {
        SourceDataRange sourceRange = {((range *)map->item[i])->loc, ((range *)map->item[i])->len};
        sourceMap.emplace_back(sourceRange);
    }
    
    MarkdownParser *p = static_cast<MarkdownParser *>(opaque);
    p->blockDidParse(sourceMap);
}

void MarkdownParser::blockDidParse(const SourceDataBlock& sourceMap)
{
    if (m_renderContext.back().empty() ) {
        return;
    }
    
    MarkdownBlock& lastBlock = m_renderContext.back().back();
    AppendSourceDataBlock(lastBlock.sourceMap, sourceMap);
}
