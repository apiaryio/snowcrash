//
//  MSON.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/4/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "MSON.h"

using namespace mson;

bool Value::empty() const
{
    return (this->literal.empty() && this->variable == false);
}

bool Symbol::empty() const
{
    return (this->literal.empty() && this->variable == false);
}

bool TypeName::empty() const
{
    return (this->name == UndefinedTypeName && this->symbol.empty());
}

bool TypeSpecification::empty() const
{
    return (this->nestedTypes.empty() && this->name.empty());
}

bool TypeDefinition::empty() const
{
    return (this->attributes == 0 && this->typeSpecification.empty());
}

bool ValueDefinition::empty() const
{
    return (this->values.empty() && this->typeDefinition.empty());
}

MemberTypes& TypeSection::Content::members()
{
    if (!m_members.get())
        throw MEMBERS_NOT_SET_ERR;

    return *m_members;
}

const MemberTypes& TypeSection::Content::members() const
{
    if (!m_members.get())
        throw MEMBERS_NOT_SET_ERR;

    return *m_members;
}

TypeSection::Content::Content(const Markdown& description_, const Literal& value_)
: description(description_), value(value_)
{
    m_members.reset(::new MemberTypes);
}

TypeSection::Content::Content(const TypeSection::Content& rhs)
{
    this->description = rhs.description;
    this->value = rhs.value;
    m_members.reset(::new MemberTypes(*rhs.m_members.get()));
}

TypeSection::Content& TypeSection::Content::operator=(const TypeSection::Content& rhs)
{
    this->description = rhs.description;
    this->value = rhs.value;
    m_members.reset(::new MemberTypes(*rhs.m_members.get()));

    return *this;
}

TypeSection::Content::~Content()
{
}

bool TypeSection::empty() const
{
    return (this->type == TypeSection::UndefinedType &&
            this->content.value.empty() &&
            this->content.description.empty() &&
            this->content.members().empty());
}

bool NamedType::empty() const
{
    return (this->base.empty() && this->name.empty() &&
            this->sections.empty());
}

bool ValueMember::empty() const
{
    return (this->valueDefinition.empty() && this->sections.empty() &&
            this->description.empty());
}

bool PropertyName::empty() const
{
    return (this->literal.empty() && this->variable.empty());
}

bool PropertyMember::empty() const
{
    return (this->name.empty() && this->description.empty() &&
            this->sections.empty() && this->valueDefinition.empty());
}

bool Mixin::empty() const
{
    return (this->typeDefinition.empty());
}

MemberTypes& Members::members()
{
    if (!m_members.get())
        throw MEMBERS_NOT_SET_ERR;

    return *m_members;
}

const MemberTypes& Members::members() const
{
    if (!m_members.get())
        throw MEMBERS_NOT_SET_ERR;

    return *m_members;
}

Members& Members::operator=(const MemberTypes& rhs)
{
    m_members.reset(::new MemberTypes(rhs));

    return *this;
}

Members::Members()
{
    m_members.reset(::new MemberTypes);
}

Members::Members(const Members& rhs)
{
    m_members.reset(::new MemberTypes(*rhs.m_members.get()));
}

Members& Members::operator=(const Members& rhs)
{
    m_members.reset(::new MemberTypes(*rhs.m_members.get()));

    return *this;
}

Members::~Members()
{
}

bool Members::empty() const
{
    return (this->members().empty());
}

MemberType::MemberType(const MemberType::Type& type_)
: type(type_)
{
}

MemberType::MemberType(const MemberType& rhs)
{
    this->type = rhs.type;
    this->content = rhs.content;
}

MemberType& MemberType::operator=(const MemberType& rhs)
{
    this->type = rhs.type;
    this->content = rhs.content;

    return *this;
}

MemberType::~MemberType()
{
}

/**
 * \brief Build Member Type from property member
 *
 * \param propertyMember Property member which was given
 */
void MemberType::build(const PropertyMember& propertyMember)
{
    this->type = MemberType::PropertyType;
    this->content.property = propertyMember;
}

/**
 * \brief Build Member Type from value member
 *
 * \param valueMember Value member which was given
 */
void MemberType::build(const ValueMember& valueMember)
{
    this->type = MemberType::ValueType;
    this->content.value = valueMember;
}

/**
 * \brief Build Member Type from mixin type
 *
 * \param mixin Mixin which was given
 */
void MemberType::build(const Mixin& mixin)
{
    this->type = MemberType::MixinType;
    this->content.mixin = mixin;
}

/**
 * \brief Build Member Type from one of type
 *
 * \param oneOf One of which was given
 */
void MemberType::build(const OneOf& oneOf)
{
    this->type = MemberType::OneOfType;
    this->content.oneOf = oneOf;
}

/**
 * \brief Build Member Type from members type
 *
 * \param members List of Member types for members collection
 */
void MemberType::build(const MemberTypes& members)
{
    this->type = MemberType::MembersType;
    this->content.members = members;
}

/**
 * \brief Build Member Type from a value
 *
 * \param value Value of the value member
 */
void MemberType::build(const Value& value)
{
    ValueMember valueMember;

    valueMember.valueDefinition.values.push_back(value);
    this->build(valueMember);
}
