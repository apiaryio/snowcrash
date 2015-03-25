//
//  MSONSourcemap.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 03/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "MSONSourcemap.h"

using namespace snowcrash;

bool SourceMap<mson::NamedType>::empty() const
{
    return (name.sourceMap.empty() &&
            typeDefinition.sourceMap.empty() &&
            sections.collection.empty());
}

bool SourceMap<mson::ValueMember>::empty() const
{
    return (description.sourceMap.empty() &&
            valueDefinition.sourceMap.empty() &&
            sections.collection.empty());
}

bool SourceMap<mson::PropertyMember>::empty() const
{
    return (name.sourceMap.empty() &&
            description.sourceMap.empty() &&
            valueDefinition.sourceMap.empty() &&
            sections.collection.empty());
}

SourceMap<mson::Elements>& SourceMap<mson::TypeSection>::elements()
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
}

const SourceMap<mson::Elements>& SourceMap<mson::TypeSection>::elements() const
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
}

SourceMap<mson::TypeSection>::SourceMap(const SourceMap<mson::Markdown>& description_, const SourceMap<mson::Literal>& value_)
: description(description_), value(value_)
{
    m_elements.reset(::new SourceMap<mson::Elements>);
}

SourceMap<mson::TypeSection>::SourceMap(const SourceMap<mson::TypeSection>& rhs)
{
    this->description = rhs.description;
    this->value = rhs.value;
    m_elements.reset(::new SourceMap<mson::Elements>(*rhs.m_elements.get()));
}

SourceMap<mson::TypeSection>& SourceMap<mson::TypeSection>::operator=(const SourceMap<mson::TypeSection>& rhs)
{
    this->description = rhs.description;
    this->value = rhs.value;
    m_elements.reset(::new SourceMap<mson::Elements>(*rhs.m_elements.get()));

    return *this;
}

SourceMap<mson::TypeSection>::~SourceMap()
{
}

SourceMap<mson::OneOf>& SourceMap<mson::Element>::oneOf()
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
}

const SourceMap<mson::OneOf>& SourceMap<mson::Element>::oneOf() const
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
}

SourceMap<mson::Elements>& SourceMap<mson::Element>::elements()
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
}

const SourceMap<mson::Elements>& SourceMap<mson::Element>::elements() const
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
}

SourceMap<mson::Element>& SourceMap<mson::Element>::operator=(const SourceMap<mson::Elements> &rhs)
{
    m_elements.reset(::new SourceMap<mson::Elements>(rhs));

    return *this;
}

SourceMap<mson::Element>::SourceMap()
{
    m_elements.reset(::new SourceMap<mson::Elements>);
}

SourceMap<mson::Element>::SourceMap(const SourceMap<mson::Element>& rhs)
{
    this->property = rhs.property;
    this->value = rhs.value;
    this->mixin = rhs.mixin;
    m_elements.reset(::new SourceMap<mson::Elements>(*rhs.m_elements.get()));
}

SourceMap<mson::Element>& SourceMap<mson::Element>::operator=(const SourceMap<mson::Element>& rhs)
{
    this->property = rhs.property;
    this->value = rhs.value;
    this->mixin = rhs.mixin;
    m_elements.reset(::new SourceMap<mson::Elements>(*rhs.m_elements.get()));

    return *this;
}

SourceMap<mson::Element>::~SourceMap()
{
}
