//
//  Signature.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 6/20/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "Signature.h"
#include "SectionParser.h"
#include "ActionParser.h"
#include "AssetParser.h"
#include "HeadersParser.h"
#include "PayloadParser.h"
#include "ParametersParser.h"
#include "ResourceParser.h"
#include "ResourceGroupParser.h"
#include "MSONTypeSectionParser.h"
#include "DataStructureGroupParser.h"

using namespace snowcrash;

#define TYPECHECK(T)\
if ((type = SectionProcessor<T>::sectionType(node)) != UndefinedSectionType) {\
    return type;\
}

SectionType snowcrash::SectionKeywordSignature(const mdp::MarkdownNodeIterator& node)
{
    // Note: Every-keyword defined section should be listed here...
    SectionType type = UndefinedSectionType;

    TYPECHECK(mson::TypeSection)
    TYPECHECK(mson::Mixin)
    TYPECHECK(mson::OneOf)
    TYPECHECK(Headers)
    TYPECHECK(Asset)
    TYPECHECK(Attributes)
    TYPECHECK(Payload)
    TYPECHECK(Values)
    TYPECHECK(Parameters)
    TYPECHECK(Relation)

    /*
     *  NOTE: Order is important. Resource MUST preceed the Action.
     *
     *  This is because an HTTP Request Method + URI is recognized as both %ActionSectionType and %ResourceSectionType.
     *  This is not optimal and should be addressed in the future.
     */
    TYPECHECK(Resource)
    TYPECHECK(Action)
    TYPECHECK(ResourceGroup)
    TYPECHECK(DataStructureGroup)

    return type;
}

SectionType snowcrash::RecognizeCodeBlockFirstLine(const mdp::ByteBuffer& subject)
{
    SectionType type = UndefinedSectionType;

    if (RegexMatch(subject, HeadersRegex)) {
        return HeadersSectionType;
    }
    else if (RegexMatch(subject, BodyRegex)) {
        return BodySectionType;
    }
    else if (RegexMatch(subject, SchemaRegex)) {
        return SchemaSectionType;
    }

    return type;
}

#undef TYPECHECK
