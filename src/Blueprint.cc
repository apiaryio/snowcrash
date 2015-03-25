//
//  Blueprint.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 26/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "Blueprint.h"

using namespace snowcrash;

DataStructure& DataStructure::operator=(const mson::NamedType &rhs)
{
    this->name = rhs.name;
    this->typeDefinition = rhs.typeDefinition;
    this->sections = rhs.sections;

    return *this;
}

Elements& Element::Content::elements()
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
}

const Elements& Element::Content::elements() const
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
}

Element::Content::Content()
{
    m_elements.reset(::new Elements);
}

Element::Content::Content(const Element::Content& rhs)
{
    this->copy = rhs.copy;
    this->resource = rhs.resource;
    this->dataStructure = rhs.dataStructure;
    m_elements.reset(::new Elements(*rhs.m_elements.get()));
}

Element::Content& Element::Content::operator=(const Element::Content& rhs)
{
    this->copy = rhs.copy;
    this->resource = rhs.resource;
    this->dataStructure = rhs.dataStructure;
    m_elements.reset(::new Elements(*rhs.m_elements.get()));

    return *this;
}

Element::Content::~Content()
{
}

Element::Element(const Element::Class& element_)
: element(element_)
{
}

Element::Element(const Element& rhs)
{
    this->element = rhs.element;
    this->attributes = rhs.attributes;
    this->content = rhs.content;
    this->category = rhs.category;
}

Element& Element::operator=(const Element& rhs)
{
    this->element = rhs.element;
    this->attributes = rhs.attributes;
    this->content = rhs.content;
    this->category = rhs.category;

    return *this;
}

Element::~Element()
{
}
