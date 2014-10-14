//
//  MSON.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/9/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSON_H
#define SNOWCRASH_MSON_H

#include <deque>
#include <string>
#include "Platform.h"
#include "MarkdownParser.h"

/**
 * MSON Abstract Syntax Tree
 * -------------------------
 *
 * Data types in this document define the MSON AST
 */

namespace mson {

    /** Markdown */
    typedef mdp::ByteBuffer Markdown;

    /** Literal */
    typedef std::string Literal;

    /** A simple or actual value */
    struct Value {

        /** Literal value */
        Literal literal;

        /** Flag to denote variable value */
        bool variable;
    };

    /** Collection of values */
    typedef std::deque<Value> Values;

    /** Type symbol (identifier) */
    struct Symbol {

        /** Name of the symbol */
        Literal literal;

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

    /** Collection of type names */
    typedef std::deque<TypeName> TypeNames;

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
        TypeNames nestedTypes;
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
        Values values;

        /** Type of the values */
        TypeDefinition typedefinition;
    };

    /** Forward Declaration for member type */
    struct MemberType;

    /** Collection of member types */
    typedef std::deque<MemberType> MemberTypes;

    /** Type of a type section */
    enum TypeSectionType {
        UndefinedTypeSectionType,        // Unknown
        BlockDescriptionTypeSectionType, // Markdown block description
        MemberTypeSectionType,           // Contains member types
        SampleTypeSectionType,           // Sample member types
        DefaultTypeSectionType           // Default member types
    };

    /** Section of a type */
    struct TypeSection {

        /** Content of the type section */
        struct TypeSectionContent {

            /** EITHER Block description */
            Markdown description;

            /** OR Array of member types */
            MemberTypes& members();
            const MemberTypes& members() const;

            /** Checks if member types are present */
            bool hasMembers() const;

        private:
            std::auto_ptr<MemberTypes> m_members;
        };

        /** Denotes the type of the section */
        TypeSectionType type;

        /** Content of the type section */
        TypeSectionContent content;
    };

    /** Collection of type sections */
    typedef std::deque<TypeSection> TypeSections;

    /** User-define named type */
    struct NamedType {

        /** Name of the type */
        TypeName name;

        /** The ancestor type definition */
        TypeDefinition base;

        /** List of named type's sections */
        TypeSections sections;
    };

    /** Individual member of an array or enum structure */
    struct ValueMember {

        /** Inline description */
        Markdown description;

        /** Definition of member's value */
        ValueDefinition valueDefinition;

        /** List of member type's sections */
        TypeSections sections;
    };

    /** Name of a property member */
    struct PropertyName {

        /** EITHER Literal name of the property */
        Literal literal;

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
        MemberTypes& members();
        const MemberTypes& members() const;

        /** Checks if member types are present */
        bool hasMembers() const;

    private:
        std::auto_ptr<MemberTypes> m_members;
    };

    /** Type of a member type */
    enum MemberTypeType {
        UndefinedMemberType,  // Unknown
        PropertyMemberType,   // Property member
        ValueMemberType,      // Value member
        MixinMemberType,      // Mixin
        OneOfMemberType       // One of
    };

    /** Member type of a structure */
    struct MemberType {

        /** Content of the member type */
        struct MemberTypeContent {

            /** EITHER Property member */
            PropertyMember property;

            /** OR Value member */
            ValueMember value;

            /** OR Mixin member */
            Mixin mixin;

            /** OR One of member */
            OneOf oneOf;
        };

        /** Type of the member type */
        MemberTypeType type;

        /** Content of the member type */
        MemberTypeContent content;
    };
}

#endif
