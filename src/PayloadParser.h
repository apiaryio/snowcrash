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

namespace snowcrash {
    
    enum PayloadSignature {
        UndefinedPayloadSignature,
        NoPayloadSignature,
        RequestPayloadSignature,
        ResponsePayloadSignature,
        GenericPayloadSignature
    };
    
    // Query payload signature of given block
    PayloadSignature HasPayloadSignature(const MarkdownBlock& block);
    
    // Parse Payload
    ParseSectionResult ParsePayload(const BlockIterator& begin, const BlockIterator& end, const SourceData& sourceData, Payload resource);
}

#endif
