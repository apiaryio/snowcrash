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

using namespace scpl;

namespace mson {

    /**
     * \brief Check if the given string is variable
     *
     * \param String to check
     *
     * \return True if variable string
     */
    inline bool checkVariable(const std::string& subject) {

        std::string emphasisChars = mdp::MarkdownEmphasisChars;

        if (emphasisChars.find(subject[0]) != std::string::npos &&
            subject[0] == subject[subject.length() - 1]) {

            return true;
        }

        return false;
    }

    /**
     * \brief Parse a MSON BaseTypeName into MSON BaseType
     *
     * \param type MSON BaseTypeName
     *
     * \return MSON BaseType
     */
    inline BaseType parseBaseType(const BaseTypeName& type) {

        if ((type == StringTypeName) ||
            (type == NumberTypeName) ||
            (type == BooleanTypeName)) {

            return PrimitiveBaseType;
        }

        if (type == ObjectTypeName) {
            return ObjectBaseType;
        }

        if ((type == ArrayTypeName) ||
            (type == EnumTypeName)) {

            return ValueBaseType;
        }

        return UndefinedBaseType;
    }

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

        if (checkVariable(subject)) {

            std::string escapedString = snowcrash::RetrieveEscaped(buffer, 0, true);

            value.literal = escapedString;
            value.variable = true;
        }

        if (value.literal.empty()) {
            value.literal = snowcrash::StripBackticks(buffer);
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
     * \param typeName MSON Type Name
     * \param isBaseType If false, will be parsed as a symbol
     */
    inline void parseTypeName(const std::string& subject,
                              TypeName& typeName,
                              bool isBaseType = true) {

        if (isBaseType && subject == "boolean") {
            typeName.base = BooleanTypeName;
        }
        else if (isBaseType && subject == "string") {
            typeName.base = StringTypeName;
        }
        else if (isBaseType && subject == "number") {
            typeName.base = NumberTypeName;
        }
        else if (isBaseType && subject == "array") {
            typeName.base = ArrayTypeName;
        }
        else if (isBaseType && subject == "enum") {
            typeName.base = EnumTypeName;
        }
        else if (isBaseType && subject == "object") {
            typeName.base = ObjectTypeName;
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
        else if (attribute == "nullable") {
            typeAttributes |= NullableTypeAttribute;
        }
        else if (attribute == "fixed-type") {
            typeAttributes |= FixedTypeTypeAttribute;
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
            else if (subject[i] == scpl::Delimiters::AttributeDelimiter &&
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
     * \brief Add a dependency to the dependency list of the dependents while checking for circular references
     *
     * \param node Current markdown node iterator
     * \param pd Section parser data
     * \param dependency The named type which should be added to the dependency list
     * \param dependent The named type to which the dependency should be added
     * \param report Parse result report
     */
    inline void addDependency(const mdp::MarkdownNodeIterator& node,
                              snowcrash::SectionParserData& pd,
                              const mson::Literal dependency,
                              const mson::Literal dependent,
                              snowcrash::Report& report,
                              bool circularCheck = false) {

        // First, check if the type exists
        if (pd.namedTypeDependencyTable.find(dependency) == pd.namedTypeDependencyTable.end()) {

            // ERR: We cannot find the dependency type
            std::stringstream ss;
            ss << "base type '" << dependency << "' is not defined in the document";

            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
            report.error = snowcrash::Error(ss.str(), snowcrash::MSONError, sourceMap);
            return;
        }

        std::set<mson::Literal> dependencyDeps = pd.namedTypeDependencyTable[dependency];

        // Second, check if it is circular reference between them
        if (circularCheck && (dependent == dependency ||
                              dependencyDeps.find(dependent) != dependencyDeps.end())) {

            // ERR: Dependency named type circular references itself
            std::stringstream ss;
            ss << "base type '" << dependent << "' circularly referencing itself";

            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
            report.error = snowcrash::Error(ss.str(), snowcrash::MSONError, sourceMap);
            return;
        }

        // Third, check if the dependency is already in the list
        if (pd.namedTypeDependencyTable[dependent].find(dependency) != pd.namedTypeDependencyTable[dependent].end()) {
            return;
        }

        for (mson::NamedTypeDependencyTable::iterator it = pd.namedTypeDependencyTable.begin();
             it != pd.namedTypeDependencyTable.end();
             ++it) {

            // If the entry is dependent itself or contain dependent in its list
            if (it->first == dependent || it->second.find(dependent) != it->second.end()) {

                it->second.insert(dependency);
                it->second.insert(dependencyDeps.begin(), dependencyDeps.end());
            }
        }
    }

    /**
     * \brief Parse Type Definition from a list of signature attributes
     *
     * \param node Markdown node of the signature
     * \param pd Section parser data
     * \param attributes List of signature attributes
     * \param report Parse result report
     * \param typeDefinition MSON Type Definition
     */
    inline void parseTypeDefinition(const mdp::MarkdownNodeIterator& node,
                                    snowcrash::SectionParserData& pd,
                                    const std::vector<std::string>& attributes,
                                    snowcrash::Report& report,
                                    mson::TypeDefinition& typeDefinition) {

        bool foundTypeSpecification = false;

        for (std::vector<std::string>::const_iterator it = attributes.begin();
             it != attributes.end();
             it++) {

            // If not a recognized type attribute
            if (!parseTypeAttribute(*it, typeDefinition.attributes)) {

                // If type specification is already found
                if (foundTypeSpecification) {

                    // WARN: Ignoring unrecognized type attribute
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                    report.warnings.push_back(snowcrash::Warning("ignoring unrecognized type attribute",
                                                                 snowcrash::IgnoringWarning,
                                                                 sourceMap));
                }
                else {

                    foundTypeSpecification = true;
                    parseTypeSpecification(*it, typeDefinition.typeSpecification);
                }
            }
        }

        typeDefinition.baseType = parseBaseType(typeDefinition.typeSpecification.name.base);
        NamedTypeBaseTable::iterator it = pd.namedTypeBaseTable.find(typeDefinition.typeSpecification.name.symbol.literal);

        if (typeDefinition.baseType == UndefinedBaseType &&
            it != pd.namedTypeBaseTable.end()) {

            typeDefinition.baseType = it->second;
        }

        if (typeDefinition.baseType == UndefinedBaseType &&
            !typeDefinition.typeSpecification.name.symbol.literal.empty() &&
            !typeDefinition.typeSpecification.name.symbol.variable) {

            addDependency(node, pd, typeDefinition.typeSpecification.name.symbol.literal, pd.namedTypeContext, report);
        }

        if (typeDefinition.baseType != ValueBaseType &&
            typeDefinition.baseType != ImplicitValueBaseType &&
            !typeDefinition.typeSpecification.nestedTypes.empty()) {

            // WARN: Nested types for non (array or enum) structure base type
            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
            report.warnings.push_back(snowcrash::Warning("nested types should be present only for types which are sub typed from either array or enum structure type",
                                                         snowcrash::LogicalErrorWarning,
                                                         sourceMap));
        }
    }

    /**
     * \brief Parse Property Name from a string given by signature identifier
     *
     * \param node Markdown node of the signature
     * \param pd Section parser data
     * \param subject String representing the property name
     * \param report Parse result report
     * \param propertyName MSON Property Name
     */
    inline void parsePropertyName(const mdp::MarkdownNodeIterator& node,
                                  snowcrash::SectionParserData& pd,
                                  const std::string& subject,
                                  snowcrash::Report& report,
                                  PropertyName& propertyName) {

        std::string buffer = subject;

        if (checkVariable(subject)) {

            std::string escapedString = snowcrash::RetrieveEscaped(buffer, 0, true);

            SignatureTraits traits(SignatureTraits::ValuesTrait |
                                   SignatureTraits::AttributesTrait);

            Signature signature = SignatureSectionProcessorBase<PropertyName>::parseSignature(node, pd, traits, report, escapedString);

            if (!signature.value.empty()) {
                propertyName.variable.values.push_back(parseValue(signature.value));
            }

            parseTypeDefinition(node, pd, signature.attributes, report, propertyName.variable.typeDefinition);
        }
        else {
            propertyName.literal = subject;
        }
    }

    /**
     * \brief Check is the given base types are the same after taking the implicitness into account
     */
    inline bool isSameBaseType(const mson::BaseType lhs,
                               const mson::BaseType rhs) {

        if (lhs == rhs ||
            (lhs == mson::ImplicitObjectBaseType && rhs == mson::ObjectBaseType) ||
            (lhs == mson::ObjectBaseType && rhs == mson::ImplicitObjectBaseType) ||
            (lhs == mson::ImplicitValueBaseType && rhs == mson::ValueBaseType) ||
            (lhs == mson::ValueBaseType && rhs == mson::ImplicitValueBaseType) ||
            (lhs == mson::ImplicitPrimitiveBaseType && rhs == mson::PrimitiveBaseType) ||
            (lhs == mson::PrimitiveBaseType && rhs == mson::ImplicitPrimitiveBaseType)) {

            return true;
        }

        return false;
    }
}

#endif
