//
//  test-MSONValueMemberParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/22/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONValueMemberParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Parse canonical mson value member", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- red (string, required) - A color";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description == "A color");
    REQUIRE(valueMember.node.valueDefinition.values.size() == 1);
    REQUIRE(valueMember.node.valueDefinition.values[0].literal == "red");
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.typeSpecification.name.base == mson::StringTypeName);
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol.empty());
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.typeSpecification.nestedTypes.empty());
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.attributes == mson::RequiredTypeAttribute);
}

TEST_CASE("Parse mson value member with description not on new line", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- red (string, required) - A color\n"\
    "  Which is also very nice\n\n";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description == "A color");
    REQUIRE(valueMember.node.valueDefinition.values.size() == 1);
    REQUIRE(valueMember.node.sections.size() == 1);
    REQUIRE(valueMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(valueMember.node.sections[0].content.description == "Which is also very nice\n");
    REQUIRE(valueMember.node.sections[0].content.elements().empty());
}

TEST_CASE("Parse mson value member with block description", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- red (string, required) - A color\n\n"\
    "    Which is also very nice\n\n"\
    "    - and awesome\n\n"\
    "and really really nice\n\n";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description == "A color");
    REQUIRE(valueMember.node.valueDefinition.values.size() == 1);
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.attributes == mson::RequiredTypeAttribute);
    REQUIRE(valueMember.node.sections.size() == 1);
    REQUIRE(valueMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(valueMember.node.sections[0].content.description == "Which is also very nice\n\n- and awesome\n");
}

TEST_CASE("Parse mson value member with block description, default and sample", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- red (string) - A color\n\n"\
    "    Which is also very nice\n\n"\
    "    - and awesome\n\n"\
    "    - Default: yellow\n"\
    "    - Sample\n\n"\
    "        green\n";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description == "A color");
    REQUIRE(valueMember.node.valueDefinition.values.size() == 1);
    REQUIRE(valueMember.node.valueDefinition.values[0].literal == "red");
    REQUIRE(valueMember.node.sections.size() == 3);
    REQUIRE(valueMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(valueMember.node.sections[0].content.description == "Which is also very nice\n\n- and awesome\n");
    REQUIRE(valueMember.node.sections[1].klass == mson::TypeSection::DefaultClass);
    REQUIRE(valueMember.node.sections[1].content.value == "yellow");
    REQUIRE(valueMember.node.sections[2].klass == mson::TypeSection::SampleClass);
    REQUIRE(valueMember.node.sections[2].content.value == "green\n");
}

TEST_CASE("Parse mson value member array with sample", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- (array) - List of colors\n\n"\
    "    Lots and lots of them\n\n"\
    "    - Sample\n"\
    "        - green\n"\
    "        - red\n"\
    "        - blue\n"\
    "        - yellow\n";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description == "List of colors");
    REQUIRE(valueMember.node.valueDefinition.values.empty());
    REQUIRE(valueMember.node.sections.size() == 2);
    REQUIRE(valueMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(valueMember.node.sections[0].content.description == "Lots and lots of them\n");
    REQUIRE(valueMember.node.sections[1].klass == mson::TypeSection::SampleClass);
    REQUIRE(valueMember.node.sections[1].content.elements().size() == 4);
}

TEST_CASE("Parse mson value member with multiple values", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- 1, yellow, true";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description.empty());
    REQUIRE(valueMember.node.sections.empty());
    REQUIRE(valueMember.node.valueDefinition.values.size() == 3);
    REQUIRE(valueMember.node.valueDefinition.values[0].literal == "1");
    REQUIRE(valueMember.node.valueDefinition.values[1].literal == "yellow");
    REQUIRE(valueMember.node.valueDefinition.values[2].literal == "true");
}

TEST_CASE("Parse mson value member array with items", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- (array)\n"\
    "    - red (string) - A sample value\n"\
    "    - green (string)";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description.empty());
    REQUIRE(valueMember.node.valueDefinition.values.empty());
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.typeSpecification.name.base == mson::ArrayTypeName);
    REQUIRE(valueMember.node.sections.size() == 1);

    REQUIRE(valueMember.node.sections[0].content.description.empty());
    REQUIRE(valueMember.node.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(valueMember.node.sections[0].content.elements().size() == 2);
    REQUIRE(valueMember.node.sections[0].content.elements().at(0).klass == mson::Element::ValueClass);
    REQUIRE(valueMember.node.sections[0].content.elements().at(0).content.value.valueDefinition.values.size() == 1);
    REQUIRE(valueMember.node.sections[0].content.elements().at(0).content.value.description == "A sample value");
    REQUIRE(valueMember.node.sections[0].content.elements().at(1).klass == mson::Element::ValueClass);
    REQUIRE(valueMember.node.sections[0].content.elements().at(1).content.value.valueDefinition.values.size() == 1);
}
