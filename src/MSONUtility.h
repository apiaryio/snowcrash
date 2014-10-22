//
//  MSONUtility.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/23/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONUTILITY_H
#define SNOWCRASH_MSONUTILITY_H

#include "MSONSourcemap.h"

namespace mson {

    /**
     * \brief Parse a string into MSON Value structure
     *
     * \param subject String which represents the value
     *
     * \return MSON Value
     */
    inline Value parseValue(const std::string& subject) {

        Value value;

        std::string buffer = subject;
        size_t len = buffer.length();

        std::string emphasisChars = mdp::MarkdownEmphasisChars;

        if (emphasisChars.find(buffer[0]) != std::string::npos &&
            buffer[0] == buffer[len - 1]) {

            std::string escapedString = snowcrash::RetrieveEscaped(buffer, 0, true);

            value.literal = escapedString;
            value.variable = true;
        }

        if (value.literal.empty()) {
            value.literal = buffer;
        }

        // When the value is a wildcard
        if (value.literal == "*") {

            value.literal = "";
            value.variable = true;
        }

        return value;
    }

    /**
     * \brief Parse Symbol from a string
     *
     * \param subject String which represents the symbol
     *
     * \return MSON Symbol
     */
    inline Symbol parseSymbol(const std::string& subject) {

        Symbol symbol;
        Value value = parseValue(subject);

        symbol.literal = value.literal;
        symbol.variable = value.variable;

        return symbol;
    }

    /**
     * \brief Parse Type Name from a string
     *
     * \param subject String which represents the type name
     * \param out MSON Type Name
     */
    inline void parseTypeName(const std::string& subject,
                              TypeName& typeName) {

        if (subject == "boolean") {
            typeName.name = BooleanTypeName;
        }
        else if (subject == "string") {
            typeName.name = StringTypeName;
        }
        else if (subject == "number") {
            typeName.name = NumberTypeName;
        }
        else if (subject == "array") {
            typeName.name = ArrayTypeName;
        }
        else if (subject == "enum") {
            typeName.name = EnumTypeName;
        }
        else if (subject == "object") {
            typeName.name = ObjectTypeName;
        }
        else {
            typeName.symbol = parseSymbol(subject);
        }
    }

    /**
     * \brief Check Type Attribute from a string and fill list of type attributes accordingly
     *
     * \param attribute String that needs to be checked for attribute
     * \param typeAttributes List of type attributes
     *
     * \return True if the given string is a type attribute
     */
    inline bool parseTypeAttribute(const std::string& attribute,
                                   TypeAttributes& typeAttributes) {

        bool isAttribute = true;

        if (attribute == "required") {
            typeAttributes |= RequiredTypeAttribute;
        }
        else if (attribute == "optional") {
            typeAttributes |= OptionalTypeAttribute;
        }
        else if (attribute == "fixed") {
            typeAttributes |= FixedTypeAttribute;
        }
        else if (attribute == "sample") {
            typeAttributes |= SampleTypeAttribute;
        }
        else if (attribute == "default") {
            typeAttributes |= DefaultTypeAttribute;
        }
        else {
            isAttribute = false;
        }

        return isAttribute;
    }

    /**
     * \brief Parse Type Specification from a signature attribute
     *
     * \param subject Attribute string representing the type specification
     * \param typeSpecification MSON Type Specification
     */
    inline void parseTypeSpecification(const std::string& attribute,
                                       TypeSpecification& typeSpecification) {

        std::string subject = snowcrash::StripMarkdownLink(attribute);

        bool lookingAtNested = false;    // If true, we are looking at nested types
        bool lookingForEndLink = false;  // If true, we detected a link text and are looking for the end of it
        bool alreadyParsedLink = false;  // If true, we already parsed the link text in the link

        bool trimSubject = false;

        size_t i = 0;
        std::string buffer;

        while (i < subject.length()) {

            if (subject[i] == mdp::MarkdownBeginReference &&
                !alreadyParsedLink) {

                trimSubject = true;

                if (!lookingAtNested) {

                    snowcrash::TrimString(buffer);

                    if (!buffer.empty()) {
                        parseTypeName(buffer, typeSpecification.name);
                    }

                    lookingAtNested = true;
                }
                else {
                    lookingForEndLink = true;
                }
            }
            else if (subject[i] == mdp::MarkdownEndReference &&
                     lookingAtNested &&
                     lookingForEndLink) {

                trimSubject = true;

                TypeName typeName;
                snowcrash::TrimString(buffer);

                if (!buffer.empty()) {

                    parseTypeName(buffer, typeName);
                    typeSpecification.nestedTypes.push_back(typeName);
                }

                alreadyParsedLink = true;
                lookingForEndLink = false;
            }
            else if (subject[i] == scpl::AttributeDelimiter &&
                     lookingAtNested &&
                     !lookingForEndLink) {

                trimSubject = true;

                if (alreadyParsedLink) {
                    alreadyParsedLink = false;
                }
                else {

                    TypeName typeName;
                    snowcrash::TrimString(buffer);

                    if (!buffer.empty()) {

                        parseTypeName(buffer, typeName);
                        typeSpecification.nestedTypes.push_back(typeName);
                    }
                }
            }
            else {

                buffer += subject[i];
                i++;
            }

            // Strip the subject until the current index
            if (trimSubject) {

                subject = subject.substr(i + 1);
                snowcrash::TrimString(subject);

                trimSubject = false;
                buffer.clear();
                i = 0;
            }
        }

        snowcrash::TrimString(buffer);

        if (buffer.empty() || alreadyParsedLink) {
            return;
        }

        // Remove the ending square bracket
        if (lookingAtNested &&
            buffer[buffer.length() - 1] == mdp::MarkdownEndReference) {

            buffer = buffer.substr(0, buffer.length() - 1);
        }

        TypeName typeName;

        snowcrash::TrimString(buffer);
        parseTypeName(buffer, typeName);

        if (lookingAtNested) {
            typeSpecification.nestedTypes.push_back(typeName);
        }
        else {
            typeSpecification.name = typeName;
        }
    }

    /**
     * \brief Parse Type Definition from a list of signature attributes
     *
     * \param node Markdown node of the signature
     * \param pd Section parser data
     * \param attributes List of signature attributes
     * \param out Type Definition parse result
     */
    inline void parseTypeDefinition(const mdp::MarkdownNodeIterator& node,
                                    snowcrash::SectionParserData& pd,
                                    const std::vector<std::string>& attributes,
                                    const snowcrash::ParseResultRef<TypeDefinition>& out) {

        bool foundTypeSpecification = false;

        for (std::vector<std::string>::const_iterator it = attributes.begin();
             it != attributes.end();
             it++) {

            // If not a recognized type attribute
            if (!parseTypeAttribute(*it, out.node.attributes)) {

                // If type specification is already found
                if (foundTypeSpecification) {

                    // WARN: Ignoring unrecognized type attribute
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.warnings.push_back(snowcrash::Warning("ignoring unrecognized type attribute",
                                                                     snowcrash::IgnoringWarning,
                                                                     sourceMap));
                }
                else {

                    foundTypeSpecification = true;
                    parseTypeSpecification(*it, out.node.typeSpecification);
                }
            }
        }
    }
}

#endif
