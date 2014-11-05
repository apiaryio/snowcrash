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

using namespace scpl;

namespace snowcrash {

    /** Default Type Section matching regex */
    const char* const DefaultTypeSectionRegex = "^[[:blank:]]*[Dd]efault[[:blank:]]*(:.*)?$";

    /** Sample Type Section matching regex */
    const char* const SampleTypeSectionRegex = "^[[:blank:]]*[Ss]ample[[:blank:]]*(:.*)?$";

    /** Member group Type Section matching regex */
    const char* const MemberTypeSectionRegex = "^[[:blank:]]*([Ii]tems|[Mm]embers|[Pp]roperties)[[:blank:]]*$";

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

            if (assignValues && !signature.values.empty()) {

                // TODO:
            }
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       const ParseResultRef<mson::TypeSection>& out) {

            return node;
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {
            return false;
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            mdp::ByteBuffer subject;

            if (node->type == mdp::HeaderMarkdownNodeType &&
                !node->text.empty()) {

                subject = node->text;
            }
            else if (node->type == mdp::ListItemMarkdownNodeType &&
                     !node->children().empty()) {

                subject = node->children().front().text;
            }

            TrimString(subject);

            if (RegexMatch(subject, DefaultTypeSectionRegex) ||
                RegexMatch(subject, SampleTypeSectionRegex) ||
                RegexMatch(subject, MemberTypeSectionRegex)) {

                return MSONTypeSectionSectionType;
            }

            return UndefinedSectionType;
        }
    };

    /** MSON Type Section (Header) Section Parser */
    typedef SectionParser<mson::TypeSection, HeaderSectionAdapter> MSONTypeSectionHeaderParser;

    /** MSON Type Section (List) Section Parser */
    typedef SectionParser<mson::TypeSection, ListSectionAdapter> MSONTypeSectionListParser;
}

#endif
