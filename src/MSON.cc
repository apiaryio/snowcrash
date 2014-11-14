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
        throw "no members set";

    return *m_members;
}

const MemberTypes& TypeSectionContent::members() const
{
    if (!m_members.get())
        throw "no members set";

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
        throw "no members set";

    return *m_members;
}

const MemberTypes& Members::members() const
{
    if (!m_members.get())
        throw "no members set";

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
