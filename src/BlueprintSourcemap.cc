//
//  BlueprintSourcemap.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 26/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "BlueprintSourcemap.h"

using namespace snowcrash;

SourceMap<Elements>& SourceMap<Element>::Content::elements()
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
}

const SourceMap<Elements>& SourceMap<Element>::Content::elements() const
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
}

SourceMap<Element>::Content::Content()
{
    m_elements.reset(::new SourceMap<Elements>);
}

SourceMap<Element>::Content::Content(const SourceMap<Element>::Content& rhs)
{
    this->copy = rhs.copy;
    this->resource = rhs.resource;
    this->dataStructure = rhs.dataStructure;
    m_elements.reset(::new SourceMap<Elements>(*rhs.m_elements.get()));
}

SourceMap<Element>::Content& SourceMap<Element>::Content::operator=(const SourceMap<Element>::Content& rhs)
{
    this->copy = rhs.copy;
    this->resource = rhs.resource;
    this->dataStructure = rhs.dataStructure;
    m_elements.reset(::new SourceMap<Elements>(*rhs.m_elements.get()));

    return *this;
}

SourceMap<Element>::Content::~Content()
{
}

SourceMap<Element>::SourceMap(const Element::Class& element_)
: element(element_)
{
}

SourceMap<Element>::SourceMap(const SourceMap<Element>& rhs)
{
    this->element = rhs.element;
    this->attributes = rhs.attributes;
    this->content = rhs.content;
    this->category = rhs.category;
}

SourceMap<Element>& SourceMap<Element>::operator=(const SourceMap<Element>& rhs)
{
    this->element = rhs.element;
    this->attributes = rhs.attributes;
    this->content = rhs.content;
    this->category = rhs.category;

    return *this;
}

SourceMap<Element>::~SourceMap()
{
}
