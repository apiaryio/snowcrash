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

    /** Resource iterator pair: its containment group and resource iterator itself */
    typedef std::pair<Collection<ResourceGroup>::const_iterator, ResourceIterator> ResourceIteratorPair;

    /** Internal type alias for Collection of Resource */
    typedef Collection<ResourceGroup>::type ResourceGroups;

    typedef Collection<ResourceGroup>::const_iterator ResourceGroupIterator;

    /**
     * ResourceGroup Section processor
     */
    template<>
    struct SectionProcessor<ResourceGroup> : public SectionProcessorBase<ResourceGroup> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     Report& report,
                                                     ResourceGroup& out) {

            MarkdownNodeIterator cur = node;
            SectionType nestedType = nestedSectionType(cur);

            // If starting with Resource directly
            if (nestedType != UndefinedSectionType) {

                pd.sectionsContext.push_back(nestedType);
                cur = processNestedSection(cur, cur->parent().children(), pd, report, out);
                pd.sectionsContext.pop_back();

                return cur;
            }

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

            if (pd.sectionContext() == ResourceSectionType ||
                pd.sectionContext() == ResourceMethodSectionType) {

                Resource resource;
                MarkdownNodeIterator cur = ResourceParser::parse(node, siblings, pd, report, resource);

                ResourceIterator duplicate = findResource(out.resources, resource);
                ResourceIteratorPair globalDuplicate;

                if (duplicate == out.resources.end()) {
                    globalDuplicate = findResource(pd.blueprint, resource);
                }

                if (duplicate != out.resources.end() ||
                    globalDuplicate.first != pd.blueprint.resourceGroups.end()) {

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
        static ResourceIterator findResource(const Resources& resources,
                                             const Resource& resource) {

            return std::find_if(resources.begin(),
                                resources.end(),
                                std::bind2nd(MatchResource(), resource));
        }

        /** Finds a resource in blueprint by its URI template */
        static ResourceIteratorPair findResource(const Blueprint& blueprint,
                                                 const Resource& resource) {

            for (Collection<ResourceGroup>::const_iterator it = blueprint.resourceGroups.begin();
                  it != blueprint.resourceGroups.end();
                  ++it) {

                ResourceIterator match = findResource(it->resources, resource);

                if (match != it->resources.end()) {
                    return std::make_pair(it, match);
                }
             }

             return std::make_pair(blueprint.resourceGroups.end(), ResourceIterator());
        }
    };

    /** ResourceGroup Section Parser */
    typedef SectionParser<ResourceGroup, HeaderSectionAdapter> ResourceGroupParser;
}

#endif
