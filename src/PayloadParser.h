//
//  PayloadParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/7/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#ifndef SNOWCRASH_PARSEPAYLOAD_H
#define SNOWCRASH_PARSEPAYLOAD_H

#include "BlueprintParserCore.h"
#include "Blueprint.h"

// Request matching regex
static const std::string RequestRegex("^[Rr]equest([[:space:]]+([A-Za-z0-9_]|[[:space:]])*)?([[:space:]]\\([^\\)]*\\))?[[:space:]]*$");

// Response matching regex
static const std::string ResponseRegex("^[Rr]esponse([[:space:]]+([0-9_])*)?([[:space:]]\\([^\\)]*\\))?[[:space:]]*$");

namespace snowcrash {
    
    // Payload signature
    enum PayloadSignature {
        UndefinedPayloadSignature,
        NoPayloadSignature,
        RequestPayloadSignature,
        ResponsePayloadSignature,
        GenericPayloadSignature
    };
    
    // Query payload signature of given block
    PayloadSignature HasPayloadSignature(const MarkdownBlock& block) {

        // TODO:
        //    if (block.type != ListBlockType ||
        //        block.blocks.empty() ||
        //        block.blocks.front().type != ListItemBlockType)
        //        return NoPayloadSignature;
        //
        //    MarkdownBlock::Stack::const_iterator item = block.blocks.begin();
        //
        //    // In-place list (without child blocks)
        //    if (!item->content.empty() || item->blocks.empty())
        //        return NoPayloadSignature;
        //
        //    // Sanity check, this should not happen
        //    if (item->blocks.front().type != ParagraphBlockType)
        //        return NoPayloadSignature;
        //
        //    if (RegexMatch(item->blocks.front().content, RequestRegex))
        //        return RequestPayloadSignature;
        //
        //    if (RegexMatch(item->blocks.front().content, ResponseRegex))
        //        return ResponsePayloadSignature;
        
        return NoPayloadSignature;
    }
    
    //
    // Block Classifier, Payload Context
    //
    template <>
    inline Section TClassifyBlock<Payload>(const MarkdownBlock& block, const Section& context) {
        
        // TODO:
        return UndefinedSection;
    }
    
    //
    // Payload Section Parser
    //
    template<>
    struct SectionParser<Payload> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& begin,
                                               const BlockIterator& end,
                                               const SourceData& sourceData,
                                               const Blueprint& blueprint,
                                               Payload& payload) {
            
            ParseSectionResult result = std::make_pair(Result(), begin);
            // TODO:
            switch (section) {
                case UndefinedSection:
                    break;
                    
                default:
                    result.first.error = Error("unexpected block", 1, begin->sourceMap);
                    break;
            }
            
            return result;
        }
    };
    
    typedef BlockParser<Payload, SectionParser<Payload> > PayloadParser;    
}

#endif
