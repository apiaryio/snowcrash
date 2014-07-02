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

            CaptureGroups captureGroups;

            if (RegexCapture(node->text, GroupHeaderRegex, captureGroups, 3)) {
                out.name = captureGroups[1];
                TrimString(out.name);
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         ResourceGroup& out) {

            if (pd.sectionContext() == ResourceSectionType) {

                Resource resource;
                MarkdownNodeIterator cur = ResourceParser::parse(node, siblings, pd, report, resource);

                ResourceIterator duplicate = findResource(out.resources, resource);

                // TODO: Global duplicate?
                // TODO: Some URIParser logic

                if (duplicate != out.resources.end()) {

                    // WARN: Duplicate resource
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(Warning("the resource '" + resource.uriTemplate + "' is already defined",
                                                      DuplicateWarning,
                                                      sourceMap));
                }

                out.resources.push_back(resource);

                return cur;
            }

            return node;
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {

            if (node->type == mdp::HeaderMarkdownNodeType
                && !node->text.empty()) {

                mdp::ByteBuffer subject = node->text;
                TrimString(subject);

                if (RegexMatch(subject, GroupHeaderRegex)) {
                    return ResourceGroupSectionType;
                }
            }

            return UndefinedSectionType;
        }

        static SectionType nestedSectionType(const MarkdownNodeIterator& node) {

            // Return ResourceSectionType or UndefinedSectionType
            return SectionProcessor<Resource>::sectionType(node);
        }

        /** Finds a resource inside an resources collection */
        static ResourceIterator findResource(Resources& resources,
                                             const Resource& resource) {

            return std::find_if(resources.begin(),
                                resources.end(),
                                std::bind2nd(MatchResource(), resource));
        }

    };

    /** ResourceGroup Section Parser */
    typedef SectionParser<ResourceGroup, HeaderSectionAdapter> ResourceGroupParser;
}

#endif
