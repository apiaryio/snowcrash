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
     *
     * \return MSON Type Name
     */
    inline TypeName parseTypeName(std::string& subject){

        TypeName typeName;

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

        return typeName;
    }

    /**
     * \brief Parse Type Attributes from a list of signature attributes
     *
     * \param attributes List of signature attributes
     *
     * \return MSON Type Attributes
     */
    inline TypeAttributes parseTypeAttributes() {

        TypeAttributes typeAttributes = 0;

        return typeAttributes;
    }
}

#endif
