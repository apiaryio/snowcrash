//
//  test-MSONTypeSectionParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/4/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONTypeSectionParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Type Section header block classifier", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "## Items";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(markdownAST.children().front().type == mdp::HeaderMarkdownNodeType);

    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONValueMembersSectionType);

    markdownAST.children().front().text = "Members";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONValueMembersSectionType);

    markdownAST.children().front().text = "Properties";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONPropertyMembersSectionType);

    markdownAST.children().front().text = "Default";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONSampleDefaultSectionType);

    markdownAST.children().front().text = "Sample";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONSampleDefaultSectionType);
}

TEST_CASE("Type Section list block classifier", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Items";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(markdownAST.children().front().type == mdp::ListItemMarkdownNodeType);
    REQUIRE(!markdownAST.children().front().children().empty());

    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONValueMembersSectionType);

    markdownAST.children().front().children().front().text = "Members";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONValueMembersSectionType);

    markdownAST.children().front().children().front().text = "Properties";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONPropertyMembersSectionType);

    markdownAST.children().front().children().front().text = "Default";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONSampleDefaultSectionType);

    markdownAST.children().front().children().front().text = "Default : 400";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONSampleDefaultSectionType);

    markdownAST.children().front().children().front().text = "Sample: red, green";
    sectionType = SectionProcessor<mson::TypeSection>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == MSONSampleDefaultSectionType);
}

TEST_CASE("Parse canonical mson sample list type section", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Sample: 75";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::PrimitiveBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value == "75");
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().empty());

    SourceMapHelper::check(typeSection.sourceMap.value.sourceMap, 0, 13);
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.empty());
}

TEST_CASE("Parse array mson sample list type section", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Sample: 75, 100";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::ValueBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value.empty());
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().size() == 2);
    REQUIRE(typeSection.node.content.elements().at(0).klass == mson::Element::ValueClass);
    REQUIRE(typeSection.node.content.elements().at(0).content.value.valueDefinition.values.size() == 1);
    REQUIRE(typeSection.node.content.elements().at(0).content.value.valueDefinition.values[0].literal == "75");
    REQUIRE(typeSection.node.content.elements().at(1).klass == mson::Element::ValueClass);
    REQUIRE(typeSection.node.content.elements().at(1).content.value.valueDefinition.values.size() == 1);
    REQUIRE(typeSection.node.content.elements().at(1).content.value.valueDefinition.values[0].literal == "100");

    REQUIRE(typeSection.sourceMap.value.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.size() == 2);
    SourceMapHelper::check(typeSection.sourceMap.elements().collection[0].value.valueDefinition.sourceMap, 0, 18);
    SourceMapHelper::check(typeSection.sourceMap.elements().collection[1].value.valueDefinition.sourceMap, 0, 18);
}

TEST_CASE("Parse mson sample list type section for a string but having values", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Sample: 75, 100";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::PrimitiveBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value == "75, 100");
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().empty());

    SourceMapHelper::check(typeSection.sourceMap.value.sourceMap, 0, 18);
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.empty());
}

TEST_CASE("Parse mson sample list type section for an object with a value", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Sample: 75, 100";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::ObjectBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.size() == 1);
    REQUIRE(typeSection.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value.empty());
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().empty());

    REQUIRE(typeSection.sourceMap.value.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.empty());
}

TEST_CASE("Parse mson sample list type section with values as list items", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Sample\n"\
    "  - red\n"\
    "  - green\n";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::ValueBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value.empty());
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().size() == 2);
    REQUIRE(typeSection.node.content.elements().at(0).klass == mson::Element::ValueClass);
    REQUIRE(typeSection.node.content.elements().at(0).content.value.valueDefinition.values.size() == 1);
    REQUIRE(typeSection.node.content.elements().at(0).content.value.valueDefinition.values[0].literal == "red");
    REQUIRE(typeSection.node.content.elements().at(1).klass == mson::Element::ValueClass);
    REQUIRE(typeSection.node.content.elements().at(1).content.value.valueDefinition.values.size() == 1);
    REQUIRE(typeSection.node.content.elements().at(1).content.value.valueDefinition.values[0].literal == "green");

    REQUIRE(typeSection.sourceMap.value.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.size() == 2);
    SourceMapHelper::check(typeSection.sourceMap.elements().collection[0].value.valueDefinition.sourceMap, 13, 4);
    SourceMapHelper::check(typeSection.sourceMap.elements().collection[1].value.valueDefinition.sourceMap, 21, 6);
}

TEST_CASE("Parse multi-line mson sample list type section without newline", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Sample\n"\
    "     red\n"\
    "       green\n\n"\
    " yellow";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::PrimitiveBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value == " red\n   green\n\nyellow");
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().empty());

    REQUIRE(typeSection.sourceMap.value.sourceMap.size() == 4);
    SourceMapHelper::check(typeSection.sourceMap.value.sourceMap, 2, 7, 1);
    SourceMapHelper::check(typeSection.sourceMap.value.sourceMap, 13, 5, 2);
    SourceMapHelper::check(typeSection.sourceMap.value.sourceMap, 22, 10, 3);
    SourceMapHelper::check(typeSection.sourceMap.value.sourceMap, 33, 6, 4);
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.empty());
}

TEST_CASE("Parse multi-line mson sample list type section with newline", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Sample\n\n"\
    "     red\n"\
    "       green\n"\
    "yellow";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::PrimitiveBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value == " red\n   green\nyellow");
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().empty());

    SourceMapHelper::check(typeSection.sourceMap.value.sourceMap, 14, 5, 23, 15);
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.empty());
}

TEST_CASE("Parse mson sample list type section with values as para for values base type", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Sample\n\n"\
    "    a\n"\
    "    b";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::ValueBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value.empty());
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().empty());

    REQUIRE(typeSection.sourceMap.value.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.empty());
}

TEST_CASE("Parse markdown multi-line mson sample list type section", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Sample\n"\
    "  - red\n\n"\
    "  - green";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::PrimitiveBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value == "- red\n\n- green");
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().empty());

    SourceMapHelper::check(typeSection.sourceMap.value.sourceMap, 11, 6, 20, 7);
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.empty());
}

TEST_CASE("Parse mson sample header type section with values as list items", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "# Sample\n"\
    "  - red\n"\
    "  - green\n";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::ValueBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionHeaderParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value.empty());
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().size() == 2);
    REQUIRE(typeSection.node.content.elements().at(0).klass == mson::Element::ValueClass);
    REQUIRE(typeSection.node.content.elements().at(0).content.value.valueDefinition.values.size() == 1);
    REQUIRE(typeSection.node.content.elements().at(0).content.value.valueDefinition.values[0].literal == "red");
    REQUIRE(typeSection.node.content.elements().at(1).klass == mson::Element::ValueClass);
    REQUIRE(typeSection.node.content.elements().at(1).content.value.valueDefinition.values.size() == 1);
    REQUIRE(typeSection.node.content.elements().at(1).content.value.valueDefinition.values[0].literal == "green");

    REQUIRE(typeSection.sourceMap.value.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.size() == 2);
    SourceMapHelper::check(typeSection.sourceMap.elements().collection[0].value.valueDefinition.sourceMap, 13, 4);
    SourceMapHelper::check(typeSection.sourceMap.elements().collection[1].value.valueDefinition.sourceMap, 21, 6);
}

TEST_CASE("Parse multi-line mson sample header type section", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "# Sample\n"\
    "   red\n"\
    "     green\n"\
    "yellow";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::PrimitiveBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionHeaderParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value == "   red\n     green\nyellow");
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().empty());

    SourceMapHelper::check(typeSection.sourceMap.value.sourceMap, 9, 24);
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.empty());
}

TEST_CASE("Parse multi-line mson sample header type section with multiple nested nodes", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "# Sample\n"\
    "   red\n"\
    "     green\n\n"\
    " yellow";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::PrimitiveBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionHeaderParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value == "   red\n     green\n\n yellow");
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().empty());

    SourceMapHelper::check(typeSection.sourceMap.value.sourceMap, 9, 26, 1);
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.empty());
}

TEST_CASE("Parse markdown multi-line mson sample header type section", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "# Sample\n"\
    "- red\n\n"\
    "- green";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::PrimitiveBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionHeaderParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value == "- red\n\n- green");
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().empty());

    SourceMapHelper::check(typeSection.sourceMap.value.sourceMap, 9, 14);
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.empty());
}

TEST_CASE("Parse mson items list type section for values base type containing one of", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Items\n"\
    "    - One Of\n"\
    "        - first_name\n"\
    "        - last_name";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::ValueBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.size() == 1);
    REQUIRE(typeSection.report.warnings[0].code == LogicalErrorWarning);

    REQUIRE(typeSection.node.content.elements().empty());

    REQUIRE(typeSection.sourceMap.value.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.empty());
}

TEST_CASE("Parse mson properties list type section for values base type", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Properties\n"\
    "    - first_name\n"\
    "    - last_name";

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::ValueBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.size() == 1);
    REQUIRE(typeSection.report.warnings[0].code == LogicalErrorWarning);
    REQUIRE(typeSection.node.empty());

    REQUIRE(typeSection.sourceMap.value.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.empty());
}

TEST_CASE("Parse mson sample type section for a simple object", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Sample\n"\
    "    - user (object)\n"\
    "        - username: pksunkara\n"\
    "        - admin: false";

    mson::Element member;
    SourceMap<mson::Element> memberSM;

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::ObjectBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value.empty());
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().size() == 1);
    REQUIRE(typeSection.node.content.elements().at(0).klass == mson::Element::PropertyClass);
    REQUIRE(typeSection.node.content.elements().at(0).content.property.name.literal == "user");
    REQUIRE(typeSection.node.content.elements().at(0).content.property.sections.size() == 1);
    REQUIRE(typeSection.node.content.elements().at(0).content.property.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(typeSection.node.content.elements().at(0).content.property.sections[0].content.elements().size() == 2);

    member = typeSection.node.content.elements().at(0).content.property.sections[0].content.elements().at(0);
    REQUIRE(member.klass == mson::Element::PropertyClass);
    REQUIRE(member.content.property.name.literal == "username");
    REQUIRE(member.content.property.valueDefinition.values.size() == 1);
    REQUIRE(member.content.property.valueDefinition.values[0].literal == "pksunkara");

    member = typeSection.node.content.elements().at(0).content.property.sections[0].content.elements().at(1);
    REQUIRE(member.klass == mson::Element::PropertyClass);
    REQUIRE(member.content.property.name.literal == "admin");
    REQUIRE(member.content.property.valueDefinition.values.size() == 1);
    REQUIRE(member.content.property.valueDefinition.values[0].literal == "false");

    REQUIRE(typeSection.sourceMap.value.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.size() == 1);
    SourceMapHelper::check(typeSection.sourceMap.elements().collection[0].property.name.sourceMap, 15, 14);
    SourceMapHelper::check(typeSection.sourceMap.elements().collection[0].property.valueDefinition.sourceMap, 15, 14);
    REQUIRE(typeSection.sourceMap.elements().collection[0].property.sections.collection.size() == 1);
    REQUIRE(typeSection.sourceMap.elements().collection[0].property.sections.collection[0].elements().collection.size() == 2);

    memberSM = typeSection.sourceMap.elements().collection[0].property.sections.collection[0].elements().collection[0];
    SourceMapHelper::check(memberSM.property.name.sourceMap, 39, 20);
    SourceMapHelper::check(memberSM.property.valueDefinition.sourceMap, 39, 20);

    memberSM = typeSection.sourceMap.elements().collection[0].property.sections.collection[0].elements().collection[1];
    SourceMapHelper::check(memberSM.property.name.sourceMap, 67, 15);
    SourceMapHelper::check(memberSM.property.valueDefinition.sourceMap, 67, 15);
}

TEST_CASE("Parse mson sample type section for a complex object", "[mson][type_section]")
{
    mdp::ByteBuffer source = \
    "- Sample\n"\
    "    - user (object)\n"\
    "        - data (array)\n"\
    "            - pksunkara\n"\
    "            - 1200\n"\
    "            - (object)\n"\
    "                - admin: false";

    mson::Element member, submember;
    SourceMap<mson::Element> memberSM, submemberSM;

    ParseResult<mson::TypeSection> typeSection;
    typeSection.node.baseType = mson::ObjectBaseType;
    SectionParserHelper<mson::TypeSection, MSONTypeSectionListParser>::parse(source, MSONSampleDefaultSectionType, typeSection, ExportSourcemapOption);

    REQUIRE(typeSection.report.error.code == Error::OK);
    REQUIRE(typeSection.report.warnings.empty());

    REQUIRE(typeSection.node.klass == mson::TypeSection::SampleClass);
    REQUIRE(typeSection.node.content.value.empty());
    REQUIRE(typeSection.node.content.description.empty());
    REQUIRE(typeSection.node.content.elements().size() == 1);
    REQUIRE(typeSection.node.content.elements().at(0).klass == mson::Element::PropertyClass);
    REQUIRE(typeSection.node.content.elements().at(0).content.property.name.literal == "user");
    REQUIRE(typeSection.node.content.elements().at(0).content.property.sections.size() == 1);
    REQUIRE(typeSection.node.content.elements().at(0).content.property.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(typeSection.node.content.elements().at(0).content.property.sections[0].content.elements().size() == 1);

    member = typeSection.node.content.elements().at(0).content.property.sections[0].content.elements().at(0);
    REQUIRE(member.klass == mson::Element::PropertyClass);
    REQUIRE(member.content.property.name.literal == "data");
    REQUIRE(member.content.property.valueDefinition.values.empty());
    REQUIRE(member.content.property.valueDefinition.typeDefinition.baseType == mson::ValueBaseType);
    REQUIRE(member.content.property.sections.size() == 1);
    REQUIRE(member.content.property.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(member.content.property.sections[0].content.elements().size() == 3);

    submember = member.content.property.sections[0].content.elements().at(0);
    REQUIRE(submember.klass == mson::Element::ValueClass);
    REQUIRE(submember.content.value.valueDefinition.values.size() == 1);
    REQUIRE(submember.content.value.valueDefinition.values[0].literal == "pksunkara");
    REQUIRE(submember.content.value.sections.empty());

    submember = member.content.property.sections[0].content.elements().at(1);
    REQUIRE(submember.klass == mson::Element::ValueClass);
    REQUIRE(submember.content.value.valueDefinition.values.size() == 1);
    REQUIRE(submember.content.value.valueDefinition.values[0].literal == "1200");
    REQUIRE(submember.content.value.sections.empty());

    submember = member.content.property.sections[0].content.elements().at(2);
    REQUIRE(submember.klass == mson::Element::ValueClass);
    REQUIRE(submember.content.value.valueDefinition.values.empty());
    REQUIRE(submember.content.value.valueDefinition.typeDefinition.baseType == mson::ObjectBaseType);
    REQUIRE(submember.content.value.sections.size() == 1);
    REQUIRE(submember.content.value.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(submember.content.value.sections[0].content.elements().size() == 1);

    member = submember.content.value.sections[0].content.elements().at(0);
    REQUIRE(member.klass == mson::Element::PropertyClass);
    REQUIRE(member.content.property.name.literal == "admin");
    REQUIRE(member.content.property.valueDefinition.values.size() == 1);
    REQUIRE(member.content.property.valueDefinition.values[0].literal == "false");
    REQUIRE(member.content.property.sections.empty());

    REQUIRE(typeSection.sourceMap.value.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.description.sourceMap.empty());
    REQUIRE(typeSection.sourceMap.elements().collection.size() == 1);
    SourceMapHelper::check(typeSection.sourceMap.elements().collection[0].property.name.sourceMap, 15, 14);
    SourceMapHelper::check(typeSection.sourceMap.elements().collection[0].property.valueDefinition.sourceMap, 15, 14);
    REQUIRE(typeSection.sourceMap.elements().collection[0].property.sections.collection.size() == 1);
    REQUIRE(typeSection.sourceMap.elements().collection[0].property.sections.collection[0].elements().collection.size() == 1);

    memberSM = typeSection.sourceMap.elements().collection[0].property.sections.collection[0].elements().collection[0];
    SourceMapHelper::check(memberSM.property.name.sourceMap, 39, 13);
    SourceMapHelper::check(memberSM.property.valueDefinition.sourceMap, 39, 13);
    REQUIRE(memberSM.property.sections.collection.size() == 1);
    REQUIRE(memberSM.property.sections.collection[0].elements().collection.size() == 3);

    submemberSM = memberSM.property.sections.collection[0].elements().collection[0];
    SourceMapHelper::check(submemberSM.value.valueDefinition.sourceMap, 66, 10);
    REQUIRE(submemberSM.value.sections.collection.empty());

    submemberSM = memberSM.property.sections.collection[0].elements().collection[1];
    SourceMapHelper::check(submemberSM.value.valueDefinition.sourceMap, 90, 5);
    REQUIRE(submemberSM.value.sections.collection.empty());

    submemberSM = memberSM.property.sections.collection[0].elements().collection[2];
    SourceMapHelper::check(submemberSM.value.valueDefinition.sourceMap, 109, 9);
    REQUIRE(submemberSM.value.sections.collection.size() == 1);
    REQUIRE(submemberSM.value.sections.collection[0].elements().collection.size() == 1);

    memberSM = submemberSM.value.sections.collection[0].elements().collection[0];
    SourceMapHelper::check(memberSM.property.name.sourceMap, 134, 15);
    SourceMapHelper::check(memberSM.property.valueDefinition.sourceMap, 134, 15);
    REQUIRE(memberSM.property.sections.collection.empty());
}
