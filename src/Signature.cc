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

SectionType snowcrash::SectionKeywordSignature(const mdp::MarkdownNodeIterator& node)
{
    // Note: Every-keyword defined section should be listed here...
    SectionType type = UndefinedSectionType;

    if ((type = SectionProcessor<mson::TypeSection>::sectionType(node)) != UndefinedSectionType)
        return type;

    if ((type = SectionProcessor<Headers>::sectionType(node)) != UndefinedSectionType)
        return type;

    if ((type = SectionProcessor<Asset>::sectionType(node)) != UndefinedSectionType)
        return type;

    if ((type = SectionProcessor<Attributes>::sectionType(node)) != UndefinedSectionType)
        return type;

    if ((type = SectionProcessor<Payload>::sectionType(node)) != UndefinedSectionType)
        return type;

    if ((type = SectionProcessor<Values>::sectionType(node)) != UndefinedSectionType)
        return type;

    if ((type = SectionProcessor<Parameters>::sectionType(node)) != UndefinedSectionType)
        return type;

    if ((type = SectionProcessor<Relation>::sectionType(node)) != UndefinedSectionType)
        return type;

    /*
     *  NOTE: Order is important. Resource MUST preceed the Action.
     *
     *  This is because an HTTP Request Method + URI is recognized as both %ActionSectionType and %ResourceSectionType.
     *  This is not optimal and should be addressed in the future.
     */
    if ((type = SectionProcessor<Resource>::sectionType(node)) != UndefinedSectionType)
        return type;

    if ((type = SectionProcessor<Action>::sectionType(node)) != UndefinedSectionType)
        return type;

    if ((type = SectionProcessor<ResourceGroup>::sectionType(node)) != UndefinedSectionType)
        return type;

    if ((type = SectionProcessor<DataStructureGroup>::sectionType(node)) != UndefinedSectionType)
        return type;

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
