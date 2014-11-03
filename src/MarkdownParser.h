//
//  MarkdownParser.h
//  markdownparser
//
//  Created by Zdenek Nemec on 4/18/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef MARKDOWNPARSER_MARKDOWNPARSER_H
#define MARKDOWNPARSER_MARKDOWNPARSER_H

#include "ByteBuffer.h"
#include "MarkdownNode.h"
#include "markdown.h"

namespace mdp {

    const char* const MarkdownEmphasisChars = "*_";
    const char* const MarkdownListItemChars = "+-*";

    const char MarkdownBeginReference = '[';
    const char MarkdownEndReference = ']';

    const char* const MarkdownLinkRegex = "^[[:blank:]]*\\[(.*)][[:blank:]]*(\\[([^][()]*)]|\\(([^][()]+)\\))[[:blank:]]*$";

    /**
     *  GitHub-flavored Markdown Parser
     */
    class MarkdownParser {
    public:
        MarkdownParser();
        MarkdownParser(const MarkdownParser&);
        MarkdownParser& operator=(const MarkdownParser&);
        
        /**
         *  \brief Parse source buffer
         *
         *  \param source   Markdown source data to be parsed
         *  \param ast      Parsed AST (root node)
         */
        void parse(const ByteBuffer& source, MarkdownNode& ast);
        
    private:
        MarkdownNode* m_workingNode;
        bool m_listBlockContext;
        const ByteBuffer* m_source;
        size_t m_sourceLength;
        
        static const size_t OutputUnitSize;
        static const size_t MaxNesting;
        static const int ParserExtensions;

        typedef sd_callbacks RenderCallbacks;
        RenderCallbacks renderCallbacks();
        
        typedef void * RenderCallbackData;
        RenderCallbackData renderCallbackData();
        
        // Header
        static void renderHeader(struct buf *ob, const struct buf *text, int level, void *opaque);
        void renderHeader(const ByteBuffer& text, int level);
        
        // List
        static void beginList(int flags, void *opaque);
        void beginList(int flags);
        
        static void renderList(struct buf *ob, const struct buf *text, int flags, void *opaque);
        void renderList(const ByteBuffer& text, int flags);
        
        // List item
        static void beginListItem(int flags, void *opaque);
        void beginListItem(int flags);
        
        static void renderListItem(struct buf *ob, const struct buf *text, int flags, void *opaque);
        void renderListItem(const ByteBuffer& text, int flags);
        
        // Code block
        static void renderBlockCode(struct buf *ob, const struct buf *text, const struct buf *lang, void *opaque);
        void renderBlockCode(const ByteBuffer& text, const ByteBuffer& language);
        
        // Paragraph
        static void renderParagraph(struct buf *ob, const struct buf *text, void *opaque);
        void renderParagraph(const ByteBuffer& text);
        
        // Horizontal Rule
        static void renderHorizontalRule(struct buf *ob, void *opaque);
        void renderHorizontalRule();
        
        // HTML
        static void renderHTML(struct buf *ob, const struct buf *text, void *opaque);
        void renderHTML(const ByteBuffer& text);
        
        // Quote
        static void beginQuote(void *opaque);
        void beginQuote();
        
        static void renderQuote(struct buf *ob, const struct buf *text, void *opaque);
        void renderQuote(const std::string& text);
        
        // Source maps
        static void blockDidParse(const src_map* map, const uint8_t *txt_data, size_t size, void *opaque);
        void blockDidParse(const BytesRangeSet& sourceMap);
    };
}

#endif
