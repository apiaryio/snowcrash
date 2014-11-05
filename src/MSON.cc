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

TypeSectionContent::TypeSectionContent(const Markdown& description_)
: description(description_)
{
    m_members.reset(::new MemberTypes);
}

TypeSectionContent::TypeSectionContent(const TypeSectionContent& rhs)
{
    this->description = rhs.description;
    m_members.reset(::new MemberTypes(*rhs.m_members.get()));
}

TypeSectionContent& TypeSectionContent::operator=(const TypeSectionContent& rhs)
{
    this->description = rhs.description;
    m_members.reset(::new MemberTypes(*rhs.m_members.get()));

    return *this;
}

TypeSectionContent::~TypeSectionContent()
{
}

MemberTypes& OneOf::members()
{
    if (!m_members.get())
        throw "no members set";

    return *m_members;
}

const MemberTypes& OneOf::members() const
{
    if (!m_members.get())
        throw "no members set";

    return *m_members;
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
