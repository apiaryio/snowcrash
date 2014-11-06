//
//  MSONPropertyMemberParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONPROPERTYMEMBERPARSER_H
#define SNOWCRASH_MSONPROPERTYMEMBERPARSER_H

#include "SectionParser.h"
#include "MSONUtility.h"
#include "MSONValueMemberParser.h"

using namespace scpl;

namespace snowcrash {

    /**
     * MSON Property Member Section Processor
     */
    template<>
    struct SectionProcessor<mson::PropertyMember> : public SectionProcessor<mson::ValueMember> {

        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(SignatureTraits::IdentifierTrait |
                                            SignatureTraits::ValuesTrait |
                                            SignatureTraits::AttributesTrait |
                                            SignatureTraits::ContentTrait);

            return signatureTraits;
        }

        static void finalizeSignature(const MarkdownNodeIterator& node,
                                      SectionParserData& pd,
                                      const Signature& signature,
                                      const ParseResultRef<mson::PropertyMember>& out) {

        }
    };
    
    /** MSON Property Member Section Parser */
    typedef SectionParser<mson::PropertyMember, ListSectionAdapter> MSONPropertyMemberParser;
}

#endif
