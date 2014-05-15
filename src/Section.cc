//
//  Section.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/14/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "Section.h"

using namespace snowcrash;

std::string snowcrash::SectionName(const SectionType& section) {

    switch (section) {
        case ModelSectionType:
        case ModelBodySectionType:
            return "model";
            
        case ObjectSectionType:
        case ObjectBodySectionType:
            return "object";
            
        case RequestSectionType:
        case RequestBodySectionType:
            return "request";
            
        case ResponseSectionType:
        case ResponseBodySectionType:
            return "response";
            
        case BodySectionType:
        case DanglingBodySectionType:
            return "message-body";
            
        case SchemaSectionType:
        case DanglingSchemaSectionType:
            return "message-schema";
            
        case HeadersSectionType:
            return "headers";
            
        default:
            return "section";
    }
}

bool snowcrash::RecognizeSection(const mdp::MarkdownNodeIterator& node){
    return false;
}
