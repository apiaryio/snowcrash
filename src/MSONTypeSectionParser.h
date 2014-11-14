//
//  MSONTypeSectionParser.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/4/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONTYPESECTIONPARSER_H
#define SNOWCRASH_MSONTYPESECTIONPARSER_H

#include "SectionParser.h"
#include "MSONUtility.h"
#include "MSONMixinParser.h"

using namespace scpl;

namespace snowcrash {

    /** MSON Default Type Section matching regex */
    const char* const MSONDefaultTypeSectionRegex = "^[[:blank:]]*[Dd]efault[[:blank:]]*(:.*)?$";

    /** MSON Sample Type Section matching regex */
    const char* const MSONSampleTypeSectionRegex = "^[[:blank:]]*[Ss]ample[[:blank:]]*(:.*)?$";

    /** MSON Value Members Type Section matching regex */
    const char* const MSONValueMembersTypeSectionRegex = "^[[:blank:]]*([Ii]tems|[Mm]embers)[[:blank:]]*$";

    /** MSON Property Members Type Section matching regex */
    const char* const MSONPropertyMembersTypeSectionRegex = "^[[:blank:]]*([Pp]roperties)[[:blank:]]*$";

    /**
     * MSON Type Section Section Processor
     */
    template<>
    struct SectionProcessor<mson::TypeSection> : public SignatureSectionProcessorBase<mson::TypeSection> {

        static SignatureTraits signatureTraits() {

            SignatureTraits signatureTraits(SignatureTraits::IdentifierTrait |
                                            SignatureTraits::ValuesTrait);

            return signatureTraits;
        }

        static void finalizeSignature(const MarkdownNodeIterator& node,
                                      SectionParserData& pd,
                                      const Signature& signature,
                                      const ParseResultRef<mson::TypeSection>& out) {

            bool assignValues = false;

            if (IEqual<std::string>()(signature.identifier, "Default")) {

                out.node.type = mson::DefaultTypeSectionType;
                assignValues = true;
            }
            else if (IEqual<std::string>()(signature.identifier, "Sample")) {

                out.node.type = mson::SampleTypeSectionType;
                assignValues = true;
            }
            else if (IEqual<std::string>()(signature.identifier, "Items") ||
                     IEqual<std::string>()(signature.identifier, "Members") ||
                     IEqual<std::string>()(signature.identifier, "Properties")) {

                out.node.type = mson::MemberTypeSectionType;
            }

            if (assignValues &&
                (!signature.values.empty() || !signature.value.empty())) {

                // TODO: Build values for sample/default type sections
            }
        }

        NO_DESCRIPTION(mson::TypeSection)

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator&,
                                                         const MarkdownNodes&,
                                                         SectionParserData&,
                                                         const ParseResultRef<mson::TypeSection>&);

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            mdp::ByteBuffer subject, remaining;

            if (node->type == mdp::HeaderMarkdownNodeType &&
                !node->text.empty()) {

                subject = node->text;
            }
            else if (node->type == mdp::ListItemMarkdownNodeType &&
                     !node->children().empty()) {

                subject = node->children().front().text;
            }

            subject = GetFirstLine(subject, remaining);
            TrimString(subject);

            if (RegexMatch(subject, MSONDefaultTypeSectionRegex) ||
                RegexMatch(subject, MSONSampleTypeSectionRegex)) {

                return MSONTypeSectionSectionType;
            }

            if (RegexMatch(subject, MSONValueMembersTypeSectionRegex)) {
                return MSONValueMembersSectionType;
            }

            if (RegexMatch(subject, MSONPropertyMembersTypeSectionRegex)) {
                return MSONPropertyMembersSectionType;
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator&);
    };

    /** MSON Type Section (Header) Section Parser */
    typedef SectionParser<mson::TypeSection, HeaderSectionAdapter> MSONTypeSectionHeaderParser;

    /** MSON Type Section (List) Section Parser */
    typedef SectionParser<mson::TypeSection, ListSectionAdapter> MSONTypeSectionListParser;
}

#endif
