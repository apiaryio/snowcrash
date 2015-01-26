//
//  test-ResourceParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "ResourceParser.h"
#include "snowcrash.h"

using namespace snowcrash;
using namespace snowcrashtest;

mdp::ByteBuffer ResourceFixture = \
"# My Resource [/resource/{id}{?limit}]\n\n"\
"Awesome description\n\n"\
"+ Resource Model (text/plain)\n\n"\
"        X.O.\n\n"\
"+ Parameters\n"\
"    + id = `1234` (optional, number, `0000`)\n\n"\
"        Lorem ipsum\n"\
"        + Values\n"\
"            + `1234`\n"\
"            + `0000`\n"\
"            + `beef`\n"\
"    + limit\n\n"\
"## My Method [GET]\n\n"\
"Method Description\n\n"\
"+ Response 200 (text/plain)\n\n"\
"        OK.";

TEST_CASE("Resource block classifier", "[resource]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(ResourceFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<Resource>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ResourceSectionType);

    // Nameless resource: "/resource"
    markdownAST.children().front().text = "/resource";
    sectionType = SectionProcessor<Resource>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ResourceSectionType);

    // Keyword "group"
    markdownAST.children().front().text = "Group A";
    sectionType = SectionProcessor<Resource>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == UndefinedSectionType);

    // Resource Method
    markdownAST.children().front().text = "GET /resource";
    sectionType = SectionProcessor<Resource>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ResourceSectionType);
}

TEST_CASE("Parse resource", "[resource]")
{
    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(ResourceFixture, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.empty());

    REQUIRE(resource.node.name == "My Resource");
    REQUIRE(resource.node.uriTemplate == "/resource/{id}{?limit}");
    REQUIRE(resource.node.description == "Awesome description\n\n");
    REQUIRE(resource.node.headers.empty());

    REQUIRE(resource.node.model.name == "Resource");
    REQUIRE(resource.node.model.assets.body.source == "X.O.\n");

    REQUIRE(resource.node.parameters.size() == 2);
    REQUIRE(resource.node.parameters[0].name == "id");
    REQUIRE(resource.node.parameters[0].description == "Lorem ipsum\n");
    REQUIRE(resource.node.parameters[1].name == "limit");

    REQUIRE(resource.node.actions.size() == 1);
    REQUIRE(resource.node.actions.front().method == "GET");

    REQUIRE(resource.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.name.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.name.sourceMap[0].length == 40);
    REQUIRE(resource.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.description.sourceMap[0].location == 40);
    REQUIRE(resource.sourceMap.description.sourceMap[0].length == 21);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 40);
    REQUIRE(resource.sourceMap.headers.collection.size() == 0);
    REQUIRE(resource.sourceMap.parameters.collection.size() == 2);
    REQUIRE(resource.sourceMap.parameters.collection[0].name.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.parameters.collection[0].name.sourceMap[0].location == 125);
    REQUIRE(resource.sourceMap.parameters.collection[0].name.sourceMap[0].length == 40);
    REQUIRE(resource.sourceMap.parameters.collection[1].name.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.parameters.collection[1].name.sourceMap[0].location == 271);
    REQUIRE(resource.sourceMap.parameters.collection[1].name.sourceMap[0].length == 6);
    REQUIRE(resource.sourceMap.model.name.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.model.name.sourceMap[0].location == 63);
    REQUIRE(resource.sourceMap.model.name.sourceMap[0].length == 29);
    REQUIRE(resource.sourceMap.model.assets.body.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.model.assets.body.sourceMap[0].location == 96);
    REQUIRE(resource.sourceMap.model.assets.body.sourceMap[0].length == 9);
    REQUIRE(resource.sourceMap.actions.collection.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap[0].location == 278);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap[0].length == 20);
}

TEST_CASE("Parse partially defined resource", "[resource]")
{
    mdp::ByteBuffer source = \
    "# /1\n"\
    "## GET\n"\
    "+ Request\n"\
    "p1\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.size() == 2); // no response & preformatted asset
    REQUIRE(resource.report.warnings[0].code == IndentationWarning);
    REQUIRE(resource.report.warnings[1].code == EmptyDefinitionWarning);

    REQUIRE(resource.node.name.empty());
    REQUIRE(resource.node.uriTemplate == "/1");
    REQUIRE(resource.node.description.empty());
    REQUIRE(resource.node.model.name.empty());
    REQUIRE(resource.node.model.assets.body.source.empty());
    REQUIRE(resource.node.actions.size() == 1);
    REQUIRE(resource.node.actions.front().method == "GET");
    REQUIRE(resource.node.actions.front().description.empty());
    REQUIRE(!resource.node.actions.front().examples.empty());
    REQUIRE(resource.node.actions.front().examples.front().requests.size() == 1);
    REQUIRE(resource.node.actions.front().examples.front().requests.front().name.empty());
    REQUIRE(resource.node.actions.front().examples.front().requests.front().description.empty());
    REQUIRE(resource.node.actions.front().examples.front().requests.front().assets.body.source == "p1\n\n");

    REQUIRE(resource.sourceMap.name.sourceMap.empty());
    REQUIRE(resource.sourceMap.description.sourceMap.empty());
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 5);
    REQUIRE(resource.sourceMap.actions.collection.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap[0].location == 5);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap[0].length == 7);
}

TEST_CASE("Parse multiple method descriptions", "[resource]")
{
    mdp::ByteBuffer source = \
    "# /1\n"\
    "# GET\n"\
    "p1\n"\
    "# POST\n"\
    "p2\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.size() == 2); // 2x no response

    REQUIRE(resource.node.uriTemplate == "/1");
    REQUIRE(resource.node.description.empty());
    REQUIRE(resource.node.model.name.empty());
    REQUIRE(resource.node.model.assets.body.source.empty());
    REQUIRE(resource.node.actions.size() == 2);
    REQUIRE(resource.node.actions[0].method == "GET");
    REQUIRE(resource.node.actions[0].description == "p1\n");
    REQUIRE(resource.node.actions[1].method == "POST");
    REQUIRE(resource.node.actions[1].description == "p2\n");

    REQUIRE(resource.sourceMap.name.sourceMap.empty());
    REQUIRE(resource.sourceMap.description.sourceMap.empty());
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 5);
    REQUIRE(resource.sourceMap.actions.collection.size() == 2);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap[0].location == 5);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap[0].length == 6);
    REQUIRE(resource.sourceMap.actions.collection[1].method.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[1].method.sourceMap[0].location == 14);
    REQUIRE(resource.sourceMap.actions.collection[1].method.sourceMap[0].length == 7);
}

TEST_CASE("Parse multiple methods", "[resource]")
{
    mdp::ByteBuffer source = \
    "# /1\n"\
    "A\n"\
    "## GET\n"\
    "B\n"\
    "+ Response 200\n"\
    "    + Body\n\n"\
    "            Code 1\n\n"\
    "## POST\n"\
    "C\n"\
    "+ Request D\n"\
    "+ Response 200\n"\
    "    + Body\n\n"
    "            {}\n\n"\
    "## PUT\n"\
    "E\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.size() == 2); // empty reuqest asset & no response

    REQUIRE(resource.node.uriTemplate == "/1");
    REQUIRE(resource.node.description == "A\n");
    REQUIRE(resource.node.model.name.empty());
    REQUIRE(resource.node.model.assets.body.source.empty());
    REQUIRE(resource.node.actions.size() == 3);

    REQUIRE(resource.node.actions[0].method == "GET");
    REQUIRE(resource.node.actions[0].description == "B\n");
    REQUIRE(resource.node.actions[0].examples.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].requests.empty());
    REQUIRE(resource.node.actions[0].examples[0].responses.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].responses[0].name == "200");
    REQUIRE(resource.node.actions[0].examples[0].responses[0].description.empty());
    REQUIRE(resource.node.actions[0].examples[0].responses[0].assets.body.source == "Code 1\n");

    REQUIRE(resource.node.actions[1].method == "POST");
    REQUIRE(resource.node.actions[1].description == "C\n");
    REQUIRE(resource.node.actions[1].examples.size() == 1);
    REQUIRE(resource.node.actions[1].examples[0].requests.size() == 1);
    REQUIRE(resource.node.actions[1].examples[0].requests[0].name == "D");
    REQUIRE(resource.node.actions[1].examples[0].requests[0].description.empty());
    REQUIRE(resource.node.actions[1].examples[0].requests[0].description.empty());
    REQUIRE(resource.node.actions[1].examples[0].responses.size() == 1);
    REQUIRE(resource.node.actions[1].examples[0].responses[0].name == "200");
    REQUIRE(resource.node.actions[1].examples[0].responses[0].description.empty());
    REQUIRE(resource.node.actions[1].examples[0].responses[0].assets.body.source == "{}\n");

    REQUIRE(resource.node.actions[2].method == "PUT");
    REQUIRE(resource.node.actions[2].description == "E\n");
    REQUIRE(resource.node.actions[2].examples.empty());

    REQUIRE(resource.sourceMap.name.sourceMap.empty());
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 5);
    REQUIRE(resource.sourceMap.actions.collection.size() == 3);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap[0].location == 7);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap[0].length == 7);
    REQUIRE(resource.sourceMap.actions.collection[1].method.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[1].method.sourceMap[0].location == 63);
    REQUIRE(resource.sourceMap.actions.collection[1].method.sourceMap[0].length == 8);
    REQUIRE(resource.sourceMap.actions.collection[2].method.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[2].method.sourceMap[0].location == 128);
    REQUIRE(resource.sourceMap.actions.collection[2].method.sourceMap[0].length == 7);
}

TEST_CASE("Parse description with list", "[resource]")
{
    mdp::ByteBuffer source = \
    "# /1\n"\
    "+ A\n"\
    "+ B\n\n"\
    "p1\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.empty());

    REQUIRE(resource.node.uriTemplate == "/1");
    REQUIRE(resource.node.description == "+ A\n\n+ B\n\np1\n");
    REQUIRE(resource.node.model.name.empty());
    REQUIRE(resource.node.model.assets.body.source.empty());
    REQUIRE(resource.node.actions.empty());

    REQUIRE(resource.sourceMap.name.sourceMap.empty());
    REQUIRE(resource.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.description.sourceMap[0].location == 5);
    REQUIRE(resource.sourceMap.description.sourceMap[0].length == 12);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 5);
    REQUIRE(resource.sourceMap.actions.collection.empty());
}

TEST_CASE("Parse resource with a HR", "[resource][block]")
{
    mdp::ByteBuffer source = \
    "# /1\n"\
    "A\n"\
    "---\n"\
    "B\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.empty());

    REQUIRE(resource.node.uriTemplate == "/1");
    REQUIRE(resource.node.description == "A\n---\n\nB\n");
    REQUIRE(resource.node.model.name.empty());
    REQUIRE(resource.node.model.assets.body.source.empty());
    REQUIRE(resource.node.actions.empty());

    REQUIRE(resource.sourceMap.name.sourceMap.empty());
    REQUIRE(resource.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.description.sourceMap[0].location == 5);
    REQUIRE(resource.sourceMap.description.sourceMap[0].length == 8);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 5);
    REQUIRE(resource.sourceMap.actions.collection.empty());
}

TEST_CASE("Parse resource method abbreviation", "[resource]")
{
    mdp::ByteBuffer source = \
    "# GET /resource\n"\
    "Description\n"\
    "+ Response 200\n"\
    "    + Body\n\n"\
    "            {}\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.empty());

    REQUIRE(resource.node.name.empty());
    REQUIRE(resource.node.uriTemplate == "/resource");
    REQUIRE(resource.node.actions.size() == 1);
    REQUIRE(resource.node.actions[0].method == "GET");
    REQUIRE(resource.node.actions[0].description == "Description\n");

    REQUIRE(resource.node.actions[0].examples[0].responses.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].responses[0].description.empty());
    REQUIRE(resource.node.actions[0].examples[0].responses[0].assets.body.source == "{}\n");

    REQUIRE(resource.sourceMap.name.sourceMap.empty());
    REQUIRE(resource.sourceMap.description.sourceMap.empty());
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 16);
    REQUIRE(resource.sourceMap.actions.collection.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.actions.collection[0].method.sourceMap[0].length == 16);
}

TEST_CASE("Parse resource without name", "[resource]")
{
    mdp::ByteBuffer source = "# /resource\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.empty());

    REQUIRE(resource.node.uriTemplate == "/resource");
    REQUIRE(resource.node.name.empty());
    REQUIRE(resource.node.model.name.empty());
    REQUIRE(resource.node.model.assets.body.source.empty());
    REQUIRE(resource.node.actions.size() == 0);

    REQUIRE(resource.sourceMap.name.sourceMap.empty());
    REQUIRE(resource.sourceMap.description.sourceMap.empty());
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 12);
    REQUIRE(resource.sourceMap.actions.collection.empty());
}

TEST_CASE("Warn about parameters not in URI template", "[resource][source]")
{
    mdp::ByteBuffer source = \
    "# /resource/{id}\n"\
    "+ Parameters\n"\
    "    + olive\n\n"\
    "## GET\n"\
    "+ Parameters\n"\
    "    + cheese\n"\
    "    + id\n\n"\
    "+ Response 204\n\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.size() == 2);
    REQUIRE(resource.report.warnings[0].code == LogicalErrorWarning);
    REQUIRE(resource.report.warnings[1].code == LogicalErrorWarning);

    REQUIRE(resource.node.parameters.size() == 1);
    REQUIRE(resource.node.parameters[0].name == "olive");
    REQUIRE(resource.node.actions.size() == 1);
    REQUIRE(resource.node.actions[0].examples.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].responses.size() == 1);

    REQUIRE(resource.sourceMap.name.sourceMap.empty());
    REQUIRE(resource.sourceMap.description.sourceMap.empty());
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 17);
    REQUIRE(resource.sourceMap.parameters.collection.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection.size() == 1);
}

TEST_CASE("Parse nameless resource with named model", "[resource][model][source]")
{
    mdp::ByteBuffer source = \
    "# /message\n"\
    "+ Super Model\n"\
    "\n"\
    "        AAA\n"\
    "\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.empty());

    REQUIRE(resource.node.model.name == "Super");
    REQUIRE(resource.node.model.assets.body.source == "AAA\n");
    REQUIRE(resource.node.actions.empty());

    REQUIRE(resource.sourceMap.name.sourceMap.empty());
    REQUIRE(resource.sourceMap.description.sourceMap.empty());
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 11);
    REQUIRE(resource.sourceMap.model.name.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.model.name.sourceMap[0].location == 13);
    REQUIRE(resource.sourceMap.model.name.sourceMap[0].length == 13);
    REQUIRE(resource.sourceMap.parameters.collection.empty());
    REQUIRE(resource.sourceMap.actions.collection.empty());
}

TEST_CASE("Parse nameless resource with nameless model", "[resource][model][source]")
{
    mdp::ByteBuffer source = \
    "# /message\n"\
    "+ Model\n"\
    "\n"\
    "        AAA\n"\
    "\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == ModelError);
    REQUIRE(resource.report.warnings.empty());

    REQUIRE(resource.node.model.name.empty());

    REQUIRE(resource.sourceMap.name.sourceMap.empty());
    REQUIRE(resource.sourceMap.description.sourceMap.empty());
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 11);
    REQUIRE(resource.sourceMap.model.name.sourceMap.empty());
    REQUIRE(resource.sourceMap.parameters.collection.empty());
    REQUIRE(resource.sourceMap.actions.collection.empty());
}

TEST_CASE("Parse named resource with nameless model", "[resource][model][source]")
{
    mdp::ByteBuffer source = \
    "# Message [/message]\n"\
    "+ Model\n\n"\
    "        AAA\n"\
    "\n"\
    "## Retrieve a message [GET]\n"\
    "+ Response 200\n\n"\
    "    [Message][]\n\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.empty());

    REQUIRE(resource.node.model.name == "Message");
    REQUIRE(resource.node.model.assets.body.source == "AAA\n");
    REQUIRE(resource.node.actions.size() == 1);
    REQUIRE(resource.node.actions[0].name == "Retrieve a message");
    REQUIRE(resource.node.actions[0].method == "GET");
    REQUIRE(resource.node.actions[0].examples.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].responses.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].responses[0].name == "200");
    REQUIRE(resource.node.actions[0].examples[0].responses[0].assets.body.source == "AAA\n");

    REQUIRE(resource.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.name.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.name.sourceMap[0].length == 21);
    REQUIRE(resource.sourceMap.description.sourceMap.empty());
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 21);
    REQUIRE(resource.sourceMap.parameters.collection.empty());
    REQUIRE(resource.sourceMap.actions.collection.size() == 1);
    REQUIRE(resource.sourceMap.model.name.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.model.name.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.model.name.sourceMap[0].length == 21);
}

TEST_CASE("Parse model with unrecognised resource", "[resource][model]")
{
    mdp::ByteBuffer source = \
    "# Resource [/1]\n\n"\
    "+ Model (plain/text)\n\n"\
    "        AAA\n\n"\
    "## Retrieve a resource [GET]\n\n"\
    "+ Response 200\n\n"\
    "    + Headers\n\n"\
    "            X-Header: A\n\n"\
    "    + Body\n\n"\
    "            [Resource][]";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.size() == 1);
    REQUIRE(resource.report.warnings[0].code == IgnoringWarning);

    REQUIRE(resource.node.model.name == "Resource");
    REQUIRE(resource.node.model.assets.body.source == "AAA\n");
    REQUIRE(resource.node.actions.size() == 1);
    REQUIRE(resource.node.actions[0].name == "Retrieve a resource");
    REQUIRE(resource.node.actions[0].method == "GET");
    REQUIRE(resource.node.actions[0].examples.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].responses.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].responses[0].name == "200");
    REQUIRE(resource.node.actions[0].examples[0].responses[0].assets.body.source == "[Resource][]\n");
    REQUIRE(resource.node.actions[0].examples[0].responses[0].description == "");
}

TEST_CASE("Parse named resource with lazy referencing", "[resource][model][issue][#84]")
{
    mdp::ByteBuffer source = \
    "#api name\n\n"\
    "# Resource 1 [/1]\n"\
    "## Retrieve [GET]\n\n"\
    "+ Response 200\n\n"\
    "    [Resource 2][]\n\n"\
    "# Resource 2 [/2]\n"\
    "+ Model (text/plain)\n\n"\
    "        `resource model` 2\n";

    ParseResult<Blueprint> blueprint;
    parse(source, ExportSourcemapOption, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.name == "api name");
    REQUIRE(blueprint.node.description == "");

    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 2);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].uriTemplate == "/1");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].name == "Resource 1");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].name == "Resource 1");

    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].method == "GET");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].name == "Retrieve");

    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].assets.body.source == "`resource model` 2\n");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[0].first == "Content-Type");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[0].second == "text/plain");

    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].reference.id == "Resource 2");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].reference.type == Reference::ModelReference);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].reference.meta.state == Reference::StateResolved);

    SourceMap<Resources> resourcesSourceMap = blueprint.sourceMap.resourceGroups.collection[0].resources;
    SourceMap<TransactionExamples> examplesSourceMap = resourcesSourceMap.collection[0].actions.collection[0].examples;

    REQUIRE(examplesSourceMap.collection[0].responses.collection[0].headers.collection[0].sourceMap.size() == 1);
    REQUIRE(examplesSourceMap.collection[0].responses.collection[0].headers.collection[0].sourceMap[0].length == 20);
    REQUIRE(examplesSourceMap.collection[0].responses.collection[0].headers.collection[0].sourceMap[0].location == 104);

    REQUIRE(examplesSourceMap.collection[0].responses.collection[0].reference.sourceMap.size() == 1);
    REQUIRE(examplesSourceMap.collection[0].responses.collection[0].reference.sourceMap[0].length == 15);
    REQUIRE(examplesSourceMap.collection[0].responses.collection[0].reference.sourceMap[0].location == 68);
}

TEST_CASE("Parse named resource with lazy referencing with both response and request", "[resource][model]")
{
    mdp::ByteBuffer source = \
    "# API\n"\
    "\n"\
    "# Collection of Items [/items]\n"\
    "+ Model (application/json)\n"\
    "\n"\
    "        [ { item 1 }, { item 2 } ]\n"\
    "\n"\
    "## Create New Item [POST]\n"\
    "+ Request\n"\
    "\n"\
    "    [Item][]\n"\
    "\n"\
    "+ Response 200\n"\
    "\n"\
    "    [Collection of Items][]\n"\
    "\n"\
    "# Item [/items/{id}]\n"\
    "+ Model (application/json)\n"\
    "\n"\
    "        { item }\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.name == "API");
    REQUIRE(blueprint.node.description == "");

    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 2);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].uriTemplate == "/items");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].name == "Collection of Items");

    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].method == "POST");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].name == "Create New Item");

    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].requests.size() == 1);

    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].name == "");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].assets.body.source == "{ item }\n");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].headers.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].headers[0].first == "Content-Type");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].headers[0].second == "application/json");

    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].reference.id == "Item");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].reference.type == Reference::ModelReference);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].requests[0].reference.meta.state == Reference::StateResolved);

    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].assets.body.source == "[ { item 1 }, { item 2 } ]\n");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers.size() == 1);

    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].reference.id == "Collection of Items");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].reference.type == Reference::ModelReference);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].reference.meta.state == Reference::StateResolved);
}

TEST_CASE("Expect to have a warning when 100 responce's reference has a body", "[resource][model]")
{
    mdp::ByteBuffer source = \
    "# API\n"\
    "\n"\
    "# Collection of Items [/items]\n"\
    "+ Model (application/json)\n"\
    "\n"\
    "        [ { item 1 }, { item 2 } ]\n"\
    "\n"\
    "## Create New Item [POST]\n"\
    "+ Request\n"\
    "\n"\
    "    [Collection of Items][]\n"\
    "\n"\
    "+ Response 100\n"\
    "\n"\
    "    [Item][]\n"\
    "\n"\
    "# Item [/items/{id}]\n"\
    "+ Model (application/json)\n"\
    "\n"\
    "        { item }\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);

    REQUIRE(blueprint.report.warnings[0].code == EmptyDefinitionWarning);
}

TEST_CASE("Parse named resource with nameless model but reference a non-existing model", "[resource]")
{
    mdp::ByteBuffer source = \
    "# Posts [/posts]\n"\
    "+ Model\n\n"\
    "        {}\n"\
    "\n"\
    "## List [GET]\n"\
    "+ Response 200\n\n"\
    "    [Post][]\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == ModelError);
    REQUIRE(blueprint.report.warnings.empty());
}

TEST_CASE("Parse root resource", "[resource]")
{
    mdp::ByteBuffer source = "# API Root [/]\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.empty());

    REQUIRE(resource.node.name == "API Root");
    REQUIRE(resource.node.uriTemplate == "/");
    REQUIRE(resource.node.actions.empty());

    REQUIRE(resource.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.name.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.name.sourceMap[0].length == 15);
    REQUIRE(resource.sourceMap.description.sourceMap.empty());
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 15);
    REQUIRE(resource.sourceMap.parameters.collection.empty());
    REQUIRE(resource.sourceMap.actions.collection.empty());
}

TEST_CASE("Parse resource with invalid URI Tempalte", "[resource]")
{
    mdp::ByteBuffer source = "# Resource [/id{? limit}]\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.size() == 1);
    REQUIRE(resource.report.warnings[0].code == URIWarning);

    REQUIRE(resource.node.name == "Resource");
    REQUIRE(resource.node.uriTemplate == "/id{? limit}");
    REQUIRE(resource.node.actions.empty());

    REQUIRE(resource.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.name.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.name.sourceMap[0].length == 26);
    REQUIRE(resource.sourceMap.description.sourceMap.empty());
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 26);
    REQUIRE(resource.sourceMap.parameters.collection.empty());
    REQUIRE(resource.sourceMap.actions.collection.empty());
}

TEST_CASE("Deprecated resource and action headers", "[resource]")
{
    mdp::ByteBuffer source = \
    "# /\n"\
    "+ Headers\n\n"\
    "        header1: value1\n\n"\
    "## GET\n"\
    "+ Headers\n\n"\
    "        header2: value2\n\n"\
    "+ Response 200\n"\
    "    + Headers\n\n"\
    "            header3: value3\n\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.size() == 2);
    REQUIRE(resource.report.warnings[0].code == DeprecatedWarning);
    REQUIRE(resource.report.warnings[1].code == DeprecatedWarning);

    REQUIRE(resource.node.headers.empty());
    REQUIRE(resource.node.actions.size() == 1);
    REQUIRE(resource.node.actions[0].headers.empty());
    REQUIRE(resource.node.actions[0].examples.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].responses.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].responses[0].headers.size() == 3);
    REQUIRE(resource.node.actions[0].examples[0].responses[0].headers[0].first == "header1");
    REQUIRE(resource.node.actions[0].examples[0].responses[0].headers[0].second == "value1");
    REQUIRE(resource.node.actions[0].examples[0].responses[0].headers[1].first == "header2");
    REQUIRE(resource.node.actions[0].examples[0].responses[0].headers[1].second == "value2");
    REQUIRE(resource.node.actions[0].examples[0].responses[0].headers[2].first == "header3");
    REQUIRE(resource.node.actions[0].examples[0].responses[0].headers[2].second == "value3");

    REQUIRE(resource.sourceMap.name.sourceMap.empty());
    REQUIRE(resource.sourceMap.description.sourceMap.empty());
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].location == 0);
    REQUIRE(resource.sourceMap.uriTemplate.sourceMap[0].length == 4);
    REQUIRE(resource.sourceMap.parameters.collection.empty());
    REQUIRE(resource.sourceMap.headers.collection.empty());
    REQUIRE(resource.sourceMap.actions.collection.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].headers.collection.empty());
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection[0].headers.collection.size() == 3);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection[0].headers.collection[0].sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection[0].headers.collection[0].sourceMap[0].location == 19);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection[0].headers.collection[0].sourceMap[0].length == 20);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection[0].headers.collection[1].sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection[0].headers.collection[1].sourceMap[0].location == 62);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection[0].headers.collection[1].sourceMap[0].length == 20);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection[0].headers.collection[2].sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection[0].headers.collection[2].sourceMap[0].location == 121);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection[0].headers.collection[2].sourceMap[0].length == 20);
}

TEST_CASE("Bug fix for recognition of model as a part of other word or as a quote, issue #92 and #152", "[model]")
{
    mdp::ByteBuffer source = \
    "## Resource [/resource]\n"\
    "### Attributes\n"\
    "- A\n"\
    "- Cmodel\n"\
    "- Single data model for all exchange data\n"\
    "- `model`\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.size() == 0);

    REQUIRE(resource.node.name == "Resource");
    REQUIRE(resource.node.description == "### Attributes\n\n- A\n\n- Cmodel\n\n- Single data model for all exchange data\n\n- `model`\n");
}

TEST_CASE("Parse resource with multi-word named model", "[resource][model]")
{
    mdp::ByteBuffer source = \
    "# My Resource [/resource]\n\n"\
    "Awesome description\n\n"\
    "+ a really good name Model (text/plain)\n\n"\
    "        body of the `model`\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.empty());

    REQUIRE(resource.node.model.name == "a really good name");
    REQUIRE(resource.node.model.assets.body.source == "body of the `model`\n");
    REQUIRE(resource.node.actions.empty());
}

TEST_CASE("Dangling transaction example assets", "[resource]")
{
    mdp::ByteBuffer source = \
    "# A [/a]\n"\
    "## GET\n"\
    "+ Request A\n"\
    "\n"\
    "```js\n"\
    "dangling request body\n"\
    "```\n"\
    "\n"\
    "+ Response 200\n"\
    "\n"\
    "```\n"\
    "dangling response body\n"\
    "```\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.size() == 3);
    REQUIRE(resource.report.warnings[0].code == EmptyDefinitionWarning);
    REQUIRE(resource.report.warnings[1].code == IndentationWarning);
    REQUIRE(resource.report.warnings[2].code == IndentationWarning);

    REQUIRE(resource.node.name == "A");
    REQUIRE(resource.node.uriTemplate == "/a");
    REQUIRE(resource.node.actions.size() == 1);

    REQUIRE(resource.node.actions[0].method == "GET");
    REQUIRE(resource.node.actions[0].examples.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].requests.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].requests[0].name == "A");
    REQUIRE(resource.node.actions[0].examples[0].requests[0].assets.body.source == "dangling request body\n\n");

    REQUIRE(resource.node.actions[0].examples[0].responses.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].responses[0].name == "200");
    REQUIRE(resource.node.actions[0].examples[0].responses[0].assets.body.source == "dangling response body\n\n");

    REQUIRE(resource.sourceMap.actions.collection.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].requests.collection.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].requests.collection[0].assets.body.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].requests.collection[0].assets.body.sourceMap[0].location == 29);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].requests.collection[0].assets.body.sourceMap[0].length == 33);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection[0].assets.body.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection[0].assets.body.sourceMap[0].location == 78);
    REQUIRE(resource.sourceMap.actions.collection[0].examples.collection[0].responses.collection[0].assets.body.sourceMap[0].length == 31);
}

TEST_CASE("Body list item in description", "[resource][regression][#190]")
{
    mdp::ByteBuffer source = \
    "## GET /A\n"\
    "Lorem Ipsum\n"\
    "\n"\
    "+ Body\n"\
    "\n"\
    "    { ... }\n"\
    "\n"\
    "+ Response 200\n";

    ParseResult<Resource> resource;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.empty());

    REQUIRE(resource.node.actions.size() == 1);
    REQUIRE(resource.node.actions[0].description == "Lorem Ipsum\n\n+ Body\n\n    { ... }\n\n");

    REQUIRE(resource.sourceMap.actions.collection.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].description.sourceMap.size() == 1);
    REQUIRE(resource.sourceMap.actions.collection[0].description.sourceMap[0].location == 10);
    REQUIRE(resource.sourceMap.actions.collection[0].description.sourceMap[0].length == 34);
}

TEST_CASE("Parse resource attributes", "[resource]")
{
    mdp::ByteBuffer source = \
    "# Coupons [/coupons]\n\n"\
    "+ Attributes (array[Coupon])\n\n"\
    "## List [GET]\n\n"\
    "+ Response 200 (application/json)\n\n"\
    "  + Attributes (Coupons)";

    ParseResult<Resource> resource;
    NamedTypes namedTypes;

    NamedTypeHelper::build("Coupon", mson::ObjectBaseType, namedTypes);
    NamedTypeHelper::build("Coupons", mson::ValueBaseType, namedTypes);
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption, Models(), NULL, namedTypes);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.empty());

    REQUIRE(resource.node.name == "Coupons");
    REQUIRE(resource.node.attributes.source.name.symbol.literal == "Coupons");
    REQUIRE(resource.node.attributes.source.typeDefinition.typeSpecification.name.base == mson::ArrayTypeName);
    REQUIRE(resource.node.attributes.source.typeDefinition.typeSpecification.nestedTypes.size() == 1);
    REQUIRE(resource.node.attributes.source.typeDefinition.typeSpecification.nestedTypes[0].symbol.literal == "Coupon");

    REQUIRE(resource.node.actions.size() == 1);
    REQUIRE(resource.node.actions[0].examples.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].responses.size() == 1);
}

TEST_CASE("Parse unnamed resource attributes", "[resource]")
{
    mdp::ByteBuffer source = \
    "# /coupons\n\n"\
    "+ Attributes (array[Coupon])\n\n"\
    "## List [GET]\n\n"\
    "+ Response 200 (application/json)\n\n"\
    "  + Attributes (Coupons)";

    ParseResult<Resource> resource;
    NamedTypes namedTypes;

    NamedTypeHelper::build("Coupon", mson::ObjectBaseType, namedTypes);
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, resource, ExportSourcemapOption, Models(), NULL, namedTypes);

    REQUIRE(resource.report.error.code == Error::OK);
    REQUIRE(resource.report.warnings.size() == 1); // Unknown type 'Coupons'

    REQUIRE(resource.node.name.empty());
    REQUIRE(resource.node.attributes.source.name.empty());
    REQUIRE(resource.node.attributes.source.typeDefinition.typeSpecification.name.base == mson::ArrayTypeName);
    REQUIRE(resource.node.attributes.source.typeDefinition.typeSpecification.nestedTypes.size() == 1);
    REQUIRE(resource.node.attributes.source.typeDefinition.typeSpecification.nestedTypes[0].symbol.literal == "Coupon");

    REQUIRE(resource.node.actions.size() == 1);
    REQUIRE(resource.node.actions[0].examples.size() == 1);
    REQUIRE(resource.node.actions[0].examples[0].responses.size() == 1);
}
