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
    markdownParser.parse(BlueprintFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());

    // meta: verse
    REQUIRE(SectionProcessor<Blueprint>::sectionType(markdownAST.children().begin()) == BlueprintSectionType);
    
    // # Snowcrash API
    REQUIRE(SectionProcessor<Blueprint>::sectionType(markdownAST.children().begin() + 1) == BlueprintSectionType);
    
    // ## Character
    REQUIRE(SectionProcessor<Blueprint>::sectionType(markdownAST.children().begin() + 2) == BlueprintSectionType);
    
    // Uncle Enzo
    REQUIRE(SectionProcessor<Blueprint>::sectionType(markdownAST.children().begin() + 3) == BlueprintSectionType);
}

TEST_CASE("Parse canonical blueprint", "[blueprint]")
{
    Blueprint blueprint;
    Report report;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(BlueprintFixture, BlueprintSectionType, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(blueprint.metadata.size() == 1);
    REQUIRE(blueprint.metadata[0].first == "meta");
    REQUIRE(blueprint.metadata[0].second == "verse");

    REQUIRE(blueprint.name == "Snowcrash API");
    REQUIRE(blueprint.description == "## Character\n\nUncle Enzo\n\n");
    REQUIRE(blueprint.resourceGroups.size() == 2);

    REQUIRE(blueprint.resourceGroups[0].name == "First");
    REQUIRE(blueprint.resourceGroups[0].description == "p1\n");
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);

    REQUIRE(blueprint.resourceGroups[1].name == "Second");
    REQUIRE(blueprint.resourceGroups[1].description == "p2\n");
    REQUIRE(blueprint.resourceGroups[1].resources.empty());
}

TEST_CASE("Parse blueprint with multiple metadata sections", "[blueprint]")
{
    mdp::ByteBuffer source = "FORMAT: 1A\n\n";
    source += BlueprintFixture;

    Blueprint blueprint;
    Report report;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(blueprint.metadata.size() == 2);
    REQUIRE(blueprint.metadata[0].first == "FORMAT");
    REQUIRE(blueprint.metadata[0].second == "1A");
    REQUIRE(blueprint.metadata[1].first == "meta");
    REQUIRE(blueprint.metadata[1].second == "verse");

    REQUIRE(blueprint.name == "Snowcrash API");
    REQUIRE(blueprint.description == "## Character\n\nUncle Enzo\n\n");
    REQUIRE(blueprint.resourceGroups.size() == 2);

    REQUIRE(blueprint.resourceGroups[0].name == "First");
    REQUIRE(blueprint.resourceGroups[0].description == "p1\n");
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);

    REQUIRE(blueprint.resourceGroups[1].name == "Second");
    REQUIRE(blueprint.resourceGroups[1].description == "p2\n");
    REQUIRE(blueprint.resourceGroups[1].resources.empty());
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

    Blueprint blueprint;
    Report report;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(blueprint.name == "API");
    REQUIRE(blueprint.description == "A\n");
    REQUIRE(blueprint.resourceGroups.size() == 1);

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

    Blueprint blueprint;
    Report report;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(blueprint.name.empty());
    REQUIRE(blueprint.description == "A\n\n# B\n");
    REQUIRE(blueprint.resourceGroups.size() == 0);
}

TEST_CASE("Parse nameless blueprint with a list description", "[blueprint]")
{
    mdp::ByteBuffer source = "+ List\n";

    Blueprint blueprint;
    Report report;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(blueprint.name.empty());
    REQUIRE(blueprint.description == "+ List\n");
    REQUIRE(blueprint.resourceGroups.size() == 0);
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

    Blueprint blueprint;
    Report report;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 2); // groups with same name & no response

    REQUIRE(blueprint.resourceGroups.size() == 2);

    REQUIRE(blueprint.resourceGroups[0].name == "Name");
    REQUIRE(blueprint.resourceGroups[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);

    REQUIRE(blueprint.resourceGroups[1].name == "Name");
    REQUIRE(blueprint.resourceGroups[1].description.empty());
}

TEST_CASE("Test parser options - required blueprint name", "[blueprint]")
{
    mdp::ByteBuffer source = "Lorem Ipsum";

    Blueprint blueprint;
    Report report;

    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint);
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint, Symbols(), RequireBlueprintNameOption);
    REQUIRE(report.error.code != Error::OK);
}

TEST_CASE("Test required blueprint name on blueprint that starts with metadata", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "meta: data\n"\
    "foo:bar\n\n"\
    "Hello";

    Blueprint blueprint;
    Report report;

    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint, Symbols(), RequireBlueprintNameOption);
    REQUIRE(report.error.code != Error::OK);
}

TEST_CASE("Should parse nested lists in description", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# API\n"\
    "+ List\n"\
    "   + Nested Item\n";

    Blueprint blueprint;
    Report report;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(blueprint.name == "API");
    REQUIRE(blueprint.description == "+ List\n   + Nested Item\n");
    REQUIRE(blueprint.resourceGroups.empty());
}

TEST_CASE("Should parse paragraph without final newline", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# API\n"\
    "Lorem Ipsum";

    Blueprint blueprint;
    Report report;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(blueprint.name == "API");
    REQUIRE(blueprint.description == "Lorem Ipsum");
    REQUIRE(blueprint.resourceGroups.empty());
}

TEST_CASE("Blueprint starting with Resource Group should be parsed", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# Group Posts\n"\
    "## /posts";

    Blueprint blueprint;
    Report report;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(blueprint.name.empty());
    REQUIRE(blueprint.description.empty());
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups.front().name == "Posts");
    REQUIRE(blueprint.resourceGroups.front().resources.size() == 1);
    REQUIRE(blueprint.resourceGroups.front().resources.front().uriTemplate == "/posts");
}

TEST_CASE("Blueprint starting with Resource should be parsed", "[blueprint]")
{
    mdp::ByteBuffer source = "# /posts";

    Blueprint blueprint;
    Report report;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(blueprint.name.empty());
    REQUIRE(blueprint.description.empty());
    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups.front().name.empty());
    REQUIRE(blueprint.resourceGroups.front().resources.size() == 1);
    REQUIRE(blueprint.resourceGroups.front().resources.front().uriTemplate == "/posts");
}

TEST_CASE("Checking a resource with global resources for duplicates", "[blueprint]")
{
    mdp::ByteBuffer source = \
    "# /posts\n"\
    "# Group Posts\n"\
    "## Posts [/posts]\n"\
    "### Creat a post [POST]\n"\
    "### List posts [GET]\n";

    Blueprint blueprint;
    Report report;
    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint, Symbols(), 0, &blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 3); // 2x no response & duplicate resource

    REQUIRE(blueprint.name.empty());
    REQUIRE(blueprint.description.empty());
    REQUIRE(blueprint.resourceGroups.size() == 2);

    REQUIRE(blueprint.resourceGroups[0].name.empty());
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].uriTemplate == "/posts");

    REQUIRE(blueprint.resourceGroups[1].name == "Posts");
    REQUIRE(blueprint.resourceGroups[1].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[1].resources[0].actions.size() == 2);
    REQUIRE(blueprint.resourceGroups[1].resources[0].uriTemplate == "/posts");
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

    Blueprint blueprint;
    Report report;

    SectionParserHelper<Blueprint, BlueprintParser>::parse(source, BlueprintSectionType, report, blueprint, Symbols(), 0, &blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // no response
    REQUIRE(report.warnings[0].code == EmptyDefinitionWarning);

    REQUIRE(blueprint.name == "S");
    REQUIRE(blueprint.description == "Hello\n\n+ Response\n\nMoar text\n\n");

    REQUIRE(blueprint.metadata.size() == 1);
    REQUIRE(blueprint.metadata[0].first == "FORMAT");
    REQUIRE(blueprint.metadata[0].second == "1A");

    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].uriTemplate == "/");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].method == "GET");
}
