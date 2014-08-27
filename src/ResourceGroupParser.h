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
    struct SectionProcessor<ResourceGroup, ResourceGroupSM> : public SectionProcessorBase<ResourceGroup, ResourceGroupSM> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
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

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         ResourceGroup& out,
                                                         ResourceGroupSM& outSM) {

            if (pd.sectionContext() == ResourceSectionType ||
                pd.sectionContext() == ResourceMethodSectionType) {

                Resource resource;
                ResourceSM resourceSM;

                MarkdownNodeIterator cur = ResourceParser::parse(node, siblings, pd, report, resource, resourceSM);

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

        static MarkdownNodeIterator processUnexpectedNode(const MarkdownNodeIterator& node,
                                                          const MarkdownNodes& siblings,
                                                          SectionParserData& pd,
                                                          SectionType& lastSectionType,
                                                          Report& report,
                                                          ResourceGroup& out,
                                                          ResourceGroupSM& outSM) {

            mdp::ByteBuffer method;

            if (isDependentAction(node, method) &&
                !out.resources.empty()) {

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
            nested.push_back(ResourceMethodSectionType);
            SectionTypes types = SectionProcessor<Resource, ResourceSM>::nestedSectionTypes();
            nested.insert(nested.end(), types.begin(), types.end());

            return nested;
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            mdp::ByteBuffer method;

            if (isCompleteAction(node, method)) {
                return false;
            }

            return SectionProcessorBase<ResourceGroup, ResourceGroupSM>::isDescriptionNode(node, sectionType);
        }

        static bool isUnexpectedNode(const MarkdownNodeIterator& node,
                                     SectionType sectionType) {

            mdp::ByteBuffer method;

            if (isDependentAction(node, method)) {
                return true;
            }

            return SectionProcessorBase<ResourceGroup, ResourceGroupSM>::isUnexpectedNode(node, sectionType);
        }

        /**
         *  \brief Check if a node represents a complete action
         *
         *  \node   Node to check
         *  \method Output buffer to store the HTTP request method for the transition
         *  \return True if the node signatures a complete transition, false otherwise
         *
         *  A complete transtion (action) is a transtition defined
         *  as a combination of an HTTP request method and an URI.
         */
        static bool isCompleteAction(const MarkdownNodeIterator& node,
                                     mdp::ByteBuffer& method) {

            CaptureGroups captureGroups;
            mdp::ByteBuffer subject = node->text;

            TrimString(subject);

            if (RegexCapture(subject, ActionHeaderRegex, captureGroups, 3) && !captureGroups[2].empty()) {

                method = captureGroups[1];
                return true;
            }

            return false;
        }
        
        /**
         *  \brief Check if a node represents a dependent action
         *
         *  A dependent action is defined by an HTTP request method only and as 
         *  such it depends on its parent resource URI.
         */
        static bool isDependentAction(const MarkdownNodeIterator& node,
                                      mdp::ByteBuffer& method) {
            
            CaptureGroups captureGroups;
            mdp::ByteBuffer subject = node->text;
            
            TrimString(subject);
            
            if (RegexCapture(subject, ActionHeaderRegex, captureGroups, 3) && captureGroups[2].empty()) {
                
                method = captureGroups[1];
                return true;
            }
            
            if (RegexCapture(subject, NamedActionHeaderRegex, captureGroups, 3)) {
                
                method = captureGroups[2];
                return true;
            }
            
            return false;
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
    typedef SectionParser<ResourceGroup, ResourceGroupSM, HeaderSectionAdapter> ResourceGroupParser;
}

#endif
