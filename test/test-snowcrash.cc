//
//  test-snowcrash.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#define CATCH_CONFIG_MAIN
#include "snowcrashtest.h"
#include "snowcrash.h"

using namespace snowcrash;
using namespace snowcrashtest;

TEST_CASE("Parse empty blueprint", "[parser]")
{
    mdp::ByteBuffer source = "";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.metadata.empty());
    REQUIRE(blueprint.node.name.empty());
    REQUIRE(blueprint.node.description.empty());
    REQUIRE(blueprint.node.resourceGroups.empty());
}

TEST_CASE("Parse simple blueprint", "[parser]")
{
    mdp::ByteBuffer source = \
    "# Snowcrash API \n\n"\
    "# GET /resource\n"\
    "Resource **description**\n\n"\
    "+ Response 200\n"\
    "    + Body\n\n"\
    "            Text\n\n"\
    "            { ... }\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.name == "Snowcrash API");
    REQUIRE(blueprint.node.description.empty());
    REQUIRE(blueprint.node.resourceGroups.size() == 1);

    ResourceGroup& resourceGroup = blueprint.node.resourceGroups.front();
    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 1);

    Resource& resource = resourceGroup.resources.front();
    REQUIRE(resource.uriTemplate == "/resource");
    REQUIRE(resource.actions.size() == 1);

    Action& action = resource.actions[0];
    REQUIRE(action.method == "GET");
    REQUIRE(action.description == "Resource **description**\n\n");
    REQUIRE(action.examples.size() == 1);
    REQUIRE(action.examples.front().requests.empty());
    REQUIRE(action.examples.front().responses.size() == 1);

    Response& response = action.examples.front().responses[0];
    REQUIRE(response.name == "200");
    REQUIRE(response.body == "Text\n\n{ ... }\n");
}

TEST_CASE("Parse blueprint with unsupported characters", "[parser]")
{
    ParseResult<Blueprint> blueprint1;
    parse("hello\t", 0, blueprint1);

    REQUIRE(blueprint1.report.error.code != Error::OK);
    REQUIRE(blueprint1.report.error.location.size() == 1);
    REQUIRE(blueprint1.report.error.location[0].location == 5);
    REQUIRE(blueprint1.report.error.location[0].length == 1);

    ParseResult<Blueprint> blueprint2;
    parse("sun\n\rsalt\n\r", 0, blueprint2);

    REQUIRE(blueprint2.report.error.code != Error::OK);
    REQUIRE(blueprint2.report.error.location.size() == 1);
    REQUIRE(blueprint2.report.error.location[0].location == 4);
    REQUIRE(blueprint2.report.error.location[0].length == 1);
}

TEST_CASE("Do not report duplicate response when media type differs", "[method][#14]")
{
    mdp::ByteBuffer source = \
    "# GET /message\n"\
    "+ Response 200 (application/json)\n\n"\
    "        { \"msg\": \"Hello.\" }\n\n"\
    "+ Response 200 (text/plain)\n\n"\
    "        Hello.\n";

    ParseResult<Blueprint> blueprint;

    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());
}

TEST_CASE("Support description ending with an list item", "[parser][#8]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ a description item\n"\
    "+ Response 200\n\n"\
    "        ...\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].description == "+ a description item\n");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "...\n");
}

TEST_CASE("Invalid ‘warning: empty body asset’ for certain status codes", "[parser][#13]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 304\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "304");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body.empty());
}

TEST_CASE("SIGTERM parsing blueprint", "[parser][#45]")
{
    mdp::ByteBuffer source = \
    "# A\n"\
    "# B\n"\
    "C\n\n"\
    "D\n\n"\
    "E\n\n"\
    "F\n\n"\
    "G\n\n"\
    "# /1\n"\
    "# GET\n"\
    "+ Request\n"\
    "+ Response 200\n"\
    "    + Body\n\n"\
    "            H\n\n"\
    "I\n"\
    "# J\n"\
    "> K";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 4);
}

TEST_CASE("Parse adjacent asset blocks", "[parser][#9]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ response 200\n"\
    "\n"\
    "asset\n"\
    "\n"\
    "    pre\n"\
    "+ response 404\n"\
    "\n"\
    "        Not found\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 2);

    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 2);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "asset\n\npre\n\n");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[1].name == "404");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[1].body == "Not found\n");
}

TEST_CASE("Parse adjacent asset list blocks", "[parser][#9]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ response 200\n"\
    "+ list\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == IgnoringWarning);

    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body.empty());
}

TEST_CASE("Parse adjacent nested asset blocks", "[parser][#9]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ response 200\n"\
    "    + body\n"\
    "\n"\
    "        A\n"\
    "\n"\
    "    B\n"\
    "C\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 2);
    REQUIRE(blueprint.report.warnings[0].code == IndentationWarning);
    REQUIRE(blueprint.report.warnings[1].code == IndentationWarning);

    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "A\nB\nC\n\n");
}

TEST_CASE("Exception while parsing a blueprint with leading empty space", "[regression][parser]")
{
    mdp::ByteBuffer source = \
    "\n"\
    "# PUT /branch\n";
    
    ParseResult<Blueprint> blueprint;
    
    REQUIRE_NOTHROW(parse(source, 0, blueprint));
    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == EmptyDefinitionWarning);
}

TEST_CASE("Invalid source map without closing newline", "[regression][parser]")
{
    mdp::ByteBuffer source = \
    "# PUT /branch";
    
    ParseResult<Blueprint> blueprint;
    
    REQUIRE_NOTHROW(parse(source, 0, blueprint));
    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == EmptyDefinitionWarning);
    REQUIRE(blueprint.report.warnings[0].location.size() == 1);
    REQUIRE(blueprint.report.warnings[0].location[0].location == 0);
    REQUIRE(blueprint.report.warnings[0].location[0].length == 13);
}

TEST_CASE("Warn about missing API name if there is an API description", "[parser][regression]")
{
    mdp::ByteBuffer source1 = \
    "Hello World\n";
    
    ParseResult<Blueprint> blueprint1;
    parse(source1, 0, blueprint1);
    
    REQUIRE(blueprint1.report.error.code == Error::OK);
    REQUIRE(blueprint1.report.warnings.size() == 1);
    REQUIRE(blueprint1.report.warnings[0].code == APINameWarning);
    
    REQUIRE(blueprint1.node.name.empty());
    REQUIRE(blueprint1.node.description == "Hello World\n");
    REQUIRE(blueprint1.node.resourceGroups.empty());
    
    mdp::ByteBuffer source2 = \
    "# API\n"\
    "Hello World\n";
    
    ParseResult<Blueprint> blueprint2;
    parse(source2, 0, blueprint2);
    
    REQUIRE(blueprint2.report.error.code == Error::OK);
    REQUIRE(blueprint2.report.warnings.empty());
    
    REQUIRE(blueprint2.node.name == "API");
    REQUIRE(blueprint2.node.description == "Hello World\n");
    REQUIRE(blueprint2.node.resourceGroups.empty());

    mdp::ByteBuffer source3 = \
    "# POST /1\n"\
    "+ Response 201";
    
    ParseResult<Blueprint> blueprint3;
    parse(source3, 0, blueprint3);
    
    REQUIRE(blueprint3.report.error.code == Error::OK);
    REQUIRE(blueprint3.report.warnings.empty());
}

TEST_CASE("Resource with incorrect URI segfault", "[parser][regression]")
{
    mdp::ByteBuffer source = \
    "# Group A\n"\
    "## Resource [wronguri]\n"\
    "### Retrieve [GET]\n"\
    "+ Response 200\n"\
    "\n";
    
    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());
    
    REQUIRE(blueprint.node.name.empty());
    REQUIRE(blueprint.node.description.empty());
    
    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].name == "A");
    REQUIRE(blueprint.node.resourceGroups[0].description == "## Resource [wronguri]\n\n### Retrieve [GET]\n\n+ Response 200\n\n");
    
}

TEST_CASE("Dangling block not recognized", "[parser][regression][#186]")
{
    mdp::ByteBuffer source = \
    "# A [/a]\n"\
    "+ Model\n"\
    "\n"\
    "```js\n"\
    "    { ... }\n"\
    "```\n";
    
    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == IndentationWarning);
    
    REQUIRE(blueprint.node.name.empty());
    REQUIRE(blueprint.node.description.empty());
    
    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].name == "A");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].uriTemplate == "/a");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].model.body == "    { ... }\n\n");
}

TEST_CASE("Ignoring block recovery", "[parser][regression][#188]")
{
    mdp::ByteBuffer source = \
    "## Note [/notes/{id}]\n"\
    "\n"\
    "+ Parameters\n"\
    "    + id\n"\
    "\n"\
    "+ Response 200\n"\
    "\n"\
    "### Remove a Note [DELETE]\n";
    
    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 2);
    REQUIRE(blueprint.report.warnings[0].code == IgnoringWarning);
    REQUIRE(blueprint.report.warnings[1].code == EmptyDefinitionWarning);
    
    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].name == "Note");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].name == "Remove a Note");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].method == "DELETE");
}

TEST_CASE("Ignoring dangling model assets", "[parser][regression][#196]")
{
    mdp::ByteBuffer source = \
    "# A [/A]\n"\
    "+ model (Y)\n"\
    "\n"\
    "{ A }\n"\
    "\n"\
    "## POST /B\n"\
    "+ Response 200\n"\
    "\n"\
    "    [A][]\n";
    
    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == IndentationWarning);

    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 2);
    REQUIRE(blueprint.node.resourceGroups[0].resources[1].name.empty());
    REQUIRE(blueprint.node.resourceGroups[0].resources[1].uriTemplate == "/B");
    REQUIRE(blueprint.node.resourceGroups[0].resources[1].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[1].actions[0].method == "POST");
    REQUIRE(blueprint.node.resourceGroups[0].resources[1].actions[0].examples.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[1].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[1].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.node.resourceGroups[0].resources[1].actions[0].examples[0].responses[0].body == "{ A }\n\n");
}

TEST_CASE("Ignoring local media type", "[parser][regression][#195]")
{
    mdp::ByteBuffer source = \
    "# A [/A]\n"\
    "+ model (Y)\n"\
    "\n"\
    "        { A }\n"\
    "\n"\
    "## Retrieve [GET]\n"\
    "+ Response 200 (X)\n"\
    "\n"\
    "    [A][]\n";

    
    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == IgnoringWarning);
    
    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[0].first == "Content-Type");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[0].second == "Y");
}

TEST_CASE("Using local media type", "[parser][regression][#195]")
{
    mdp::ByteBuffer source = \
    "# A [/A]\n"\
    "+ model\n"\
    "\n"\
    "        { A }\n"\
    "\n"\
    "## Retrieve [GET]\n"\
    "+ Response 200 (X)\n"\
    "\n"\
    "    [A][]\n";
    
    
    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());
    
    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[0].first == "Content-Type");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[0].second == "X");
}

TEST_CASE("Parse ill-formated header", "[parser][#198][regression]")
{
    mdp::ByteBuffer source = \
    "# GET /A\n"\
    "+ Response 200\n"\
    "    + Header\n"\
    "        Location: new_url\n";
    
    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == IndentationWarning);
    
    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[0].first == "Location");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].headers[0].second == "new_url");
}

TEST_CASE("Overshadow parameters", "[parser][#201][regression][parameters]")
{
    mdp::ByteBuffer source = \
    "# /{a,b,c}\n"\
    "\n"\
    "## GET\n"\
    "+ parameters\n"\
    "    + a ... 1\n"\
    "    + b ... 2\n"\
    "    + c ... 3\n"\
    "\n"\
    "+ parameters\n"\
    "    + a ... 4\n"\
    "\n"\
    "+ response 200\n";
    
    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.size() == 1);
    REQUIRE(blueprint.report.warnings[0].code == RedefinitionWarning);
    
    REQUIRE(blueprint.node.resourceGroups.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters.size() == 4);

    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters[0].name == "a");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters[0].description == "1");
    
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters[1].name == "b");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters[1].description == "2");

    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters[2].name == "c");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters[2].description == "3");

    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters[3].name == "a");
    REQUIRE(blueprint.node.resourceGroups[0].resources[0].actions[0].parameters[3].description == "4");
}

TEST_CASE("Segfault parsing metadata only", "[parser][#205][regression]")
{
    mdp::ByteBuffer source = \
    "FORMAT: 1A : SOJ\n";

    ParseResult<Blueprint> blueprint;
    parse(source, 0, blueprint);

    REQUIRE(blueprint.report.error.code == Error::OK);
    REQUIRE(blueprint.report.warnings.empty());

    REQUIRE(blueprint.node.metadata.size() == 1);
    REQUIRE(blueprint.node.metadata[0].first == "FORMAT");
    REQUIRE(blueprint.node.metadata[0].second == "1A : SOJ");
    REQUIRE(blueprint.node.resourceGroups.empty());
}
