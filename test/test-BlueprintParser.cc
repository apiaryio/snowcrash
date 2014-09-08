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
    markdownParser.parse(blueprint.nodeFixture, markdownAST);

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
    SectionParserHelper<Blueprint, BlueprintParser>::parse(BlueprintFixture, BlueprintSectionType, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.metadata.size() == 1);
    REQUIRE(blueprint.node.metadata[0].first == "meta");
    REQUIRE(blueprint.node.metadata[0].second == "verse");

    REQUIRE(blueprint.node.name == "Snowcrash API");
    REQUIRE(blueprint.node.description == "## Character\n\nUncle Enzo\n\n");
    REQUIRE(blueprint.node.resourceGroups.size() == 2);

    REQUIRE(blueprint.node.resourceGroups[0].name == "First");
    REQUIRE(blueprint.node.resourceGroups[0].description == "p1\n");
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);

    REQUIRE(blueprint.node.resourceGroups[1].name == "Second");
    REQUIRE(blueprint.node.resourceGroups[1].description == "p2\n");
    REQUIRE(blueprint.node.resourceGroups[1].resources.empty());
}

TEST_CASE("Parse blueprint with multiple metadata sections", "[blueprint]")
{
    mdp::ByteBuffer source = "FORMAT: 1A\n\n";
    source += BlueprintFixture;

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.metadata.size() == 2);
    REQUIRE(blueprint.node.metadata[0].first == "FORMAT");
    REQUIRE(blueprint.node.metadata[0].second == "1A");
    REQUIRE(blueprint.node.metadata[1].first == "meta");
    REQUIRE(blueprint.node.metadata[1].second == "verse");

    REQUIRE(blueprint.node.name == "Snowcrash API");
    REQUIRE(blueprint.node.description == "## Character\n\nUncle Enzo\n\n");
    REQUIRE(blueprint.node.resourceGroups.size() == 2);

    REQUIRE(blueprint.node.resourceGroups[0].name == "First");
    REQUIRE(blueprint.node.resourceGroups[0].description == "p1\n");
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);

    REQUIRE(blueprint.node.resourceGroups[1].name == "Second");
    REQUIRE(blueprint.node.resourceGroups[1].description == "p2\n");
    REQUIRE(blueprint.node.resourceGroups[1].resources.empty());
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
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.name == "API");
    REQUIRE(blueprint.node.description == "A\n");
    REQUIRE(blueprint.node.resourceGroups.size() == 1);

    ResourceGroup group = blueprint.resourceGroups.front();
    REQUIRE(group.name.empty());
    REQUIRE(group.description.empty());
    REQUIRE(group.resources.size() == 1);

    Resource resource = group.resources.front();
    REQUIRE(resource.uriTemplate == "/resource");
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions.front().examples.size() == 1);
    REQUIRE(resource.actions.front().examples.front().responses.size() == 1);
    REQUIRE(resource.actions.front().examples.front().responses.front().body == "{}\n");
}

TEST_CASE("Parse nameless blueprint description", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "A\n"\
    "# B\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == APINameWarning);

    REQUIRE(blueprint.node.name.empty());
    REQUIRE(blueprint.node.description == "A\n\n# B\n");
    REQUIRE(blueprint.node.resourceGroups.size() == 0);
}

TEST_CASE("Parse nameless blueprint with a list description", "[blueprint]")
{
    mdp::ByteBuffer source = "+ List\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == APINameWarning);

    REQUIRE(blueprint.node.name.empty());
    REQUIRE(blueprint.node.description == "+ List\n");
    REQUIRE(blueprint.node.resourceGroups.size() == 0);
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
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 2); // groups with same name & no response

    REQUIRE(blueprint.node.resourceGroups.size() == 2);

    REQUIRE(blueprint.node.resourceGroups[0].name == "Name");
    REQUIRE(blueprint.node.resourceGroups[0].description.empty());
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);

    REQUIRE(blueprint.node.resourceGroups[1].name == "Name");
    REQUIRE(blueprint.node.resourceGroups[1].description.empty());
}

TEST_CASE("Test parser options - required blueprint name", "[blueprint]")
{
    mdp::ByteBuffer source = "Lorem Ipsum";

    ParseResult<Blueprint> blueprint;

    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint);
    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == APINameWarning);

    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint, Symbols(), RequireBlueprintNameOption);
    REQUIRE(blueprint.report.error.code != Error::OK);
}

TEST_CASE("Test required blueprint name on blueprint that starts with metadata", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "meta: data\n"\
    "foo:bar\n\n"\
    "Hello";

    ParseResult<Blueprint> blueprint;

    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint, Symbols(), RequireBlueprintNameOption);
    REQUIRE(blueprint.report.error.code != Error::OK);
}

TEST_CASE("Should parse nested lists in description", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# API\n"\
    "+ List\n"\
    "   + Nested Item\n";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.name == "API");
    REQUIRE(blueprint.node.description == "+ List\n   + Nested Item\n");
    REQUIRE(blueprint.node.resourceGroups.empty());
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
    REQUIRE(blueprint.node.resourceGroups.empty());
}

TEST_CASE("Blueprint starting with Resource Group should be parsed", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Group Posts\n"\
    "## /posts";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.name.empty());
    REQUIRE(blueprint.node.description.empty());
    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups.front().name == "Posts");
    REQUIRE(blueprint.node.resourceGroups.front().resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups.front().resources.front().uriTemplate == "/posts");
}

TEST_CASE("Blueprint starting with Resource should be parsed", "[blueprint]")
{
    mdp::ByteBuffer source = "# /posts";

    ParseResult<Blueprint> blueprint;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.name.empty());
    REQUIRE(blueprint.node.description.empty());
    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups.front().name.empty());
    REQUIRE(blueprint.node.resourceGroups.front().resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups.front().resources.front().uriTemplate == "/posts");
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
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint, Symbols(), 0, &blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 3); // 2x no response & duplicate resource

    REQUIRE(blueprint.node.name.empty());
    REQUIRE(blueprint.node.description.empty());
    REQUIRE(blueprint.node.resourceGroups.size() == 2);

    REQUIRE(blueprint.node.resourceGroups[0].name.empty());
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.empty());
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].uriTemplate == "/posts");

    REQUIRE(blueprint.node.resourceGroups[1].name == "Posts");
    REQUIRE(blueprint.node.resourceGroups[1].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[1].resources[0].actions.size() == 2);
    REQUIRE(blueprint.node.resourceGroups[1].resources[0].uriTemplate == "/posts");
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

    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint, Symbols(), 0, &blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1); // no response
    REQUIRE(blueprint.report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(blueprint.node.name == "S");
    REQUIRE(blueprint.node.description == "Hello\n\n+ Response\n\nMoar text\n\n");

    REQUIRE(blueprint.node.metadata.size() == 1);
    REQUIRE(blueprint.node.metadata[0].first == "FORMAT");
    REQUIRE(blueprint.node.metadata[0].second == "1A");

    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].uriTemplate == "/");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].method == "GET");
}
