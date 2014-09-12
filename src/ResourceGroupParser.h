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
    struct SectionProcessor<ResourceGroup> : public SectionProcessorBase<ResourceGroup> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     const MarkdownNodes& siblings,
                                                     SectionParserData& pd,
                                                     SectionLayout& layout,
                                                     ParseResult<ResourceGroup>& out) {

            MarkdownNodeIterator cur = node;
            SectionType nestedType = nestedSectionType(cur);

            // Resources only, parse as exclusive nested sections
            if (nestedType != UndefinedSectionType) {
                layout = ExclusiveNestedSectionLayout;
                return cur;
            }

            CaptureGroups captureGroups;

            if (RegexCapture(node->text, GroupHeaderRegex, captureGroups, 3)) {
                out.node.name = captureGroups[1];
                TrimString(out.node.name);
            }

            if (pd.exportSM() && !out.node.name.empty()) {
                out.sourceMap.name.sourceMap = node->sourceMap;
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         ParseResult<ResourceGroup>& out) {

            if (pd.sectionContext() == ResourceSectionType) {

                ParseResult<Resource> resource;
                MarkdownNodeIterator cur = ResourceParser::parse(node, siblings, pd, resource);

                out.report += resource.report;

                ResourceIterator duplicate = SectionProcessor<Resource>::findResource(out.node.resources, resource.node);
                ResourceIteratorPair globalDuplicate;

                if (duplicate == out.node.resources.end()) {
                    globalDuplicate = findResource(pd.blueprint, resource.node);
                }

                if (duplicate != out.node.resources.end() ||
                    globalDuplicate.first != pd.blueprint.resourceGroups.end()) {

                    // WARN: Duplicate resource
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.warnings.push_back(Warning("the resource '" + resource.node.uriTemplate + "' is already defined",
                                                          DuplicateWarning,
                                                          sourceMap));
                }

                out.node.resources.push_back(resource.node);

                if (pd.exportSM()) {
                    out.sourceMap.resources.collection.push_back(resource.sourceMap);
                }

                return cur;
            }

            return node;
        }

        static MarkdownNodeIterator processUnexpectedNode(const MarkdownNodeIterator& node,
                                                          const MarkdownNodes& siblings,
                                                          SectionParserData& pd,
                                                          SectionType& lastSectionType,
                                                          ParseResult<ResourceGroup>& out) {

            if (SectionProcessor<Action>::actionType(node) == DependentActionType &&
                !out.node.resources.empty()) {

                mdp::ByteBuffer method;
                mdp::ByteBuffer name;

                SectionProcessor<Action>::actionHTTPMethodAndName(node, method, name);
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);

                // WARN: Unexpected action
                std::stringstream ss;
                ss << "unexpected action '" << method << "', to define multiple actions for the '" << out.node.resources.back().uriTemplate;
                ss << "' resource omit the HTTP method in its definition, e.g. '# /resource'";

                out.report.warnings.push_back(Warning(ss.str(),
                                                      IgnoringWarning,
                                                      sourceMap));

                return ++MarkdownNodeIterator(node);
            }

            return SectionProcessorBase<ResourceGroup>::processUnexpectedNode(node, siblings, pd, lastSectionType, out);
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

        static SectionTypes nestedSectionTypes() {
            SectionTypes nested;

            // Resource & descendants
            nested.push_back(ResourceSectionType);
            SectionTypes types = SectionProcessor<Resource>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            return nested;
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            mdp::ByteBuffer method;

            if (SectionProcessor<Action>::actionType(node) == CompleteActionType) {
                return false;
            }

            return SectionProcessorBase<ResourceGroup>::isDescriptionNode(node, sectionType);
        }

        static bool isUnexpectedNode(const MarkdownNodeIterator& node,
                                     SectionType sectionType) {

            if (SectionProcessor<Action>::actionType(node) == DependentActionType) {
                return true;
            }

            return SectionProcessorBase<ResourceGroup>::isUnexpectedNode(node, sectionType);
        }

        /** Finds a resource in blueprint by its URI template */
        static ResourceIteratorPair findResource(const Blueprint& blueprint,
                                                 const Resource& resource) {

            for (ResourceGroupIterator it = blueprint.resourceGroups.begin();
                  it != blueprint.resourceGroups.end();
                  ++it) {

                ResourceIterator match = SectionProcessor<Resource>::findResource(it->resources, resource);

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
