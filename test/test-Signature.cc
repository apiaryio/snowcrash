//
//  test-Signature.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/15/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"

static const mdp::ByteBuffer PropertySignatureFixture = "id: 42 (yes, no) - a good message";
static const mdp::ByteBuffer EscapedPropertySignatureFixture = "`*id*(data):3`: `42` (yes, no) - a good message";

static const mdp::ByteBuffer ElementSignatureFixture = "42 (number) - a good number";
static const mdp::ByteBuffer EscapedElementSignatureFixture = "`*42*(data):3` (number) - a good number";

using namespace snowcrash;
using namespace snowcrashtest;

static const scpl::SignatureTraits::Traits PropertyMemberTypeTraits = scpl::SignatureTraits::IdentifierTrait |
                                                                      scpl::SignatureTraits::ValuesTrait |
                                                                      scpl::SignatureTraits::AttributesTrait |
                                                                      scpl::SignatureTraits::ContentTrait;

static const scpl::SignatureTraits::Traits ElementMemberTypeTraits = scpl::SignatureTraits::ValuesTrait |
                                                                     scpl::SignatureTraits::AttributesTrait |
                                                                     scpl::SignatureTraits::ContentTrait;

TEST_CASE("Property signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse(PropertySignatureFixture, blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "id");
    REQUIRE(signature.value == "42");
    REQUIRE(signature.values.size() == 1);
    REQUIRE(signature.values[0] == "42");
    REQUIRE(signature.attributes.size() == 2);
    REQUIRE(signature.attributes[0] == "yes");
    REQUIRE(signature.attributes[1] == "no");
    REQUIRE(signature.content == "a good message");
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Escaped property signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse(EscapedPropertySignatureFixture, blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "*id*(data):3");
    REQUIRE(signature.value == "42");
    REQUIRE(signature.values.size() == 1);
    REQUIRE(signature.values[0] == "`42`");
    REQUIRE(signature.attributes.size() == 2);
    REQUIRE(signature.attributes[0] == "yes");
    REQUIRE(signature.attributes[1] == "no");
    REQUIRE(signature.content == "a good message");
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Multiline signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("id\nLine 2\nLine 3\n", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "id");
    REQUIRE(signature.value.empty());
    REQUIRE(signature.values.empty());
    REQUIRE(signature.attributes.empty());
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent == "Line 2\nLine 3");
}

TEST_CASE("Identifier only signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("id", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "id");
    REQUIRE(signature.value.empty());
    REQUIRE(signature.values.empty());
    REQUIRE(signature.attributes.empty());
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Identifier enclosed by asterisk", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("*rel (Custom String)* (object)", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "*rel (Custom String)*");
    REQUIRE(signature.value.empty());
    REQUIRE(signature.values.empty());
    REQUIRE(signature.attributes.size() == 1);
    REQUIRE(signature.attributes[0] == "object");
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Identifier enclosed by underscore", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("_rel (*Custom* String)_ (object)", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "_rel (*Custom* String)_");
    REQUIRE(signature.value.empty());
    REQUIRE(signature.values.empty());
    REQUIRE(signature.attributes.size() == 1);
    REQUIRE(signature.attributes[0] == "object");
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Identifier enclosed by backticks", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("```username `is` g``ood``` (object)", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "username `is` g``ood");
    REQUIRE(signature.value.empty());
    REQUIRE(signature.values.empty());
    REQUIRE(signature.attributes.size() == 1);
    REQUIRE(signature.attributes[0] == "object");
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Identifier enclosing not completed", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("_rel (object)", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "_rel");
    REQUIRE(signature.value.empty());
    REQUIRE(signature.values.empty());
    REQUIRE(signature.attributes.size() == 1);
    REQUIRE(signature.attributes[0] == "object");
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Extra space content after identifier enclosure", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("`a`   : 42", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "a");
    REQUIRE(signature.value == "42");
    REQUIRE(signature.values.size() == 1);
    REQUIRE(signature.values[0] == "42");
    REQUIRE(signature.attributes.empty());
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Extra non-space content after identifier enclosure", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("`a`b : 42", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 0);

    REQUIRE(signature.identifier == "`a`b");
    REQUIRE(signature.value == "42");
    REQUIRE(signature.values.size() == 1);
    REQUIRE(signature.values[0] == "42");
    REQUIRE(signature.attributes.empty());
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Identifier description signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("id - a good - info", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "id");
    REQUIRE(signature.value.empty());
    REQUIRE(signature.values.empty());
    REQUIRE(signature.attributes.empty());
    REQUIRE(signature.content == "a good - info");
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Identifier value signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("id : a good data", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "id");
    REQUIRE(signature.value == "a good data");
    REQUIRE(signature.values.size() == 1);
    REQUIRE(signature.values[0] == "a good data");
    REQUIRE(signature.attributes.empty());
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Identifier attributes signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("id (number)", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "id");
    REQUIRE(signature.value.empty());
    REQUIRE(signature.values.empty());
    REQUIRE(signature.attributes.size() == 1);
    REQUIRE(signature.attributes[0] == "number");
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Element signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse(ElementSignatureFixture, blueprint, ElementMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier.empty());
    REQUIRE(signature.value == "42");
    REQUIRE(signature.values.size() == 1);
    REQUIRE(signature.values[0] == "42");
    REQUIRE(signature.attributes.size() == 1);
    REQUIRE(signature.attributes[0] == "number");
    REQUIRE(signature.content == "a good number");
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Escaped element signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse(EscapedElementSignatureFixture, blueprint, ElementMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier.empty());
    REQUIRE(signature.value == "*42*(data):3");
    REQUIRE(signature.values.size() == 1);
    REQUIRE(signature.values[0] == "`*42*(data):3`");
    REQUIRE(signature.attributes.size() == 1);
    REQUIRE(signature.attributes[0] == "number");
    REQUIRE(signature.content == "a good number");
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Element value signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("42", blueprint, ElementMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier.empty());
    REQUIRE(signature.value == "42");
    REQUIRE(signature.values.size() == 1);
    REQUIRE(signature.values[0] == "42");
    REQUIRE(signature.attributes.empty());
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Element attributes signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("(number)", blueprint, ElementMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier.empty());
    REQUIRE(signature.value.empty());
    REQUIRE(signature.values.empty());
    REQUIRE(signature.attributes.size() == 1);
    REQUIRE(signature.attributes[0] == "number");
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Element attributes description signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("(number) - something () cool", blueprint, ElementMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier.empty());
    REQUIRE(signature.value.empty());
    REQUIRE(signature.values.empty());
    REQUIRE(signature.attributes.size() == 1);
    REQUIRE(signature.attributes[0] == "number");
    REQUIRE(signature.content == "something () cool");
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Property signature parsing with element traits", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse(PropertySignatureFixture, blueprint, ElementMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier.empty());
    REQUIRE(signature.value == "id: 42");
    REQUIRE(signature.values.size() == 1);
    REQUIRE(signature.values[0] == "id: 42");
    REQUIRE(signature.attributes.size() == 2);
    REQUIRE(signature.attributes[0] == "yes");
    REQUIRE(signature.attributes[1] == "no");
    REQUIRE(signature.content == "a good message");
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Property signature parsing without identifier", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("(x)", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);

    REQUIRE(signature.identifier.empty());
    REQUIRE(signature.value.empty());
    REQUIRE(signature.values.empty());
    REQUIRE(signature.attributes.size() == 1);
    REQUIRE(signature.attributes[0] == "x");
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Property signature parsing without a value", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("id: (number)", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);

    REQUIRE(signature.identifier == "id");
    REQUIRE(signature.value.empty());
    REQUIRE(signature.values.empty());
    REQUIRE(signature.attributes.size() == 1);
    REQUIRE(signature.attributes[0] == "number");
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Element signature parsing without value and attributes", "[signature]")
{
    mdp::MarkdownNode source(mdp::RootMarkdownNodeType, NULL, "");
    mdp::MarkdownNode paragraph(mdp::ParagraphMarkdownNodeType, &source, "- content is the king");

    source.children().push_back(paragraph);

    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("", blueprint, ElementMemberTypeTraits, &source);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);

    REQUIRE(signature.identifier.empty());
    REQUIRE(signature.value.empty());
    REQUIRE(signature.values.size() == 0);
    REQUIRE(signature.attributes.empty());
    REQUIRE(signature.content == "content is the king");
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Escaped array element signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;

    scpl::Signature signature = SignatureParserHelper::parse("`1 `, 00 ``2, `3` da(t)a`` 45 ", blueprint, ElementMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier.empty());
    REQUIRE(signature.value == "`1 `, 00 ``2, `3` da(t)a`` 45");
    REQUIRE(signature.values.size() == 2);
    REQUIRE(signature.values[0] == "1");
    REQUIRE(signature.values[1] == "00 ``2, `3` da(t)a`` 45");
    REQUIRE(signature.attributes.empty());
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Unescaped array element signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("1 , 2,3 (optional, array) - numbers", blueprint, ElementMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier.empty());
    REQUIRE(signature.value == "1 , 2,3");
    REQUIRE(signature.values.size() == 3);
    REQUIRE(signature.values[0] == "1");
    REQUIRE(signature.values[1] == "2");
    REQUIRE(signature.values[2] == "3");
    REQUIRE(signature.attributes.size() == 2);
    REQUIRE(signature.attributes[0] == "optional");
    REQUIRE(signature.attributes[1] == "array");
    REQUIRE(signature.content == "numbers");
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Escaped attributes signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("1 (`optio)nal, array` , fixed)", blueprint, ElementMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier.empty());
    REQUIRE(signature.value == "1");
    REQUIRE(signature.values.size() == 1);
    REQUIRE(signature.values[0] == "1");
    REQUIRE(signature.attributes.size() == 2);
    REQUIRE(signature.attributes[0] == "`optio)nal, array`");
    REQUIRE(signature.attributes[1] == "fixed");
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Attributes with many brackets signature parsing", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("1 ([op(t[io]na)l, p][], [A](http://a.com))", blueprint, ElementMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier.empty());
    REQUIRE(signature.value == "1");
    REQUIRE(signature.values.size() == 1);
    REQUIRE(signature.values[0] == "1");
    REQUIRE(signature.attributes.size() == 2);
    REQUIRE(signature.attributes[0] == "[op(t[io]na)l, p][]");
    REQUIRE(signature.attributes[1] == "[A](http://a.com)");
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Identifier and values only", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::SignatureTraits::Traits traits = scpl::SignatureTraits::IdentifierTrait | scpl::SignatureTraits::ValuesTrait;
    scpl::Signature signature = SignatureParserHelper::parse("Sample: 10 (1), 20", blueprint, traits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "Sample");
    REQUIRE(signature.value == "10 (1), 20");
    REQUIRE(signature.values.size() == 2);
    REQUIRE(signature.values[0] == "10 (1)");
    REQUIRE(signature.values[1] == "20");
    REQUIRE(signature.attributes.empty());
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}

TEST_CASE("Identifier with an underscore", "[signature]")
{
    ParseResult<Blueprint> blueprint;
    scpl::Signature signature = SignatureParserHelper::parse("site_admin: false (boolean, default)", blueprint, PropertyMemberTypeTraits);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(signature.identifier == "site_admin");
    REQUIRE(signature.value == "false");
    REQUIRE(signature.values.size() == 1);
    REQUIRE(signature.values[0] == "false");
    REQUIRE(signature.attributes.size() == 2);
    REQUIRE(signature.attributes[0] == "boolean");
    REQUIRE(signature.attributes[1] == "default");
    REQUIRE(signature.content.empty());
    REQUIRE(signature.remainingContent.empty());
}
