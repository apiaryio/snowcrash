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
    return (this->base == UndefinedTypeName && this->symbol.empty());
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

Elements& TypeSection::Content::elements()
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
}

const Elements& TypeSection::Content::elements() const
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
}

TypeSection::Content::Content(const Markdown& description_, const Literal& value_)
: description(description_), value(value_)
{
    m_elements.reset(::new Elements);
}

TypeSection::Content::Content(const TypeSection::Content& rhs)
{
    this->description = rhs.description;
    this->value = rhs.value;
    m_elements.reset(::new Elements(*rhs.m_elements.get()));
}

TypeSection::Content& TypeSection::Content::operator=(const TypeSection::Content& rhs)
{
    this->description = rhs.description;
    this->value = rhs.value;
    m_elements.reset(::new Elements(*rhs.m_elements.get()));

    return *this;
}

TypeSection::Content::~Content()
{
}

bool TypeSection::empty() const
{
    return (this->klass == TypeSection::UndefinedClass &&
            this->content.value.empty() &&
            this->content.description.empty() &&
            this->content.elements().empty());
}

bool NamedType::empty() const
{
    return (this->typeDefinition.empty() &&
            this->name.empty() &&
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

OneOf& Element::Content::oneOf()
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
}

const OneOf& Element::Content::oneOf() const
{
    if (!m_elements.get())
        throw ELEMENTS_NOT_SET_ERR;

    return *m_elements;
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

Element::Content& Element::Content::operator=(const Elements& rhs)
{
    m_elements.reset(::new Elements(rhs));

    return *this;
}

Element::Content::Content()
{
    m_elements.reset(::new Elements);
}

Element::Content::Content(const Element::Content& rhs)
{
    this->property = rhs.property;
    this->value = rhs.value;
    this->mixin = rhs.mixin;
    m_elements.reset(::new Elements(*rhs.m_elements.get()));
}

Element::Content& Element::Content::operator=(const Element::Content& rhs)
{
    this->property = rhs.property;
    this->value = rhs.value;
    this->mixin = rhs.mixin;
    m_elements.reset(::new Elements(*rhs.m_elements.get()));

    return *this;
}

Element::Content::~Content()
{
}

Element::Element(const Element::Class& klass_)
: klass(klass_)
{
}

Element::Element(const Element& rhs)
{
    this->klass = rhs.klass;
    this->content = rhs.content;
}

Element& Element::operator=(const Element& rhs)
{
    this->klass = rhs.klass;
    this->content = rhs.content;

    return *this;
}

Element::~Element()
{
}

/**
 * \brief Build Element from property member
 *
 * \param propertyMember Property member which was given
 */
void Element::build(const PropertyMember& propertyMember)
{
    this->klass = Element::PropertyClass;
    this->content.property = propertyMember;
}

/**
 * \brief Build Element from value member
 *
 * \param valueMember Value member which was given
 */
void Element::build(const ValueMember& valueMember)
{
    this->klass = Element::ValueClass;
    this->content.value = valueMember;
}

/**
 * \brief Build Element from mixin type
 *
 * \param mixin Mixin which was given
 */
void Element::build(const Mixin& mixin)
{
    this->klass = Element::MixinClass;
    this->content.mixin = mixin;
}

/**
 * \brief Build Element from one of type
 *
 * \param oneOf One Of which was given
 */
void Element::build(const OneOf& oneOf)
{
    this->buildFromElements(oneOf);
    this->klass = Element::OneOfClass;
}

/**
 * \brief Build Element from a value
 *
 * \param value Value of the value member
 */
void Element::build(const Value& value)
{
    ValueMember valueMember;

    valueMember.valueDefinition.values.push_back(value);
    this->build(valueMember);
}

/**
 * \brief Buile Element from group of elements
 *
 * \param elements Group of elements
 */
void Element::buildFromElements(const Elements &elements)
{
    this->klass = Element::GroupClass;
    this->content = elements;
}
