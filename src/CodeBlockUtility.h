//
//  CodeBlockUtility.h
//  snowcrash
//
//  Created by Zdenek Nemec on 11/10/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_CODEBLOCKUTILITY_H
#define SNOWCRASH_CODEBLOCKUTILITY_H

#include <sstream>
#include "Section.h"
#include "StringUtility.h"

namespace snowcrash {

    struct CodeBlockUtility {

        /**
         *  \brief Expected indentation level of a code block.
         *
         *  \param  type    Section type to retrieve the indentation level for
         *  \return Indentation level (number of tabs) for a block to be
         *          considered a pre-formatted code block in given section.
         */
        static size_t codeBlockIndentationLevel(const SectionType& type)
        {
            if (type == BlueprintSectionType ||
                type == ResourceGroupSectionType ||
                type == ResourceSectionType ||
                type == ActionSectionType) {

                return 1;
            }
            else if (type == RequestBodySectionType ||
                     type == ResponseBodySectionType ||
                     type == ModelBodySectionType) {

                return 2;
            }
            else {

                return 3;
            }
        }

        /**
         *  \brief  Retrieve the textual content of a Markdown node as if it was a code block.
         *  \param  pd      Parser status
         *  \param  report  Report log
         *  \param  conten  The content retrieved
         */
        static void contentAsCodeBlock(const MarkdownNodeIterator& node,
                                       const SectionParserData& pd,
                                       Report& report,
                                       mdp::ByteBuffer& content) {

            checkPossibleReference(node, pd, report);

            if (node->type == mdp::CodeMarkdownNodeType) {
                content += node->text;

                checkExcessiveIndentation(node, pd, report);
                return;
            }

            // Other blocks, process & warn
            content += mdp::MapBytesRangeSet(node->sourceMap, pd.sourceData);

            // WARN: Not a preformatted code block
            size_t level = codeBlockIndentationLevel(pd.parentSectionContext());
            std::stringstream ss;
            ss << SectionName(pd.sectionContext());

            if (pd.sectionContext() == BodySectionType) {
                ss << " asset";
            }

            ss << " is expected to be a pre-formatted code block, every of its line indented by exactly ";
            ss << level * 4 << " spaces or " << level << " tabs";

            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
            report.warnings.push_back(Warning(ss.str(),
                                              IndentationWarning,
                                              sourceMap));
        }

        /** \brief  Retrieve the textual content of a signature markdown */
        static void signatureContentAsCodeBlock(const MarkdownNodeIterator& node,
                                                const SectionParserData& pd,
                                                Report& report,
                                                mdp::ByteBuffer& content) {

            mdp::ByteBuffer remainingContent;
            GetFirstLine(node->text, remainingContent);

            if (remainingContent.empty())
                return;

            content += remainingContent;
            content += "\n";

            // WARN: Not a preformatted code block but multiline signature
            size_t level = codeBlockIndentationLevel(pd.parentSectionContext());
            std::stringstream ss;

            ss << SectionName(pd.sectionContext());

            if (pd.sectionContext() == BodySectionType) {
                ss << " asset";
            }

            ss << " is expected to be a pre-formatted code block, separate it by a newline and ";
            ss << "indent every of its line by ";
            ss << level * 4 << " spaces or " << level << " tabs";

            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
            report.warnings.push_back(Warning(ss.str(),
                                              IndentationWarning,
                                              sourceMap));
        }

        /**
         *  \brief Check for potential excessive indentation of a list section
         *  \return True if code block contains a recognized list section, false otherwise.
         */
        static bool checkExcessiveIndentation(const MarkdownNodeIterator& node,
                                              const SectionParserData& pd,
                                              Report& report) {

            // Check for possible superfluous indentation of a recognized list items.
            mdp::ByteBuffer r;
            mdp::ByteBuffer line = GetFirstLine(node->text, r);
            TrimString(line);

            // If line appears to be a Markdown list.
            if (line.empty() ||
                (line[0] != '-' && line[0] != '+' && line[0] != '*'))
                return false;

            // Skip leading Markdown list item mark
            std::string signature = line.substr(1, std::string::npos);
            TrimStringStart(signature);

            SectionType type = RecognizeCodeBlockFirstLine(signature);

            if (type != UndefinedSectionType) {

                // Check signature.
                size_t level = codeBlockIndentationLevel(pd.sectionContext());
                --level;

                // WARN: Superfluous indentation
                std::stringstream ss;
                ss << "excessive indentation, ";
                ss << SectionName(type) << " ";

                if (level) {
                    ss << "section is expected to be indented by just ";
                    ss << level * 4 << " spaces or " << level << " tab";

                    if (level > 1)
                        ss << "s";
                }
                else {
                    ss << "section is not expected to be indented";
                }

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                report.warnings.push_back(Warning(ss.str(),
                                                  IndentationWarning,
                                                  sourceMap));
            }

            return false;
        }

        /**
         *  \brief  Parse one line of raw `key:value` data.
         *  \param  line            A line to parse
         *  \param  keyValuePair    The output buffer to place the parsed data into
         *  \return True on success, false otherwise
         */
        static bool keyValueFromLine(const mdp::ByteBuffer& line,
                                    KeyValuePair& keyValuePair) {

            std::vector<std::string> rawMetadata = SplitOnFirst(line, ':');
            if (rawMetadata.size() != 2)
                return false;

            keyValuePair = std::make_pair(rawMetadata[0], rawMetadata[1]);
            TrimString(keyValuePair.first);
            TrimString(keyValuePair.second);

            return (!keyValuePair.first.empty() && !keyValuePair.second.empty());
        }

        /**
         *  \brief Add dangling message body asset to the given string
         *  \param  out  The string to which the dangling asset should be added
         *  \param  outSM The source map to which source map should be added
         *  \return asset The value of the asset
         */
        static mdp::ByteBuffer addDanglingAsset(const MarkdownNodeIterator& node,
                                                SectionParserData& pd,
                                                SectionType& sectionType,
                                                Report& report,
                                                mdp::ByteBuffer& out) {

            mdp::ByteBuffer asset;

            if (node->type == mdp::CodeMarkdownNodeType) {
                asset = node->text;
            } else {
                asset = mdp::MapBytesRangeSet(node->sourceMap, pd.sourceData);
            }

            TwoNewLines(asset);
            out += asset;

            size_t level = CodeBlockUtility::codeBlockIndentationLevel(sectionType);

            if (node->type == mdp::CodeMarkdownNodeType)
                level--;  // Deduct one level for a code block

            checkPossibleReference(node, pd, report);

            if (level) {
                // WARN: Dangling asset
                std::stringstream ss;
                ss << "dangling message-body asset, expected a pre-formatted code block, ";
                ss << "indent every of it's line by " << level*4 << " spaces or " << level << " tabs";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                report.warnings.push_back(Warning(ss.str(),
                                                  IndentationWarning,
                                                  sourceMap));
            }

            return asset;
        }

        /**
         *  \brief Check for potential resource model reference and warn about not recognizing them
         *  \return True if code block contains a resource model reference, false otherwise.
         */
        static bool checkPossibleReference(const MarkdownNodeIterator& node,
                                           const SectionParserData& pd,
                                           Report& report) {

            mdp::ByteBuffer source = node->text;
            Identifier symbol;

            TrimString(source);

            if(GetModelReference(source, symbol)) {

                std::stringstream ss;
                ss << "found a possible '" << symbol << "' model reference, ";
                ss << "a reference must be directly in the " << SectionName(pd.sectionContext()) << " section, indented by 4 spaces or 1 tab, without any additional sections";

                mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                report.warnings.push_back(Warning(ss.str(),
                                                  IgnoringWarning,
                                                  sourceMap));

                return true;
            }

            return false;
        }
    };
}

#endif
