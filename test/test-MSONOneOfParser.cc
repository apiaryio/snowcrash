//
//  test-MSONOneOfParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONOneOfParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("OneOf block classifier", "[mson][one_of]")
{
    mdp::ByteBuffer source = \
    "- one Of";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(markdownAST.children().front().type == mdp::ListItemMarkdownNodeType);
    REQUIRE(!markdownAST.children().front().children().empty());

    sectionType = SectionProcessor<mson::OneOf>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONOneOfSectionType);

    markdownAST.children().front().children().front().text = "One of";
    sectionType = SectionProcessor<mson::OneOf>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONOneOfSectionType);
}

TEST_CASE("Parse canonical mson one of", "[mson][one_of]")
{
    mdp::ByteBuffer source = \
    "- One of\n"\
    "  - state: Andhra Pradesh\n"\
    "  - province: Madras";

    ParseResult<mson::OneOf> oneOf;
    SectionParserHelper<mson::OneOf, MSONOneOfParser>::parseMSON(source, MSONOneOfSectionType, oneOf);

    REQUIRE(oneOf.report.error.code == Error::OK);
    REQUIRE(oneOf.report.warnings.empty());

    REQUIRE(oneOf.node.members().size() == 2);

    REQUIRE(oneOf.node.members().at(0).type == mson::MemberType::PropertyType);
    REQUIRE(oneOf.node.members().at(0).content.property.name.literal == "state");
    REQUIRE(oneOf.node.members().at(0).content.property.valueDefinition.values.size() == 1);
    REQUIRE(oneOf.node.members().at(0).content.property.valueDefinition.values.at(0).literal == "Andhra Pradesh");
    REQUIRE(oneOf.node.members().at(0).content.property.sections.empty());
    REQUIRE(oneOf.node.members().at(0).content.property.description.empty());
    REQUIRE(oneOf.node.members().at(0).content.property.name.variable.empty());
    REQUIRE(oneOf.node.members().at(0).content.oneOf.empty());
    REQUIRE(oneOf.node.members().at(0).content.mixin.empty());
    REQUIRE(oneOf.node.members().at(0).content.value.empty());
    REQUIRE(oneOf.node.members().at(0).content.members.empty());

    REQUIRE(oneOf.node.members().at(1).type == mson::MemberType::PropertyType);
    REQUIRE(oneOf.node.members().at(1).content.property.name.literal == "province");
    REQUIRE(oneOf.node.members().at(1).content.property.valueDefinition.values.size() == 1);
    REQUIRE(oneOf.node.members().at(1).content.property.valueDefinition.values.at(0).literal == "Madras");
    REQUIRE(oneOf.node.members().at(1).content.property.sections.empty());
    REQUIRE(oneOf.node.members().at(1).content.property.description.empty());
    REQUIRE(oneOf.node.members().at(1).content.property.name.variable.empty());
}

TEST_CASE("Parse mson one of without any nested members", "[mson][one_of]")
{
    mdp::ByteBuffer source = "- One of\n";

    ParseResult<mson::OneOf> oneOf;
    SectionParserHelper<mson::OneOf, MSONOneOfParser>::parseMSON(source, MSONOneOfSectionType, oneOf);

    REQUIRE(oneOf.report.error.code == Error::OK);
    REQUIRE(oneOf.report.warnings.size() == 1);
    REQUIRE(oneOf.report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(oneOf.node.members().size() == 0);
}

TEST_CASE("Parse mson one of with one of", "[mson][one_of]")
{
    mdp::ByteBuffer source = \
    "- One of\n"\
    "    - last_name\n"\
    "    - one of\n"\
    "        - given_name\n"\
    "        - suffixed_name";

    ParseResult<mson::OneOf> oneOf;
    SectionParserHelper<mson::OneOf, MSONOneOfParser>::parseMSON(source, MSONOneOfSectionType, oneOf);

    REQUIRE(oneOf.report.error.code == Error::OK);
    REQUIRE(oneOf.report.warnings.empty());

    REQUIRE(oneOf.node.members().size() == 2);
    REQUIRE(oneOf.node.members().at(0).type == mson::MemberType::PropertyType);
    REQUIRE(oneOf.node.members().at(0).content.property.name.literal == "last_name");
    REQUIRE(oneOf.node.members().at(0).content.property.sections.empty());
    REQUIRE(oneOf.node.members().at(0).content.property.description.empty());
    REQUIRE(oneOf.node.members().at(0).content.property.valueDefinition.empty());

    REQUIRE(oneOf.node.members().at(1).type == mson::MemberType::OneOfType);
    REQUIRE(oneOf.node.members().at(1).content.oneOf.members().size() == 2);
    REQUIRE(oneOf.node.members().at(1).content.oneOf.members().at(0).type == mson::MemberType::PropertyType);
    REQUIRE(oneOf.node.members().at(1).content.oneOf.members().at(0).content.property.name.literal == "given_name");
    REQUIRE(oneOf.node.members().at(1).content.oneOf.members().at(0).content.property.valueDefinition.empty());
    REQUIRE(oneOf.node.members().at(1).content.oneOf.members().at(1).type == mson::MemberType::PropertyType);
    REQUIRE(oneOf.node.members().at(1).content.oneOf.members().at(1).content.property.name.literal == "suffixed_name");
    REQUIRE(oneOf.node.members().at(1).content.oneOf.members().at(1).content.property.valueDefinition.empty());
}

TEST_CASE("Parse mson one of with member group")
{
    mdp::ByteBuffer source = \
    "- One Of\n"\
    "    - full_name\n"\
    "    - Properties\n"\
    "        - first_name\n"\
    "        - last_name";

    ParseResult<mson::OneOf> oneOf;
    SectionParserHelper<mson::OneOf, MSONOneOfParser>::parseMSON(source, MSONOneOfSectionType, oneOf);

    REQUIRE(oneOf.report.error.code == Error::OK);
    REQUIRE(oneOf.report.warnings.empty());

    REQUIRE(oneOf.node.members().size() == 2);
    REQUIRE(oneOf.node.members().at(0).type == mson::MemberType::PropertyType);
    REQUIRE(oneOf.node.members().at(0).content.property.name.literal == "full_name");
    REQUIRE(oneOf.node.members().at(0).content.property.sections.empty());
    REQUIRE(oneOf.node.members().at(0).content.property.description.empty());
    REQUIRE(oneOf.node.members().at(0).content.property.valueDefinition.empty());
    REQUIRE(oneOf.node.members().at(1).type == mson::MemberType::MembersType);
    REQUIRE(oneOf.node.members().at(1).content.property.empty());
    REQUIRE(oneOf.node.members().at(1).content.mixin.empty());
    REQUIRE(oneOf.node.members().at(1).content.value.empty());
    REQUIRE(oneOf.node.members().at(1).content.oneOf.empty());
    REQUIRE(oneOf.node.members().at(1).content.members.members().size() == 2);
    REQUIRE(oneOf.node.members().at(1).content.members.members().at(0).type == mson::MemberType::PropertyType);
    REQUIRE(oneOf.node.members().at(1).content.members.members().at(0).content.property.name.literal == "first_name");
    REQUIRE(oneOf.node.members().at(1).content.members.members().at(1).type == mson::MemberType::PropertyType);
    REQUIRE(oneOf.node.members().at(1).content.members.members().at(1).content.property.name.literal == "last_name");
}
