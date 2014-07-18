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

    Blueprint blueprint;
    Report report;

    parse(source, 0, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(blueprint.metadata.empty());
    REQUIRE(blueprint.name.empty());
    REQUIRE(blueprint.description.empty());
    REQUIRE(blueprint.resourceGroups.empty());
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

    Blueprint blueprint;
    Report report;

    parse(source, 0, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(blueprint.name == "Snowcrash API");
    REQUIRE(blueprint.description.empty());
    REQUIRE(blueprint.resourceGroups.size() == 1);

    ResourceGroup& resourceGroup = blueprint.resourceGroups.front();
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
    Blueprint blueprint1;
    Report report1;

    parse("hello\t", 0, report1, blueprint1);

    REQUIRE(report1.error.code != Error::OK);
    REQUIRE(report1.error.location.size() == 1);
    REQUIRE(report1.error.location[0].location == 5);
    REQUIRE(report1.error.location[0].length == 1);

    Blueprint blueprint2;
    Report report2;

    snowcrash::parse("sun\n\rsalt\n\r", 0, report2, blueprint2);

    REQUIRE(report2.error.code != Error::OK);
    REQUIRE(report2.error.location.size() == 1);
    REQUIRE(report2.error.location[0].location == 4);
    REQUIRE(report2.error.location[0].length == 1);
}

TEST_CASE("Do not report duplicate response when media type differs", "[method][#14]")
{
    mdp::ByteBuffer source = \
    "# GET /message\n"\
    "+ Response 200 (application/json)\n\n"\
    "        { \"msg\": \"Hello.\" }\n\n"\
    "+ Response 200 (text/plain)\n\n"\
    "        Hello.\n";

    Blueprint blueprint;
    Report report;

    parse(source, 0, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // expected API name
}

TEST_CASE("Support description ending with an list item", "[parser][#8]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ a description item\n"\
    "+ Response 200\n\n"\
    "        ...\n";

    Blueprint blueprint;
    Report report;

    parse(source, 0, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // expected API name

    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description == "+ a description item\n");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "...\n");
}

TEST_CASE("Invalid ‘warning: empty body asset’ for certain status codes", "[parser][#13]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ Response 304\n";

    Blueprint blueprint;
    Report report;

    parse(source, 0, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // expected API name

    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "304");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body.empty());
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

    Blueprint blueprint;
    Report report;

    parse(source, 0, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 4);
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

    Blueprint blueprint;
    Report report;

    parse(source, 0, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
//    REQUIRE(report.warnings.size() == 2); // expected API name

    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 2);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "asset\n\npre\n");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[1].name == "404");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[1].body == "Not found\n");
}

TEST_CASE("Parse adjacent asset list blocks", "[parser][#9]")
{
    mdp::ByteBuffer source = \
    "# GET /1\n"\
    "+ response 200\n"\
    "+ list\n";

    Blueprint blueprint;
    Report report;

    parse(source, 0, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 2); // expected API name
    REQUIRE(report.warnings[0].code == IgnoringWarning);

    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body.empty());
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

    Blueprint blueprint;
    Report report;

    parse(source, 0, report, blueprint);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 3); // expected API name
    REQUIRE(report.warnings[0].code == IndentationWarning);
    REQUIRE(report.warnings[1].code == IndentationWarning);

    REQUIRE(blueprint.resourceGroups.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].description.empty());
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses.size() == 1);
    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].name == "200");
//    REQUIRE(blueprint.resourceGroups[0].resources[0].actions[0].examples[0].responses[0].body == "A\nB\nC\n");
}
