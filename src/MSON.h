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
#include <map>
#include <stdexcept>

#include "Platform.h"
#include "MarkdownParser.h"

#define MEMBERS_NOT_SET_ERR std::logic_error("no members set")

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

    /**
     * Kind of Base Type
     *
     * This is an internal thing to keep track of what kind of type,
     * that particular named type or member is sub-typed from
     */
    enum BaseType {
        UndefinedBaseType = 0,     // Undefined
        PrimitiveBaseType,         // Primitive Types
        ImplicitPrimitiveBaseType, // Primitive Types (implicit)
        ObjectBaseType,            // Object Structure Type
        ImplicitObjectBaseType,    // Object Structure Type (implicit)
        ValueBaseType,             // Array & Enum Structure Type
        ImplicitValueBaseType      // Array & Enum Structure Type (implicit)
    };

    /** Named Types base type table */
    typedef std::map<Literal, BaseType> NamedTypeBaseTable;

    /** Named Types inheritance table */
    typedef std::map<Literal, Literal> NamedTypeInheritanceTable;

    /** A simple or actual value */
    struct Value {

        /** Constructor */
        Value()
        : variable(false) {}

        /** Literal value */
        Literal literal;

        /** Flag to denote variable value */
        bool variable;

        /** Check if empty */
        bool empty() const;
    };

    /** Collection of values */
    typedef std::vector<Value> Values;

    /** Type symbol (identifier) */
    struct Symbol {

        /** Constructor */
        Symbol()
        : variable(false) {}

        /** Name of the symbol */
        Literal literal;

        /** Flag to denote variable type name */
        bool variable;

        /** Check if empty */
        bool empty() const;
    };

    /** Value of type name if based type */
    enum BaseTypeName {
        UndefinedTypeName = 0, // Not a base type name
        BooleanTypeName,       // `boolean` type name
        StringTypeName,        // `string` type name
        NumberTypeName,        // `number` type name
        ArrayTypeName,         // `array` type name
        EnumTypeName,          // `enum` type name
        ObjectTypeName         // `object` type name
    };

    /** Base or named type's name */
    struct TypeName {

        /** Constructor */
        TypeName(const BaseTypeName& base_ = UndefinedTypeName)
        : base(base_) {}

        /** EITHER Base type's value */
        BaseTypeName base;

        /** OR Named type's identifier */
        Symbol symbol;

        /** Check if empty */
        bool empty() const;
    };

    /** Collection of type names */
    typedef std::vector<TypeName> TypeNames;

    /** Attribute of a type */
    enum TypeAttribute {
        RequiredTypeAttribute = (1 << 0),  // The type is required
        OptionalTypeAttribute = (1 << 1),  // The type is optional
        FixedTypeAttribute    = (1 << 2),  // The type is fixed
        SampleTypeAttribute   = (1 << 3),  // The type is a sample
        DefaultTypeAttribute  = (1 << 4)   // The type is default
    };

    /** List of type attributes */
    typedef unsigned int TypeAttributes;

    /** Defines sub-typed types for a type */
    struct TypeSpecification {

        /** Name of the type */
        TypeName name;

        /** Array of nested types */
        TypeNames nestedTypes;

        /** Check if empty */
        bool empty() const;
    };

    /** Definition of an instance of a type */
    struct TypeDefinition {

        /** Constructor */
        TypeDefinition()
        : baseType(UndefinedBaseType), attributes(0) {}

        /**
         * Base Type (for the type definition)
         *
         * Representing the base type from which this member or
         * named type is sub-typed from. Not present in the AST
         */
        BaseType baseType;

        /** Type specification */
        TypeSpecification typeSpecification;

        /** List of type attributes (byte-wise OR) */
        TypeAttributes attributes;

        /** Check if empty */
        bool empty() const;
    };

    /** Value definition of a type instance */
    struct ValueDefinition {

        /** List of values */
        Values values;

        /** Type of the values */
        TypeDefinition typeDefinition;

        /** Check if empty */
        bool empty() const;
    };

    /** Forward Declaration for member type */
    struct MemberType;

    /** Collection of member types */
    typedef std::vector<MemberType> MemberTypes;

    /** Section of a type */
    struct TypeSection {

        /** Type of a type section */
        enum Type {
            UndefinedType = 0,    // Unknown
            BlockDescriptionType, // Markdown block description
            MemberType,           // Contains member types
            SampleType,           // Sample member types
            DefaultType           // Default member types
        };

        /** Content of the type section */
        struct Content {

            /** EITHER Block description */
            Markdown description;

            /** OR Literal value */
            Literal value;

            /** OR Array of member types */
            MemberTypes& members();
            const MemberTypes& members() const;

            /** Constructor */
            Content(const Markdown& description_ = Markdown(), const Literal& value_ = Literal());

            /** Copy constructor */
            Content(const TypeSection::Content& rhs);

            /** Assignment operator */
            TypeSection::Content& operator=(const TypeSection::Content& rhs);

            /** Desctructor */
            ~Content();

        private:
            std::auto_ptr<MemberTypes> m_members;
        };

        /** Constructor */
        TypeSection(const TypeSection::Type& type_ = TypeSection::UndefinedType)
        : baseType(UndefinedBaseType), type(type_) {}

        /** Base Type (for the parent of the type section) */
        BaseType baseType;

        /** Denotes the type of the section */
        TypeSection::Type type;

        /** Content of the type section */
        TypeSection::Content content;

        /** Check if empty */
        bool empty() const;
    };

    /** Collection of type sections */
    typedef std::vector<TypeSection> TypeSections;

    /** User-define named type */
    struct NamedType {

        /** Name of the type */
        TypeName name;

        /** The ancestor type definition */
        TypeDefinition typeDefinition;

        /** List of named type's sections */
        TypeSections sections;

        /** Check if empty */
        bool empty() const;
    };

    /** Individual member of an array or enum structure */
    struct ValueMember {

        /** Inline description */
        Markdown description;

        /** Definition of member's value */
        ValueDefinition valueDefinition;

        /** List of member type's sections */
        TypeSections sections;

        /** Check if empty */
        bool empty() const;
    };

    /** Name of a property member */
    struct PropertyName {

        /** EITHER Literal name of the property */
        Literal literal;

        /** OR Variable name of the property */
        ValueDefinition variable;

        /** Check if empty */
        bool empty() const;
    };

    /** Individual member of an object structure */
    struct PropertyMember : public ValueMember {

        /** Name of the property */
        PropertyName name;

        /** Check if empty */
        bool empty() const;
    };

    /** Mixin type */
    typedef TypeDefinition Mixin;

    /** Collection of Member types */
    struct Members {

        /** List of mutually exclusive member types */
        MemberTypes& members();
        const MemberTypes& members() const;

        /** Builds the member structures from the list of member types */
        Members& operator=(const MemberTypes& rhs);

        /** Constructor */
        Members();

        /** Copy constructor */
        Members(const Members& rhs);

        /** Assignment operator */
        Members& operator=(const Members& rhs);

        /** Desctructor */
        ~Members();

        /** Check if empty */
        bool empty() const;

    protected:
        std::auto_ptr<MemberTypes> m_members;
    };

    /** One Of type */
    typedef Members OneOf;

    /** Member of a structure */
    struct MemberType {

        /** Type of a member type */
        enum Type {
            UndefinedType = 0, // Unknown
            PropertyType,      // Property member
            ValueType,         // Value member
            MixinType,         // Mixin
            OneOfType,         // One of
            MembersType        // Members collection
        };

        /** Content of the member type */
        struct Content {

            /** EITHER Property member */
            PropertyMember property;

            /** OR Value member */
            ValueMember value;

            /** OR Mixin member */
            Mixin mixin;

            /** OR One of member */
            OneOf oneOf;

            /** OR Members collection */
            Members members;
        };

        /** Type of the member type */
        MemberType::Type type;

        /** Content of the member type */
        MemberType::Content content;

        /** Constructor */
        MemberType(const MemberType::Type& type_ = MemberType::UndefinedType);

        /** Copy constructor */
        MemberType(const MemberType& rhs);

        /** Assignment operator */
        MemberType& operator=(const MemberType& rhs);

        /** Functions which allow the building of member type */
        void build(const PropertyMember& propertyMember);
        void build(const ValueMember& valueMember);
        void build(const Mixin& mixin);
        void build(const OneOf& oneOf);
        void build(const MemberTypes& memberTypes);
        void build(const Value& value);

        /** Desctructor */
        ~MemberType();
    };
}

#endif
