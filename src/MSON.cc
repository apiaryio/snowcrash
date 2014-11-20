//
//  MSON.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/4/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "MSON.h"

using namespace mson;

MemberTypes& TypeSectionContent::members()
{
    if (!m_members.get())
        throw MEMBERS_NOT_SET_ERR;

    return *m_members;
}

const MemberTypes& TypeSectionContent::members() const
{
    if (!m_members.get())
        throw MEMBERS_NOT_SET_ERR;

    return *m_members;
}

TypeSectionContent::TypeSectionContent(const Markdown& description_, const Literal& value_)
: description(description_), value(value_)
{
    m_members.reset(::new MemberTypes);
}

TypeSectionContent::TypeSectionContent(const TypeSectionContent& rhs)
{
    this->description = rhs.description;
    this->value = rhs.value;
    m_members.reset(::new MemberTypes(*rhs.m_members.get()));
}

TypeSectionContent& TypeSectionContent::operator=(const TypeSectionContent& rhs)
{
    this->description = rhs.description;
    this->value = rhs.value;
    m_members.reset(::new MemberTypes(*rhs.m_members.get()));

    return *this;
}

TypeSectionContent::~TypeSectionContent()
{
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

OneOf::OneOf()
{
    m_members.reset(::new MemberTypes);
}

OneOf::OneOf(const OneOf& rhs)
{
    m_members.reset(::new MemberTypes(*rhs.m_members.get()));
}

OneOf& OneOf::operator=(const OneOf& rhs)
{
    m_members.reset(::new MemberTypes(*rhs.m_members.get()));

    return *this;
}

OneOf::~OneOf()
{
}

MemberType::MemberType(const MemberTypeType& type_)
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
    this->type = PropertyMemberType;
    this->content.property = propertyMember;
}

/**
 * \brief Build Member Type from value member
 *
 * \param valueMember Value member which was given
 */
void MemberType::build(const ValueMember& valueMember)
{
    this->type = ValueMemberType;
    this->content.value = valueMember;
}

/**
 * \brief Build Member Type from mixin type
 *
 * \param mixin Mixin which was given
 */
void MemberType::build(const Mixin& mixin)
{
    this->type = MixinMemberType;
    this->content.mixin = mixin;
}

/**
 * \brief Build Member Type from one of type
 *
 * \param oneOf One of which was given
 */
void MemberType::build(const OneOf& oneOf)
{
    this->type = OneOfMemberType;
    this->content.oneOf = oneOf;
}

/**
 * \brief Build Member Type from members type
 *
 * \param members List of Member types for members collection
 */
void MemberType::build(const MemberTypes& members)
{
    this->type = MembersMemberType;
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
