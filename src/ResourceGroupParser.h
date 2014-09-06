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

    /** Internal type alias for Collection iterator of Resource */
    typedef Collection<ResourceGroup>::const_iterator ResourceGroupIterator;

    /** Resource iterator pair: its containment group and resource iterator itself */
    typedef std::pair<ResourceGroupIterator, ResourceIterator> ResourceIteratorPair;

    /**
     * ResourceGroup Section processor
     */
    template<>
    struct SectionProcessor<ResourceGroup, ResourceGroupSM> : public SectionProcessorBase<ResourceGroup, ResourceGroupSM> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     Report& report,
                                                     ResourceGroup& out,
                                                     ResourceGroupSM& outSM) {

            MarkdownNodeIterator cur = node;
            SectionType nestedType = nestedSectionType(cur);

            // Resources only, parse as exclusive nested sections
            if (nestedType != UndefinedSectionType) {
                layout = ExclusiveNestedSectionLayout;
                return cur;
            }

            CaptureGroups captureGroups;

            if (RegexCapture(node->text, GroupHeaderRegex, captureGroups, 3)) {
                out.name = captureGroups[1];
                TrimString(out.name);
            }

            if (pd.exportSM() && !out.name.empty()) {
                outSM.name = node->sourceMap;
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         ResourceGroup& out,
                                                         ResourceGroupSM& outSM) {

            if (pd.sectionContext() == ResourceSectionType) {

                Resource resource;
                ResourceSM resourceSM;

                MarkdownNodeIterator cur = ResourceParser::parse(node, siblings, pd, report, resource, resourceSM);

                ResourceIterator duplicate = SectionProcessor<Resource, ResourceSM>::findResource(out.resources, resource);
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

                if (pd.exportSM()) {
                    outSM.resources.push_back(resourceSM);
                }

                return cur;
            }

            return node;
        }

        static MarkdownNodeIterator processUnexpectedNode(const MarkdownNodeIterator& node,
                                                          const MarkdownNodes& siblings,
                                                          SectionParserData& pd,
                                                          SectionType& lastSectionType,
                                                          Report& report,
                                                          ResourceGroup& out,
                                                          ResourceGroupSM& outSM) {

            if (SectionProcessor<Action, ActionSM>::actionType(node) == DependentActionType &&
                !out.resources.empty()) {

                mdp::ByteBuffer method;
                mdp::ByteBuffer name;

                SectionProcessor<Action, ActionSM>::actionHTTPMethodAndName(node, method, name);
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);

                // WARN: Unexpected action
                std::stringstream ss;
                ss << "unexpected action '" << method << "', to define multiple actions for the '" << out.resources.back().uriTemplate;
                ss << "' resource omit the HTTP method in its definition, e.g. '# /resource'";

                report.warnings.push_back(Warning(ss.str(),
                                                  IgnoringWarning,
                                                  sourceMap));

                return ++MarkdownNodeIterator(node);
            }

            return SectionProcessorBase<ResourceGroup, ResourceGroupSM>::processUnexpectedNode(node, siblings, pd, lastSectionType, report, out, outSM);
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
            return SectionProcessor<Resource, ResourceSM>::sectionType(node);
        }

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested;

            // Resource & descendants
            nested.push_back(ResourceSectionType);
            SectionTypes types = SectionProcessor<Resource, ResourceSM>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            return nested;
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            mdp::ByteBuffer method;

            if (SectionProcessor<Action, ActionSM>::actionType(node) == CompleteActionType) {
                return false;
            }

            return SectionProcessorBase<ResourceGroup, ResourceGroupSM>::isDescriptionNode(node, sectionType);
        }

        static bool isUnexpectedNode(const MarkdownNodeIterator& node,
                                     SectionType sectionType) {

            if (SectionProcessor<Action, ActionSM>::actionType(node) == DependentActionType) {
                return true;
            }

            return SectionProcessorBase<ResourceGroup, ResourceGroupSM>::isUnexpectedNode(node, sectionType);
        }

        /** Finds a resource in blueprint by its URI template */
        static ResourceIteratorPair findResource(const Blueprint& blueprint,
                                                 const Resource& resource) {

            for (ResourceGroupIterator it = blueprint.resourceGroups.begin();
                  it != blueprint.resourceGroups.end();
                  ++it) {

                ResourceIterator match = SectionProcessor<Resource, ResourceSM>::findResource(it->resources, resource);

                if (match != it->resources.end()) {
                    return std::make_pair(it, match);
                }
             }

             return std::make_pair(blueprint.resourceGroups.end(), ResourceIterator());
        }
    };

    /** ResourceGroup Section Parser */
    typedef SectionParser<ResourceGroup, ResourceGroupSM, HeaderSectionAdapter> ResourceGroupParser;
}

#endif
