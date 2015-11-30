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
                                                     const ParseResultRef<ResourceGroup>& out) {

            MarkdownNodeIterator cur = node;
            SectionType nestedType = nestedSectionType(cur);

            // Resources only, parse as exclusive nested sections
            if (nestedType != UndefinedSectionType) {
                layout = ExclusiveNestedSectionLayout;
                return cur;
            }

            CaptureGroups captureGroups;

            if (RegexCapture(node->text, GroupHeaderRegex, captureGroups, 3)) {
                out.node.attributes.name = captureGroups[1];
                TrimString(out.node.attributes.name);
            }

            if (pd.exportSourceMap() && !out.node.attributes.name.empty()) {
                out.sourceMap.attributes.name.sourceMap = node->sourceMap;
            }

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       const MarkdownNodes& siblings,
                                                       SectionParserData& pd,
                                                       const ParseResultRef<ResourceGroup>& out) {

            // Check for a description child element
            if (out.node.content.elements().empty() ||
                (!out.node.content.elements().empty() &&
                 out.node.content.elements().back().element != Element::CopyElement)) {

                Element description(Element::CopyElement);
                out.node.content.elements().push_back(description);

                if (pd.exportSourceMap()) {

                    SourceMap<Element> descriptionSM(Element::CopyElement);
                    out.sourceMap.content.elements().collection.push_back(descriptionSM);
                }
            }

            if (!out.node.content.elements().back().content.copy.empty()) {
                TwoNewLines(out.node.content.elements().back().content.copy);
            }

            mdp::ByteBuffer content = mdp::MapBytesRangeSet(node->sourceMap, pd.sourceData);

            if (pd.exportSourceMap() && !content.empty()) {
                out.sourceMap.content.elements().collection.back().content.copy.sourceMap.append(node->sourceMap);
            }

            out.node.content.elements().back().content.copy += content;

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         const ParseResultRef<ResourceGroup>& out) {

            MarkdownNodeIterator cur = node;

            if (pd.sectionContext() == ResourceSectionType) {

                IntermediateParseResult<Resource> resource(out.report);
                cur = ResourceParser::parse(node, siblings, pd, resource);

                bool duplicate = SectionProcessor<Resource>::isResourceDuplicate(out.node.content.elements(), resource.node.uriTemplate);
                bool globalDuplicate;

                if (!duplicate) {
                    globalDuplicate = isResourceDuplicate(pd.blueprint, resource.node.uriTemplate);
                }

                if (duplicate || globalDuplicate) {

                    // WARN: Duplicate resource
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                    out.report.warnings.push_back(Warning("the resource '" + resource.node.uriTemplate + "' is already defined",
                                                          DuplicateWarning,
                                                          sourceMap));
                }

                Element resourceElement(Element::ResourceElement);
                resourceElement.content.resource = resource.node;

                out.node.content.elements().push_back(resourceElement);

                if (pd.exportSourceMap()) {

                    SourceMap<Element> resourceElementSM(Element::ResourceElement);
                    resourceElementSM.content.resource = resource.sourceMap;

                    out.sourceMap.content.elements().collection.push_back(resourceElementSM);
                }
            }

            return cur;
        }

        static MarkdownNodeIterator processUnexpectedNode(const MarkdownNodeIterator& node,
                                                          const MarkdownNodes& siblings,
                                                          SectionParserData& pd,
                                                          SectionType& lastSectionType,
                                                          const ParseResultRef<ResourceGroup>& out) {

            if (SectionProcessor<Action>::actionType(node) == DependentActionType &&
                isResourcePresent(out.node.content.elements())) {

                mdp::ByteBuffer method, name, uriTemplate;

                SectionProcessor<Action>::actionHTTPMethodAndName(node, method, name, uriTemplate);
                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);

                // WARN: Unexpected action
                std::stringstream ss;
                ss << "unexpected action '" << method << "', to define multiple actions for the '" << lastResource(out.node.content.elements()).uriTemplate;
                ss << "' resource omit the HTTP method in its definition, e.g. '# /resource'";

                out.report.warnings.push_back(Warning(ss.str(),
                                                      IgnoringWarning,
                                                      sourceMap));

                return ++MarkdownNodeIterator(node);
            }

            return SectionProcessorBase<ResourceGroup>::processUnexpectedNode(node, siblings, pd, lastSectionType, out);
        }

        static void finalize(const MarkdownNodeIterator& node,
                             SectionParserData& pd,
                             const ParseResultRef<ResourceGroup>& out) {

            out.node.element = Element::CategoryElement;
            out.node.category = Element::ResourceGroupCategory;

            if (pd.exportSourceMap()) {

                out.sourceMap.element = out.node.element;
                out.sourceMap.category = out.node.category;
            }
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

        /**
         * \brief Given a blueprint, Check if a resource already exists with the given uri template
         *
         * \param blueprint The blueprint which is formed until now
         * \param uri The resource uri template to be checked
         */
        static bool isResourceDuplicate(const Blueprint& blueprint,
                                        const URITemplate& uri) {

            for (Elements::const_iterator it = blueprint.content.elements().begin();
                  it != blueprint.content.elements().end();
                  ++it) {

                if (it->element == Element::CategoryElement &&
                    SectionProcessor<Resource>::isResourceDuplicate(it->content.elements(), uri)) {

                    return true;
                }
            }

            return false;
        }

        /**
         * \brief Given list of elements, return true if none of them is a resource element
         *
         * \param elements Collection of elements
         */
        static bool isResourcePresent(const Elements& elements) {

            for (Elements::const_iterator it = elements.begin(); it != elements.end(); ++it) {

                if (it->element == Element::ResourceElement) {
                    return false;
                }
            }

            return true;
        }

        /**
         * \brief Given list of elements, get the last resource
         *
         * \param elements Collection fo elements
         */
        static Resource lastResource(const Elements& elements) {

            Resource last;

            for (Elements::const_iterator it = elements.begin(); it != elements.end(); ++it) {

                if (it->element == Element::ResourceElement) {
                    last = it->content.resource;
                }
            }

            return last;
        }
    };

    /** ResourceGroup Section Parser */
    typedef SectionParser<ResourceGroup, HeaderSectionAdapter> ResourceGroupParser;
}

#endif
