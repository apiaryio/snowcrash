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
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember, ExportSourcemapOption);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description == "A color");
    REQUIRE(valueMember.node.valueDefinition.values.size() == 1);
    REQUIRE(valueMember.node.valueDefinition.values[0].literal == "red");
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.typeSpecification.name.base == mson::StringTypeName);
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.typeSpecification.name.symbol.empty());
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.typeSpecification.nestedTypes.empty());
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.attributes == mson::RequiredTypeAttribute);

    SourceMapHelper::check(valueMember.sourceMap.valueDefinition.sourceMap, 0, 35);
    SourceMapHelper::check(valueMember.sourceMap.description.sourceMap, 0, 35);
}

TEST_CASE("Parse mson value member with description not on new line", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- red (string, required) - A color\n"\
    "  Which is also very nice\n\n";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember, ExportSourcemapOption);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description == "A color");
    REQUIRE(valueMember.node.valueDefinition.values.size() == 1);
    REQUIRE(valueMember.node.sections.size() == 1);
    REQUIRE(valueMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(valueMember.node.sections[0].content.description == "Which is also very nice\n");
    REQUIRE(valueMember.node.sections[0].content.elements().empty());

    SourceMapHelper::check(valueMember.sourceMap.valueDefinition.sourceMap, 0, 62);
    SourceMapHelper::check(valueMember.sourceMap.description.sourceMap, 0, 62);
    REQUIRE(valueMember.sourceMap.sections.collection.size() == 1);
    SourceMapHelper::check(valueMember.sourceMap.sections.collection[0].description.sourceMap, 0, 62);
}

TEST_CASE("Parse mson value member with block description", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- red (string, required) - A color\n\n"\
    "    Which is also very nice\n\n"\
    "    - and awesome\n\n"\
    "and really really nice\n\n";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember, ExportSourcemapOption);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description == "A color");
    REQUIRE(valueMember.node.valueDefinition.values.size() == 1);
    REQUIRE(valueMember.node.valueDefinition.typeDefinition.attributes == mson::RequiredTypeAttribute);
    REQUIRE(valueMember.node.sections.size() == 1);
    REQUIRE(valueMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(valueMember.node.sections[0].content.description == "Which is also very nice\n\n- and awesome\n");

    SourceMapHelper::check(valueMember.sourceMap.valueDefinition.sourceMap, 2, 34);
    SourceMapHelper::check(valueMember.sourceMap.description.sourceMap, 2, 34);
    REQUIRE(valueMember.sourceMap.sections.collection.size() == 1);
    SourceMapHelper::check(valueMember.sourceMap.sections.collection[0].description.sourceMap, 40, 24, 69, 14);
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
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember, ExportSourcemapOption);

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

    SourceMapHelper::check(valueMember.sourceMap.valueDefinition.sourceMap, 2, 24);
    SourceMapHelper::check(valueMember.sourceMap.description.sourceMap, 2, 24);
    REQUIRE(valueMember.sourceMap.sections.collection.size() == 3);
    SourceMapHelper::check(valueMember.sourceMap.sections.collection[0].description.sourceMap, 30, 24, 59, 14);
    SourceMapHelper::check(valueMember.sourceMap.sections.collection[1].value.sourceMap, 80, 16);
    SourceMapHelper::check(valueMember.sourceMap.sections.collection[2].value.sourceMap, 118, 6);
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
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember, ExportSourcemapOption);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description == "List of colors");
    REQUIRE(valueMember.node.valueDefinition.values.empty());
    REQUIRE(valueMember.node.sections.size() == 2);
    REQUIRE(valueMember.node.sections[0].klass == mson::TypeSection::BlockDescriptionClass);
    REQUIRE(valueMember.node.sections[0].content.description == "Lots and lots of them\n");
    REQUIRE(valueMember.node.sections[1].klass == mson::TypeSection::SampleClass);
    REQUIRE(valueMember.node.sections[1].content.elements().size() == 4);

    SourceMapHelper::check(valueMember.sourceMap.valueDefinition.sourceMap, 2, 26);
    SourceMapHelper::check(valueMember.sourceMap.description.sourceMap, 2, 26);
    REQUIRE(valueMember.sourceMap.sections.collection.size() == 2);
    SourceMapHelper::check(valueMember.sourceMap.sections.collection[0].description.sourceMap, 32, 22);
    REQUIRE(valueMember.sourceMap.sections.collection[1].elements().collection.size() == 4);
}

TEST_CASE("Parse mson value member with multiple values", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- 1, yellow, true";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember, ExportSourcemapOption);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.empty());

    REQUIRE(valueMember.node.description.empty());
    REQUIRE(valueMember.node.sections.empty());
    REQUIRE(valueMember.node.valueDefinition.values.size() == 3);
    REQUIRE(valueMember.node.valueDefinition.values[0].literal == "1");
    REQUIRE(valueMember.node.valueDefinition.values[1].literal == "yellow");
    REQUIRE(valueMember.node.valueDefinition.values[2].literal == "true");

    SourceMapHelper::check(valueMember.sourceMap.valueDefinition.sourceMap, 0, 18);
    REQUIRE(valueMember.sourceMap.description.sourceMap.empty());
    REQUIRE(valueMember.sourceMap.sections.collection.empty());
}

TEST_CASE("Parse mson value member array with items", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- (array)\n"\
    "    - red (string) - A sample value\n"\
    "    - green (string)";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember, ExportSourcemapOption);

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

    SourceMapHelper::check(valueMember.sourceMap.valueDefinition.sourceMap, 2, 8);
    REQUIRE(valueMember.sourceMap.sections.collection.size() == 1);
    REQUIRE(valueMember.sourceMap.sections.collection[0].elements().collection.size() == 2);

    SourceMap<mson::ValueMember> valueMemberSM;

    valueMemberSM = valueMember.sourceMap.sections.collection[0].elements().collection[0].value;
    SourceMapHelper::check(valueMemberSM.valueDefinition.sourceMap, 16, 30);
    SourceMapHelper::check(valueMemberSM.description.sourceMap, 16, 30);

    valueMemberSM = valueMember.sourceMap.sections.collection[0].elements().collection[1].value;
    SourceMapHelper::check(valueMemberSM.valueDefinition.sourceMap, 50, 17);
}

TEST_CASE("Check warnings for object in array with defined value", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- (array)\n"\
    "    - explicit (object)\n"\
    "    - implicit\n"\
    "        - k: v\n";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember, ExportSourcemapOption);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.size() == 2);

}

TEST_CASE("Check warnings for object with defined value", "[mson][value_member]")
{
    mdp::ByteBuffer source = \
    "- (object)\n"\
    "    - k: v (object)\n";

    ParseResult<mson::ValueMember> valueMember;
    SectionParserHelper<mson::ValueMember, MSONValueMemberParser>::parse(source, MSONValueMemberSectionType, valueMember, ExportSourcemapOption);

    REQUIRE(valueMember.report.error.code == Error::OK);
    REQUIRE(valueMember.report.warnings.size() == 1);

}
