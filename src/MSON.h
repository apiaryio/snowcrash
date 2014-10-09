//
//  MSON.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/9/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSON_H
#define SNOWCRASH_MSON_H

#include <vector>
#include <string>
#include "Platform.h"
#include "Markdown.h"

/**
 * MSON Abstract Syntax Tree
 * -------------------------
 *
 * Data types in this document define the MSON AST
 */

namespace mson {

    /** A simple or actual value */
    struct Value {

        /** Literal value */
        std::string literal;

        /** Flag to denote variable value */
        bool variable;
    };

    /** Type symbol (identifier) */
    struct Symbol {

        /** Name of the symbol */
        std::string literal;

        /** Flag to denote variable type name */
        bool variable;
    };

    /** Value of type name if based type */
    enum BaseTypeName {
        UndefinedTypeName, // Not a base type name
        BooleanTypeName,   // `boolean` type name
        StringTypeName,    // `string` type name
        NumberTypeName,    // `number` type name
        ArrayTypeName,     // `array` type name
        EnumTypeName,      // `enum` type name
        ObjectTypeName     // `object` type name
    };

    /** Base or named type's name */
    struct TypeName {

        /** EITHER Base type's value */
        BaseTypeName name;

        /** OR Named type's identifier */
        Symbol symbol;
    };

    /** Attribute of a type */
    enum TypeAttribute {
        RequiredTypeAttribute = (1 << 0),  // The type is required
        OptionalTypeAttribute = (1 << 1),  // The type is optional
        DefaultTypeAttribute  = (1 << 2),  // The type is default
        SampleTypeAttribute   = (1 << 3),  // The type is a sample
        FixedTypeAttribute    = (1 << 4)   // The type is fixed
    };

    /** List of type attributes */
    typedef unsigned int TypeAttributes;

    /** Defines sub-typed types for a type */
    struct TypeSpecification {

        /** Name of the type */
        TypeName name;

        /** Array of nested types */
        std::vector<TypeName> nestedTypes;
    };

    /** Definition of an instance of a type */
    struct TypeDefinition {

        /** Type specification */
        TypeSpecification typeSpecification;

        /** List of type attributes (byte-wise OR) */
        TypeAttributes attributes;
    };

    /** Value definition of a type instance */
    struct ValueDefinition {

        /** List of values */
        std::vector<Value> values;

        /** Type of the values */
        TypeDefinition typedefinition;
    };

    /** Forward Declaration for member type */
    struct MemberType;

    /** Type of a type section */
    enum TypeSectionType {
        UndefinedTypeSectionType,        // Unknown
        BlockDescriptionTypeSectionType, // Markdown block description
        MemberTypeSectionType,           // Contains member types
        SampleTypeSectionType,           // Sample member types
        DefaultTypeSectionType           // Default member types
    };

    /** Content of a type section */
    struct TypeSectionContent {

        /** EITHER Block description */
        mdp::ByteBuffer description;

        /** OR Array of member types */
        std::vector<MemberType> members;
    };

    /** Section of a type */
    struct TypeSection {

        /** Denotes the type of the section */
        TypeSectionType type;

        /** Content of the section */
        TypeSectionContent content;
    };

    /** User-define named type */
    struct NamedType {

        /** Name of the type */
        TypeName name;

        /** The ancestor type definition */
        TypeDefinition base;

        /** List of named type's sections */
        std::vector<TypeSection> sections;
    };

    /** Individual member of an array or enum structure */
    struct ValueMember {

        /** Inline description */
        mdp::ByteBuffer description;

        /** Definition of member's value */
        ValueDefinition valueDefinition;

        /** List of member type's sections */
        std::vector<TypeSection> sections;
    };

    /** Name of a property member */
    struct PropertyName {

        /** EITHER Literal name of the property */
        std::string literal;

        /** OR Variable name of the property */
        ValueDefinition variable;
    };

    /** Individual member of an object structure */
    struct PropertyMember : public ValueMember {

        /** Name of the property */
        PropertyName name;
    };

    /** Mixin type */
    struct Mixin {

        /** Type definition of the type to be included */
        TypeDefinition typeDefinition;
    };

    /** One Of type */
    struct OneOf {

        /** List of mutually exclusive member types */
        std::vector<MemberType> members;
    };

    /** Type of a member type */
    enum MemberTypeType {
        UndefinedMemberType,  // Unknown
        PropertyMemberType,   // Property member
        ValueMemberType,      // Value member
        MixinMemberType,      // Mixin
        OneOfMemberType       // One of
    };

    struct MemberTypeContent {

        /** Property member */
        PropertyMember property;

        /** Value member */
        ValueMember value;

        /** Mixin member */
        Mixin mixin;

        /** One of member */
        OneOf oneOf;
    };

    /** Member type of a structure */
    struct MemberType {

        /** Type of the member type */
        MemberTypeType type;

        /** Content of the member type */
        MemberTypeContent content;
    };
}

#endif
