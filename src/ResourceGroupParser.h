//
//  ResourceGroupParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_RESOURCEGROUPPARSER_H
#define SNOWCRASH_RESOURCEGROUPPARSER_H

#include "SectionParser.h"
#include "ResourceParser.h"
#include "RegexMatch.h"

namespace snowcrash {

    const char* const GroupHeaderRegex = "^[[:blank:]]*[Gg]roup[[:blank:]]+" SYMBOL_IDENTIFIER "[[:blank:]]*$";

    /**
     * ResourceGroup Section processor
     */
    template<>
    struct SectionProcessor<ResourceGroup> : public SectionProcessorBase<ResourceGroup> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     Report& report,
                                                     ResourceGroup& out) {

            return ++MarkdownNodeIterator(node);
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            return UndefinedSectionType;
        }
    };

    /** ResourceGroup Section Parser */
    typedef SectionParser<ResourceGroup, HeaderSectionAdapter> ResourceGroupParser;
}

#endif
