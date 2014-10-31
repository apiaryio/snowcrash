//
//  MSONUtility.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/23/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONUTILITY_H
#define SNOWCRASH_MSONUTILITY_H

#include "MSON.h"

namespace mson {

    /**
     * \brief Parse Value from a string
     *
     * \param subject String which represents the value
     *
     * \return MSON Value
     */
    inline Value parseValue(std::string& subject) {

        Value value;
        size_t len = subject.length();

        if ((subject[0] == '*' && subject[len - 1] == '*') ||
            (subject[0] == '_' && subject[len - 1] == '_')) {

            std::string escapedString = snowcrash::RetrieveEscaped(subject, 0, true);

            value.literal = escapedString;
            value.variable = true;
        }

        if (value.literal.empty()) {
            value.literal = subject;
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
    inline Symbol parseSymbol(std::string& subject) {

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
    inline void parseTypeName(std::string& subject,
                              TypeName& typeName) {

        if (subject == "boolean") {
            typeName.name = BooleanTypeName;
        } else if (subject == "string") {
            typeName.name = StringTypeName;
        } else if (subject == "number") {
            typeName.name = NumberTypeName;
        } else if (subject == "array") {
            typeName.name = ArrayTypeName;
        } else if (subject == "enum") {
            typeName.name = EnumTypeName;
        } else if (subject == "object") {
            typeName.name = ObjectTypeName;
        } else {
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
        } else if (attribute == "optional") {
            typeAttributes |= OptionalTypeAttribute;
        } else if (attribute == "fixed") {
            typeAttributes |= FixedTypeAttribute;
        } else if (attribute == "sample") {
            typeAttributes |= SampleTypeAttribute;
        } else if (attribute == "default") {
            typeAttributes |= DefaultTypeAttribute;
        } else {
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
    inline void parseTypeSpecification(std::string subject,
                                       TypeSpecification& typeSpecification) {

        subject = snowcrash::StripMarkdownLink(subject);

        bool lookingAtNested = false;
        bool lookingForEndLink = false;
        bool alreadyParsedLink = false;
        bool trimSubject = false;

        size_t i = 0;
        std::string value = "";

        while (i < subject.length()) {

            if (subject[i] == '[' && !alreadyParsedLink) {

                trimSubject = true;

                if (!lookingAtNested) {

                    snowcrash::TrimString(value);

                    if (!value.empty()) {
                        parseTypeName(value, typeSpecification.name);
                    }

                    lookingAtNested = true;
                } else {
                    lookingForEndLink = true;
                }
            } else if (subject[i] == ']' && lookingAtNested && lookingForEndLink) {

                trimSubject = true;

                TypeName typeName;
                snowcrash::TrimString(value);

                if (!value.empty()) {
                    parseTypeName(value, typeName);
                    typeSpecification.nestedTypes.push_back(typeName);
                }

                alreadyParsedLink = true;
                lookingForEndLink = false;
            } else if (subject[i] == ',' && lookingAtNested && !lookingForEndLink) {

                trimSubject = true;

                if (alreadyParsedLink) {
                    alreadyParsedLink = false;
                } else {

                    TypeName typeName;
                    snowcrash::TrimString(value);

                    if (!value.empty()) {
                        parseTypeName(value, typeName);
                        typeSpecification.nestedTypes.push_back(typeName);
                    }
                }
            } else {

                value += subject[i];
                i++;
            }

            // Strip the subject until the current index
            if (trimSubject) {

                subject = subject.substr(i + 1);
                snowcrash::TrimString(subject);

                trimSubject = false;
                value = "";
                i = 0;
            }
        }

        snowcrash::TrimString(value);

        if (value.empty() || alreadyParsedLink) {
            return;
        }

        // Remove the ending square bracket
        if (lookingAtNested && value[value.length() - 1] == ']') {
            value = value.substr(0, value.length() - 1);
        }

        TypeName typeName;

        snowcrash::TrimString(value);
        parseTypeName(value, typeName);

        if (lookingAtNested) {
            typeSpecification.nestedTypes.push_back(typeName);
        } else {
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
                                    std::vector<std::string>& attributes,
                                    const snowcrash::ParseResultRef<TypeDefinition>& out) {

        bool foundTypeSpecification = false;
        std::vector<std::string>::iterator it;

        for (it = attributes.begin(); it != attributes.end(); it++) {

            // If not a recognized type attribute
            if (!parseTypeAttribute(*it, out.node.attributes)) {

                // If type specification is already found
                if (foundTypeSpecification) {

                    // WARN: Ignoring unrecognized type attribute
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    out.report.warnings.push_back(snowcrash::Warning("ignoring unrecognized type attribute",
                                                                     snowcrash::IgnoringWarning,
                                                                     sourceMap));
                } else {

                    foundTypeSpecification = true;
                    parseTypeSpecification(*it, out.node.typeSpecification);
                }
            }
        }
    }
}

#endif
