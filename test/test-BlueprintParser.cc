//
//  test-BlueprintParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/16/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "BlueprintParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

mdp::ByteBuffer BlueprintFixture = \
"meta: verse\n\n"\
"# Snowcrash API\n\n"\
"## Character\n"\
"Uncle Enzo\n\n"\
"# Group First\n"\
"p1\n"\
"## My Resource [/resource]\n"\
"# Group Second\n"\
"p2\n";

TEST_CASE("Blueprint block classifier", "[blueprint]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(BlueprintFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());

    // meta: verse
    sectionType = SectionProcessor<Blueprint>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == BlueprintSectionType);

    // # Snowcrash API
    sectionType = SectionProcessor<Blueprint>::sectionType(markdownAST.children().begin() + 1);
    REQUIRE(sectionType == BlueprintSectionType);

    // ## Character
    sectionType = SectionProcessor<Blueprint>::sectionType(markdownAST.children().begin() + 2);
    REQUIRE(sectionType == BlueprintSectionType);

    // Uncle Enzo
    sectionType = SectionProcessor<Blueprint>::sectionType(markdownAST.children().begin() + 3);
    REQUIRE(sectionType == BlueprintSectionType);
}

TEST_CASE("Parse canonical blueprint", "[blueprint]")
{
    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(BlueprintFixture, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.metadata.size() == 1);
    REQUIRE(blueprint.node.metadata[0].first == "meta");
    REQUIRE(blueprint.node.metadata[0].second == "verse");

    REQUIRE(blueprint.node.name == "Snowcrash API");
    REQUIRE(blueprint.node.description == "## Character\n\nUncle Enzo\n\n");
    REQUIRE(blueprint.node.content.elements().size() == 2);

    REQUIRE(blueprint.node.content.elements().at(0).attributes.name == "First");
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 2);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).element == Element::CopyElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).content.copy == "p1\n");

    REQUIRE(blueprint.node.content.elements().at(1).attributes.name == "Second");
    REQUIRE(blueprint.node.content.elements().at(1).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(0).element == Element::CopyElement);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(0).content.copy == "p2\n");

    SourceMapHelper::check(blueprint.sourceMap.name.sourceMap, 13, 17);
    SourceMapHelper::check(blueprint.sourceMap.description.sourceMap, 30, 25);
    REQUIRE(blueprint.sourceMap.metadata.collection.size() == 1);
    SourceMapHelper::check(blueprint.sourceMap.metadata.collection[0].sourceMap, 0, 13);
    REQUIRE(blueprint.sourceMap.content.elements().collection.size() == 2);
}

TEST_CASE("Parse blueprint with multiple metadata sections", "[blueprint]")
{
    mdp::ByteBuffer source = "FORMAT: 1A\n\n";
    source += BlueprintFixture;

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.metadata.size() == 2);
    REQUIRE(blueprint.node.metadata[0].first == "FORMAT");
    REQUIRE(blueprint.node.metadata[0].second == "1A");
    REQUIRE(blueprint.node.metadata[1].first == "meta");
    REQUIRE(blueprint.node.metadata[1].second == "verse");

    REQUIRE(blueprint.node.name == "Snowcrash API");
    REQUIRE(blueprint.node.description == "## Character\n\nUncle Enzo\n\n");
    REQUIRE(blueprint.node.content.elements().size() == 2);

    REQUIRE(blueprint.node.content.elements().at(0).attributes.name == "First");
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 2);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).element == Element::CopyElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).content.copy == "p1\n");

    REQUIRE(blueprint.node.content.elements().at(1).attributes.name == "Second");
    REQUIRE(blueprint.node.content.elements().at(1).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(0).element == Element::CopyElement);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(0).content.copy == "p2\n");

    SourceMapHelper::check(blueprint.sourceMap.name.sourceMap, 25, 17);
    SourceMapHelper::check(blueprint.sourceMap.description.sourceMap, 42, 25);
    REQUIRE(blueprint.sourceMap.metadata.collection.size() == 2);
    SourceMapHelper::check(blueprint.sourceMap.metadata.collection[0].sourceMap, 0, 12);
    SourceMapHelper::check(blueprint.sourceMap.metadata.collection[1].sourceMap, 12, 13);
    REQUIRE(blueprint.sourceMap.content.elements().collection.size() == 2);
}

TEST_CASE("Parse API with Name and abbreviated resource", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# API\n"\
    "A\n"\
    "# GET /resource\n"\
    "B\n"\
    "+ Response 200\n\n"\
    "        {}";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.name == "API");
    REQUIRE(blueprint.node.description == "A\n");
    REQUIRE(blueprint.node.content.elements().size() == 1);

    REQUIRE(blueprint.node.content.elements().at(0).attributes.name.empty());
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).element == Element::ResourceElement);

    Resource resource = blueprint.node.content.elements().at(0).content.elements().at(0).content.resource;
    REQUIRE(resource.uriTemplate == "/resource");
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions.front().examples.size() == 1);
    REQUIRE(resource.actions.front().examples.front().responses.size() == 1);
    REQUIRE(resource.actions.front().examples.front().responses.front().body == "{}\n");

    SourceMapHelper::check(blueprint.sourceMap.name.sourceMap, 0, 6);
    SourceMapHelper::check(blueprint.sourceMap.description.sourceMap, 6, 2);
    REQUIRE(blueprint.sourceMap.metadata.collection.size() == 0);
    REQUIRE(blueprint.sourceMap.content.elements().collection.size() == 1);
}

TEST_CASE("Parse nameless blueprint description", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "A\n"\
    "# B\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == APINameWarning);

    REQUIRE(blueprint.node.name.empty());
    REQUIRE(blueprint.node.description == "A\n\n# B\n");
    REQUIRE(blueprint.node.content.elements().size() == 0);

    REQUIRE(blueprint.sourceMap.name.sourceMap.empty());
    SourceMapHelper::check(blueprint.sourceMap.description.sourceMap, 0, 6);
    REQUIRE(blueprint.sourceMap.metadata.collection.size() == 0);
    REQUIRE(blueprint.sourceMap.content.elements().collection.size() == 0);
}

TEST_CASE("Parse nameless blueprint with a list description", "[blueprint]")
{
    mdp::ByteBuffer source = "+ List\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == APINameWarning);

    REQUIRE(blueprint.node.name.empty());
    REQUIRE(blueprint.node.description == "+ List\n");
    REQUIRE(blueprint.node.content.elements().size() == 0);

    REQUIRE(blueprint.sourceMap.name.sourceMap.empty());
    SourceMapHelper::check(blueprint.sourceMap.description.sourceMap, 0, 7);
    REQUIRE(blueprint.sourceMap.metadata.collection.size() == 0);
    REQUIRE(blueprint.sourceMap.content.elements().collection.size() == 0);
}

TEST_CASE("Parse two groups with the same name", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# API\n"\
    "# Group Name\n"\
    "## /1\n"\
    "### POST\n"\
    "+ Request\n\n"\
    "         {}\n\n"\
    "# Group Name\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 2); // groups with same name & no response

    REQUIRE(blueprint.node.content.elements().size() == 2);

    REQUIRE(blueprint.node.content.elements().at(0).attributes.name == "Name");
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);

    REQUIRE(blueprint.node.content.elements().at(1).attributes.name == "Name");
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().empty());

    SourceMapHelper::check(blueprint.sourceMap.name.sourceMap, 0, 6);
    REQUIRE(blueprint.sourceMap.description.sourceMap.empty());
    REQUIRE(blueprint.sourceMap.metadata.collection.size() == 0);
    REQUIRE(blueprint.sourceMap.content.elements().collection.size() == 2);
}

TEST_CASE("Test parser options - required blueprint name", "[blueprint]")
{
    mdp::ByteBuffer source = "Lorem Ipsum";

    ParseResult<Blueprint> blueprint;

    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint);
    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == APINameWarning);

    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, RequireBlueprintNameOption);
    REQUIRE(blueprint.report.error.code != Error::OK);
}

TEST_CASE("Test required blueprint name on blueprint that starts with metadata", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "meta: data\n"\
    "foo:bar\n\n"\
    "Hello";

    ParseResult<Blueprint> blueprint;

    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, RequireBlueprintNameOption);
    REQUIRE(blueprint.report.error.code != Error::OK);
}

TEST_CASE("Should parse nested lists in description", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# API\n"\
    "+ List\n"\
    "   + Nested Item\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.name == "API");
    REQUIRE(blueprint.node.description == "+ List\n   + Nested Item\n");
    REQUIRE(blueprint.node.content.elements().empty());

    SourceMapHelper::check(blueprint.sourceMap.name.sourceMap, 0, 6);
    SourceMapHelper::check(blueprint.sourceMap.description.sourceMap, 6, 24);
    REQUIRE(blueprint.sourceMap.metadata.collection.size() == 0);
    REQUIRE(blueprint.sourceMap.content.elements().collection.size() == 0);
}

TEST_CASE("Should parse paragraph without final newline", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# API\n"\
    "Lorem Ipsum";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.name == "API");
    REQUIRE(blueprint.node.description == "Lorem Ipsum");
    REQUIRE(blueprint.node.content.elements().empty());
}

TEST_CASE("Blueprint starting with Resource Group should be parsed", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Group Posts\n"\
    "## /posts";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.name.empty());
    REQUIRE(blueprint.node.description.empty());
    REQUIRE(blueprint.node.content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).attributes.name == "Posts");
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).content.resource.uriTemplate == "/posts");

    REQUIRE(blueprint.sourceMap.name.sourceMap.empty());
    REQUIRE(blueprint.sourceMap.description.sourceMap.empty());
    REQUIRE(blueprint.sourceMap.metadata.collection.size() == 0);
    REQUIRE(blueprint.sourceMap.content.elements().collection.size() == 1);
}

TEST_CASE("Blueprint starting with Resource should be parsed", "[blueprint]")
{
    mdp::ByteBuffer source = "# /posts";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.name.empty());
    REQUIRE(blueprint.node.description.empty());
    REQUIRE(blueprint.node.content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).attributes.name.empty());
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).content.resource.uriTemplate == "/posts");

    REQUIRE(blueprint.sourceMap.name.sourceMap.empty());
    REQUIRE(blueprint.sourceMap.description.sourceMap.empty());
    REQUIRE(blueprint.sourceMap.metadata.collection.size() == 0);
    REQUIRE(blueprint.sourceMap.content.elements().collection.size() == 1);
}

TEST_CASE("Checking a resource with global resources for duplicates", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# /posts\n"\
    "# Group Posts\n"\
    "## Posts [/posts]\n"\
    "### Creat a post [POST]\n"\
    "### List posts [GET]\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption, Models(), &blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 3); // 2x no response & duplicate resource

    REQUIRE(blueprint.node.name.empty());
    REQUIRE(blueprint.node.description.empty());
    REQUIRE(blueprint.node.content.elements().size() == 2);

    REQUIRE(blueprint.node.content.elements().at(0).attributes.name.empty());
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).content.resource.uriTemplate == "/posts");
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).content.resource.actions.empty());

    REQUIRE(blueprint.node.content.elements().at(1).attributes.name == "Posts");
    REQUIRE(blueprint.node.content.elements().at(1).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(0).content.resource.uriTemplate == "/posts");
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(0).content.resource.actions.size() == 2);


    REQUIRE(blueprint.sourceMap.name.sourceMap.empty());
    REQUIRE(blueprint.sourceMap.description.sourceMap.empty());
    REQUIRE(blueprint.sourceMap.metadata.collection.size() == 0);
    REQUIRE(blueprint.sourceMap.content.elements().collection.size() == 2);
    REQUIRE(blueprint.sourceMap.content.elements().collection[0].content.elements().collection.size() == 1);
    REQUIRE(blueprint.sourceMap.content.elements().collection[1].content.elements().collection.size() == 1);
}

TEST_CASE("Parsing unexpected blocks", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "FORMAT: 1A\n"\
    "\n"\
    "# S\n"\
    "\n"\
    "Hello\n"\
    "\n"\
    "+ Response\n"\
    "\n"\
    "Moar text\n"\
    "\n"\
    "# GET /\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1); // no response
    REQUIRE(blueprint.report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(blueprint.node.name == "S");
    REQUIRE(blueprint.node.description == "Hello\n\n+ Response\n\nMoar text\n\n");

    REQUIRE(blueprint.node.metadata.size() == 1);
    REQUIRE(blueprint.node.metadata[0].first == "FORMAT");
    REQUIRE(blueprint.node.metadata[0].second == "1A");

    REQUIRE(blueprint.node.content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).attributes.name.empty());
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 1);

    Resource resource = blueprint.node.content.elements().at(0).content.elements().at(0).content.resource;
    REQUIRE(resource.uriTemplate == "/");
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions[0].method == "GET");

    SourceMapHelper::check(blueprint.sourceMap.name.sourceMap, 12, 5);
    SourceMapHelper::check(blueprint.sourceMap.description.sourceMap, 17, 30);
    REQUIRE(blueprint.sourceMap.metadata.collection.size() == 1);
    SourceMapHelper::check(blueprint.sourceMap.metadata.collection[0].sourceMap, 0, 12);
    REQUIRE(blueprint.sourceMap.content.elements().collection.size() == 1);
}

TEST_CASE("Parsing blueprint with mson data structures", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Group Plans\n"\
    "\n"\
    "## Plans [/plans]\n"\
    "\n"\
    "+ Attributes (array[Plan])\n"\
    "\n"\
    "### List all plans [GET]\n"\
    "\n"\
    "+ Response 200 (application/json)\n"\
    "    + Attributes (Plans)\n"\
    "\n"\
    "### Create a plan [POST]\n"\
    "\n"\
    "+ Attributes (Plan Base)\n"\
    "\n"\
    "+ Request (application/json)\n"\
    "\n"\
    "+ Response 201 (application/json)\n"\
    "    + Attributes (Plan)\n"\
    "\n"\
    "## Plan [/plan/{id}]\n"\
    "\n"\
    "+ Parameters\n\n"\
    "    + id (required, string)\n"\
    "\n"\
    "+ Attributes (Plan Base)\n"\
    "    + type: Plan (default)\n"\
    "    + created ([Timestamp][])\n"\
    "\n"\
    "### Retrieve a plan [GET]\n"\
    "\n"\
    "+ Response 200 (application/json)\n"\
    "    + Attributes (Plan)\n"\
    "\n"\
    "### Update a plan [PATCH]\n"\
    "\n"\
    "+ Request (application/json)\n"\
    "    + Attributes (Plan Base)\n"\
    "\n"\
    "+ Response 200 (application/json)\n"\
    "    + Attributes (Plan)\n\n"\
    "\n"\
    "### Delete a plan [DELETE]\n"\
    "\n"\
    "+ Response 204\n"\
    "\n"\
    "# Data Structures\n"\
    "\n"\
    "## Plan Base\n"\
    "Base object for write operations\n"\
    "\n"\
    "### Properties\n"\
    "- name\n"\
    "- amount (number)\n"\
    "- trial (optional)\n\n"\
    "\n"\
    "## Timestamp (number)\n"\
    "\n"\
    "## Coupon [/coupon/{id}]\n"\
    "\n"\
    "+ Parameters\n\n"\
    "    + id (required, string)\n\n"\
    "\n"\
    "### Delete a coupon [DELETE]\n"\
    "\n"\
    "+ Response 204\n"\
    "\n";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;

    ParseResult<Blueprint> blueprint;
    mson::NamedTypeBaseTable::iterator baseIt;
    mson::NamedTypeInheritanceTable::iterator inheritanceIt;

    markdownParser.parse(source, markdownAST);
    REQUIRE(!markdownAST.children().empty());

    snowcrash::SectionParserData pd(ExportSourcemapOption, source, blueprint.node);
    pd.sectionsContext.push_back(BlueprintSectionType);

    BlueprintParser::parse(markdownAST.children().begin(), markdownAST.children(), pd, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    baseIt = pd.namedTypeBaseTable.find("Plan Base");
    REQUIRE(baseIt != pd.namedTypeBaseTable.end());
    REQUIRE(baseIt->second == mson::ImplicitObjectBaseType);

    baseIt = pd.namedTypeBaseTable.find("Plan");
    REQUIRE(baseIt != pd.namedTypeBaseTable.end());
    REQUIRE(baseIt->second == mson::ImplicitObjectBaseType);

    baseIt = pd.namedTypeBaseTable.find("Plans");
    REQUIRE(baseIt != pd.namedTypeBaseTable.end());
    REQUIRE(baseIt->second == mson::ValueBaseType);

    baseIt = pd.namedTypeBaseTable.find("Timestamp");
    REQUIRE(baseIt != pd.namedTypeBaseTable.end());
    REQUIRE(baseIt->second == mson::PrimitiveBaseType);

    inheritanceIt = pd.namedTypeInheritanceTable.find("Plan");
    REQUIRE(inheritanceIt != pd.namedTypeInheritanceTable.end());
    REQUIRE(inheritanceIt->second.first == "Plan Base");

    REQUIRE(blueprint.node.content.elements().size() == 3);
    REQUIRE(blueprint.node.content.elements().at(2).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(2).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().size() == 2);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(0).content.dataStructure.name.symbol.literal == "Plan Base");
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(1).content.dataStructure.name.symbol.literal == "Timestamp");
}

TEST_CASE("Parse blueprint with two named types having the same name", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "## Coupon\n"\
    "- real_name - Coupon's real name\n"\
    "\n"\
    "# Coupon [/coupon]\n"\
    "+ Attributes\n"\
    "    - name - Coupon name";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption, Models(), &blueprint);

    REQUIRE(blueprint.report.error.code == MSONError);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == DuplicateWarning);

    REQUIRE(blueprint.node.content.elements().size() == 2);
    REQUIRE(blueprint.node.content.elements().at(1).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(0).element == Element::ResourceElement);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(0).content.resource.attributes.empty());
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 1);
}

TEST_CASE("Parser blueprint correctly when having a big chain of inheritance in data structures", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "## GET /\n"\
    "+ Response 200 (application/json)\n"\
    "    + Attributes (Coupon A)\n"\
    "\n"\
    "# Data Structures\n"\
    "\n"\
    "## Timestamps (object)\n"\
    "+ created (number)\n"\
    "\n"\
    "## Coupon Base (Timestamps)\n"\
    "+ percent_off: 25 (number)\n"\
    "+ redeem_by (number)\n"\
    "\n"\
    "## Coupon A (Coupon Base)\n"\
    "Clone\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.content.elements().size() == 2);
    REQUIRE(blueprint.node.content.elements().size() == 2);
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).element == Element::ResourceElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).content.resource.attributes.empty());
    REQUIRE(blueprint.node.content.elements().at(1).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().size() == 3);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(0).element == Element::DataStructureElement);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(0).content.dataStructure.name.symbol.literal == "Timestamps");
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(1).element == Element::DataStructureElement);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(1).content.dataStructure.name.symbol.literal == "Coupon Base");
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(2).element == Element::DataStructureElement);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(2).content.dataStructure.name.symbol.literal == "Coupon A");
}

TEST_CASE("Report error when coming across a super type reference to non existent named type", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## Timestamps (object)\n"\
    "+ created (number)\n"\
    "\n"\
    "## Coupon Base (Timestamp)\n"\
    "+ percent_off: 25 (number)\n"\
    "+ redeem_by (number)\n"\
    "\n"\
    "## Coupon A (Coupon Base)\n"\
    "+ id: 25OFF (string)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    SourceMapHelper::check(blueprint.report.error.location, 62, 27);
}

TEST_CASE("Report error when a Data Structure inherits from itself", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## B (B)\n"\
    "+ id (string)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    SourceMapHelper::check(blueprint.report.error.location, 19, 9);
}

TEST_CASE("Report error when named type inherits a sub type in array", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## A (array[B])\n"\
    "## B (A)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    SourceMapHelper::check(blueprint.report.error.location, 35, 9);
}

TEST_CASE("Report error when data Structure inheritance graph contains a cycle", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## A (B)\n"\
    "## B (C)\n"\
    "## C (A)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    SourceMapHelper::check(blueprint.report.error.location, 19, 9);
}

TEST_CASE("Report error when data Structure inheritance graph with only a few of them forming a cycle", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## A (B)\n"\
    "## B (C)\n"\
    "## C (D)\n"\
    "## D (E)\n"\
    "## E (C)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    SourceMapHelper::check(blueprint.report.error.location, 37, 9);
}

TEST_CASE("Do not report error when named sub type is referenced in nested members", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## A (B)\n"\
    "## B\n"\
    "+ person (A)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());
    REQUIRE(blueprint.node.content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 2);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).element == Element::DataStructureElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(1).element == Element::DataStructureElement);

    DataStructure dsA = blueprint.node.content.elements().at(0).content.elements().at(0).content.dataStructure;
    REQUIRE(dsA.name.symbol.literal == "A");
    REQUIRE(dsA.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dsA.typeDefinition.typeSpecification.name.symbol.literal == "B");

    DataStructure dsB = blueprint.node.content.elements().at(0).content.elements().at(1).content.dataStructure;
    REQUIRE(dsB.name.symbol.literal == "B");
    REQUIRE(dsB.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dsB.typeDefinition.empty());
    REQUIRE(dsB.sections.size() == 1);
    REQUIRE(dsB.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(dsB.sections[0].content.elements().size() == 1);
    REQUIRE(dsB.sections[0].content.elements().at(0).klass == mson::Element::PropertyClass);

    mson::PropertyMember person = dsB.sections[0].content.elements().at(0).content.property;
    REQUIRE(person.name.literal == "person");
    REQUIRE(person.valueDefinition.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(person.valueDefinition.typeDefinition.typeSpecification.name.symbol.literal == "A");
}

TEST_CASE("Do not report error when there are circular references in nested members", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## B (C)\n"\
    "## C\n"\
    "+ id (A)\n"
    "\n"\
    "## A\n"\
    "+ id (B)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());
    REQUIRE(blueprint.node.content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 3);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).element == Element::DataStructureElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(1).element == Element::DataStructureElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(2).element == Element::DataStructureElement);

    DataStructure dsB = blueprint.node.content.elements().at(0).content.elements().at(0).content.dataStructure;
    REQUIRE(dsB.name.symbol.literal == "B");
    REQUIRE(dsB.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dsB.typeDefinition.typeSpecification.name.symbol.literal == "C");

    DataStructure dsC = blueprint.node.content.elements().at(0).content.elements().at(1).content.dataStructure;
    REQUIRE(dsC.name.symbol.literal == "C");
    REQUIRE(dsC.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dsC.typeDefinition.empty());
    REQUIRE(dsC.sections.size() == 1);
    REQUIRE(dsC.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(dsC.sections[0].content.elements().size() == 1);
    REQUIRE(dsC.sections[0].content.elements().at(0).klass == mson::Element::PropertyClass);

    mson::PropertyMember idC = dsC.sections[0].content.elements().at(0).content.property;
    REQUIRE(idC.name.literal == "id");
    REQUIRE(idC.valueDefinition.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(idC.valueDefinition.typeDefinition.typeSpecification.name.symbol.literal == "A");

    DataStructure dsA = blueprint.node.content.elements().at(0).content.elements().at(2).content.dataStructure;
    REQUIRE(dsA.name.symbol.literal == "A");
    REQUIRE(dsA.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dsA.typeDefinition.empty());
    REQUIRE(dsA.sections.size() == 1);
    REQUIRE(dsA.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(dsA.sections[0].content.elements().size() == 1);
    REQUIRE(dsA.sections[0].content.elements().at(0).klass == mson::Element::PropertyClass);

    mson::PropertyMember idA = dsA.sections[0].content.elements().at(0).content.property;
    REQUIRE(idA.name.literal == "id");
    REQUIRE(idA.valueDefinition.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(idA.valueDefinition.typeDefinition.typeSpecification.name.symbol.literal == "B");
}

TEST_CASE("Do not report error when named sub type is referenced in nested members when reference happens first", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## B\n"\
    "+ person (A)\n"\
    "\n"\
    "## A (B)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());
    REQUIRE(blueprint.node.content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 2);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).element == Element::DataStructureElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(1).element == Element::DataStructureElement);

    DataStructure dsB = blueprint.node.content.elements().at(0).content.elements().at(0).content.dataStructure;
    REQUIRE(dsB.name.symbol.literal == "B");
    REQUIRE(dsB.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dsB.typeDefinition.empty());
    REQUIRE(dsB.sections.size() == 1);
    REQUIRE(dsB.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(dsB.sections[0].content.elements().size() == 1);
    REQUIRE(dsB.sections[0].content.elements().at(0).klass == mson::Element::PropertyClass);

    mson::PropertyMember person = dsB.sections[0].content.elements().at(0).content.property;
    REQUIRE(person.name.literal == "person");
    REQUIRE(person.valueDefinition.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(person.valueDefinition.typeDefinition.typeSpecification.name.symbol.literal == "A");

    DataStructure dsA = blueprint.node.content.elements().at(0).content.elements().at(1).content.dataStructure;
    REQUIRE(dsA.name.symbol.literal == "A");
    REQUIRE(dsA.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dsA.typeDefinition.typeSpecification.name.symbol.literal == "B");
}

TEST_CASE("Do not report error when a resource attributes type is circularly referenced in nested members", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Post [/]\n"\
    "\n"\
    "+ Attributes (B)\n"\
    "    + id\n"\
    "\n"\
    "# Data Structures\n"\
    "\n"\
    "## B\n"\
    "+ posts (Post)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());
    REQUIRE(blueprint.node.content.elements().size() == 2);
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).element == Element::ResourceElement);

    Resource r = blueprint.node.content.elements().at(0).content.elements().at(0).content.resource;
    REQUIRE(r.name == "Post");
    REQUIRE(r.attributes.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(r.attributes.typeDefinition.typeSpecification.name.symbol.literal == "B");
    REQUIRE(r.attributes.sections.size() == 1);
    REQUIRE(r.attributes.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(r.attributes.sections[0].content.elements().size() == 1);
    REQUIRE(r.attributes.sections[0].content.elements().at(0).klass == mson::Element::PropertyClass);

    mson::PropertyMember id = r.attributes.sections[0].content.elements().at(0).content.property;
    REQUIRE(id.name.literal == "id");
    REQUIRE(id.valueDefinition.typeDefinition.baseType == mson::ImplicitPrimitiveBaseType);

    REQUIRE(blueprint.node.content.elements().at(1).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(0).element == Element::DataStructureElement);

    DataStructure dsB = blueprint.node.content.elements().at(1).content.elements().at(0).content.dataStructure;
    REQUIRE(dsB.name.symbol.literal == "B");
    REQUIRE(dsB.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dsB.typeDefinition.empty());
    REQUIRE(dsB.sections.size() == 1);
    REQUIRE(dsB.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(dsB.sections[0].content.elements().size() == 1);
    REQUIRE(dsB.sections[0].content.elements().at(0).klass == mson::Element::PropertyClass);

    mson::PropertyMember posts = dsB.sections[0].content.elements().at(0).content.property;
    REQUIRE(posts.name.literal == "posts");
    REQUIRE(posts.valueDefinition.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(posts.valueDefinition.typeDefinition.typeSpecification.name.symbol.literal == "Post");
}

TEST_CASE("Report error when named sub type is referenced as mixin", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## A (B)\n"\
    "## B\n"\
    "+ Include A\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    SourceMapHelper::check(blueprint.report.error.location, 35, 10);
}

TEST_CASE("Report error when named sub type is referenced as mixin when reference happens first", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## B\n"\
    "+ Include A\n"\
    "\n"\
    "## A (B)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    SourceMapHelper::check(blueprint.report.error.location, 26, 10);
}

TEST_CASE("Report error when circular reference in mixins", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## A\n"\
    "+ Include B\n"\
    "\n"\
    "## B \n" \
    "+ Include A\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    SourceMapHelper::check(blueprint.report.error.location, 45, 10);
}

TEST_CASE("Do not report error when named type references itself in array", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## Comment\n"\
    "+ user (string)\n"\
    "+ children (array[Comment])\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());
    REQUIRE(blueprint.node.content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(0).content.elements().at(0).element == Element::DataStructureElement);

    DataStructure dsComment = blueprint.node.content.elements().at(0).content.elements().at(0).content.dataStructure;
    REQUIRE(dsComment.name.symbol.literal == "Comment");
    REQUIRE(dsComment.typeDefinition.baseType == mson::ImplicitObjectBaseType);
    REQUIRE(dsComment.typeDefinition.empty());
    REQUIRE(dsComment.sections.size() == 1);
    REQUIRE(dsComment.sections[0].klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(dsComment.sections[0].content.elements().size() == 2);
    REQUIRE(dsComment.sections[0].content.elements().at(0).klass == mson::Element::PropertyClass);

    mson::PropertyMember user = dsComment.sections[0].content.elements().at(0).content.property;
    REQUIRE(user.name.literal == "user");
    REQUIRE(user.valueDefinition.typeDefinition.baseType == mson::PrimitiveBaseType);

    mson::PropertyMember children  = dsComment.sections[0].content.elements().at(1).content.property;
    REQUIRE(children.name.literal == "children");
    REQUIRE(children.valueDefinition.typeDefinition.baseType == mson::ValueBaseType);
    REQUIRE(children.valueDefinition.typeDefinition.typeSpecification.nestedTypes.size() == 1);
    REQUIRE(children.valueDefinition.typeDefinition.typeSpecification.nestedTypes.at(0).symbol.literal == "Comment");
}

TEST_CASE("Report error when a named type is defined twice with inheritance", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## A (B)\n"\
    "## B (A)\n"\
    "## B (C)\n"\
    "## C (object)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    SourceMapHelper::check(blueprint.report.error.location, 19, 9);
}

TEST_CASE("Report error when a named type is defined twice with base type", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## A (string)\n"\
    "## A (object)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    SourceMapHelper::check(blueprint.report.error.location, 33, 14);
}

TEST_CASE("Report error when a named type is defined twice, once with base type and other inheritance", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "\n"\
    "## A (B)\n"\
    "## A (object)\n"\
    "## B (object)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    SourceMapHelper::check(blueprint.report.error.location, 28, 14);
}

TEST_CASE("Parse mson signature attributes with mismatched square brackets", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# /\n"\
    "+ Attributes (array[Note)";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
}

TEST_CASE("Parse named type mson signature attributes with no closing bracket", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "## B (A(";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    REQUIRE(blueprint.report.error.message == "base type 'A(' is not defined in the document");
}

TEST_CASE("Parse correctly when a resource named type is non-circularly referenced in action attributes", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Question [/question]\n"\
    "+ Attributes\n"\
    "    + question\n"\
    "\n"\
    "# Choice [/choice]\n"\
    "## Vote on choice [POST]\n"\
    "+ Attributes\n"\
    "    + bla (Question)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
}

TEST_CASE("Report error when not finding a super type of the nested member", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "## A\n"\
    "+ choice (B)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    REQUIRE(blueprint.report.error.message == "base type 'B' is not defined in the document");
}

TEST_CASE("Report error when not finding a nested super type of the nested member", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "## A\n"\
    "+ choice (array[B])\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    REQUIRE(blueprint.report.error.message == "base type 'B' is not defined in the document");
}

TEST_CASE("Report error when not finding a mixin type", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "## A\n"\
    "+ Include B\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    REQUIRE(blueprint.report.error.message == "base type 'B' is not defined in the document");
}

TEST_CASE("When an object contains a mixin of array type", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "## C (object)\n"\
    "+ Include A\n"\
    "\n"\
    "## D (object)\n"\
    "+ Include B";

    NamedTypes namedTypes;
    NamedTypeHelper::build("A", mson::ObjectBaseType, namedTypes);
    NamedTypeHelper::build("B", mson::ValueBaseType, namedTypes);

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption, Models(), &blueprint, namedTypes);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == LogicalErrorWarning);
    SourceMapHelper::check(blueprint.report.warnings[0].location, 59, 11);
}

TEST_CASE("When an array contains a mixin of object type", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "## C (array)\n"\
    "+ Include A\n"\
    "\n"\
    "## D (array)\n"\
    "+ Include B";

    NamedTypes namedTypes;
    NamedTypeHelper::build("A", mson::ObjectBaseType, namedTypes);
    NamedTypeHelper::build("B", mson::ValueBaseType, namedTypes);

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption, Models(), &blueprint, namedTypes);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == LogicalErrorWarning);
    SourceMapHelper::check(blueprint.report.warnings[0].location, 31, 13);
}

TEST_CASE("When an object member contains a mixin of array type", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "## C\n"\
    "+ d (object)\n"\
    "    + Include A\n"\
    "+ e (object)\n"\
    "    + Include B";

    NamedTypes namedTypes;
    NamedTypeHelper::build("A", mson::ObjectBaseType, namedTypes);
    NamedTypeHelper::build("B", mson::ValueBaseType, namedTypes);

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption, Models(), &blueprint, namedTypes);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == LogicalErrorWarning);
    SourceMapHelper::check(blueprint.report.warnings[0].location, 69, 11);
}

TEST_CASE("When an array member contains a mixin of object type", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Data Structures\n"\
    "## C\n"\
    "+ d (array)\n"\
    "    + Include A\n"\
    "+ e (array)\n"\
    "    + Include B";

    NamedTypes namedTypes;
    NamedTypeHelper::build("A", mson::ObjectBaseType, namedTypes);
    NamedTypeHelper::build("B", mson::ValueBaseType, namedTypes);

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption, Models(), &blueprint, namedTypes);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == LogicalErrorWarning);
    SourceMapHelper::check(blueprint.report.warnings[0].location, 39, 12);
}

TEST_CASE("Any named type data structure should be able to be overridden when referenced", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Tesla\n"\
    "\n"\
    "# Data Structures\n"\
    "## User (object)\n"\
    "+ username: kyle\n"\
    "\n"\
    "# GET /sample\n"\
    "+ Response 200 (application/json)\n"\
    "    + Attributes\n"\
    "        + data (object, required)\n"\
    "            + users (array, required)\n"\
    "                + (User)\n"\
    "                    + relation: family\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.content.elements().size() == 2);
    REQUIRE(blueprint.node.content.elements().at(1).element == Element::CategoryElement);
    REQUIRE(blueprint.node.content.elements().at(1).category == Element::ResourceGroupCategory);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().size() == 1);
    REQUIRE(blueprint.node.content.elements().at(1).content.elements().at(0).element == Element::ResourceElement);

    Resource resource = blueprint.node.content.elements().at(1).content.elements().at(0).content.resource;

    REQUIRE(resource.uriTemplate == "/sample");
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions.at(0).method == "GET");
    REQUIRE(resource.actions.at(0).examples.size() == 1);
    REQUIRE(resource.actions.at(0).examples.at(0).requests.empty());
    REQUIRE(resource.actions.at(0).examples.at(0).responses.size() == 1);

    Response response = resource.actions.at(0).examples.at(0).responses.at(0);

    REQUIRE(response.name == "200");
    REQUIRE(response.attributes.sections.size() == 1);
    REQUIRE(response.attributes.sections.at(0).klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(response.attributes.sections.at(0).content.elements().size() == 1);
    REQUIRE(response.attributes.sections.at(0).content.elements().at(0).klass == mson::Element::PropertyClass);

    mson::PropertyMember property1 = response.attributes.sections.at(0).content.elements().at(0).content.property;

    REQUIRE(property1.name.literal == "data");
    REQUIRE(property1.sections.size() == 1);
    REQUIRE(property1.sections.at(0).klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(property1.sections.at(0).content.elements().size() == 1);
    REQUIRE(property1.sections.at(0).content.elements().at(0).klass == mson::Element::PropertyClass);

    mson::PropertyMember property2 = property1.sections.at(0).content.elements().at(0).content.property;

    REQUIRE(property2.name.literal == "users");
    REQUIRE(property2.sections.size() == 1);
    REQUIRE(property2.sections.at(0).klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(property2.sections.at(0).content.elements().size() == 1);
    REQUIRE(property2.sections.at(0).content.elements().at(0).klass == mson::Element::ValueClass);

    mson::ValueMember value = property2.sections.at(0).content.elements().at(0).content.value;

    REQUIRE(value.valueDefinition.typeDefinition.typeSpecification.name.symbol.literal == "User");
    REQUIRE(value.sections.size() == 1);
    REQUIRE(value.sections.at(0).klass == mson::TypeSection::MemberTypeClass);
    REQUIRE(value.sections.at(0).content.elements().size() == 1);
    REQUIRE(value.sections.at(0).content.elements().at(0).klass == mson::Element::PropertyClass);

    mson::PropertyMember property3 = value.sections.at(0).content.elements().at(0).content.property;

    REQUIRE(property3.name.literal == "relation");
    REQUIRE(property3.sections.empty());
    REQUIRE(property3.valueDefinition.values.size() == 1);
    REQUIRE(property3.valueDefinition.values[0].literal == "family");
    REQUIRE(property3.valueDefinition.typeDefinition.empty());
}

TEST_CASE("Parse attributes with mixin and no base type mentioned for attributes", "[blueprint][362]")
{
    mdp::ByteBuffer source = \
    "# GET /\n"\
    "+ Response 200\n"\
    "    + Attributes\n"\
    "        + Include A\n"\
    "\n"\
    "# Data Structures\n"\
    "\n"\
    "## A (object)\n"\
    "+ a: a\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());
    REQUIRE(blueprint.node.content.elements().size() == 2);
}

TEST_CASE("Parse attributes with mixin and no base type mentioned for mixin", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# GET /\n"\
    "+ Response 200\n"\
    "    + Attributes (object)\n"\
    "        + Include A\n"\
    "\n"\
    "# Data Structures\n"\
    "\n"\
    "## A\n"\
    "+ a: a\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());
    REQUIRE(blueprint.node.content.elements().size() == 2);
}

TEST_CASE("Report error when not finding a super type of the nested member from attributes", "[blueprint][354]")
{
    mdp::ByteBuffer source = \
    "# GET /\n"\
    "+ Response 200\n"\
    "    + Attributes\n"\
    "        + data (A)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    REQUIRE(blueprint.report.error.message == "base type 'A' is not defined in the document");
    REQUIRE(blueprint.report.warnings.empty());
}

TEST_CASE("Report error when not finding a super type of the attributes", "[blueprint][354]")
{
    mdp::ByteBuffer source = \
    "# GET /\n"\
    "+ Response 200\n"\
    "    + Attributes (A)\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint, ExportSourcemapOption);

    REQUIRE(blueprint.report.error.code == MSONError);
    REQUIRE(blueprint.report.error.message == "base type 'A' is not defined in the document");
    REQUIRE(blueprint.report.warnings.empty());
}
