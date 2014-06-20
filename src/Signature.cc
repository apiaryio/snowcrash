//
//  Signature.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 6/20/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "Signature.h"
#include "SectionParser.h"
#include "AssetParser.h"
#include "HeadersParser.h"
#include "PayloadParser.h"
#include "ParameterParser.h"
#include "ParametersParser.h"

using namespace snowcrash;

bool snowcrash::HasSectionKeywordSignature(const mdp::MarkdownNodeIterator& node)
{
    // Note: Every-keyword defined section should be listed here...
    return (SectionProcessor<Headers>::sectionType(node) != UndefinedSectionType ||
            SectionProcessor<Asset>::sectionType(node) != UndefinedSectionType ||
            SectionProcessor<Payload>::sectionType(node) != UndefinedSectionType ||
            SectionProcessor<Values>::sectionType(node) != UndefinedSectionType ||
            SectionProcessor<Parameters>::sectionType(node) != UndefinedSectionType);
}
