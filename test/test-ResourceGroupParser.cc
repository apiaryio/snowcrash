//
//  test-ResouceGroupParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "ResourceGroupParser.h"

using namespace snowcrash;
using namespace snowcrashtest;

mdp::ByteBuffer ResourceGroupFixture = \
"# Group First\n\n"\
"Fiber Optics\n\n"\
"# My Resource [/resource/{id}]\n\n"\
"+ Model (text/plain)\n\n"\
"        X.O.\n\n"\
"+ Parameters\n"\
"    + id = `1234` (optional, number, `0000`)\n\n"\
"## My Method [GET]\n\n"\
"Method Description\n\n"\
"+ Response 200 (text/plain)\n\n"\
"        OK.\n\n";

TEST_CASE("Resource group block classifier", "[resource_group]")
{
    mdp::ByteBuffer source = ResourceGroupFixture;

    source += "# Group Second\n\n"\
    "Assembly language\n";

    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    SectionType sectionType;
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    sectionType = SectionProcessor<ResourceGroup>::sectionType(markdownAST.children().begin());
    REQUIRE(sectionType == ResourceGroupSectionType);
    sectionType = SectionProcessor<ResourceGroup>::sectionType(markdownAST.children().begin() + 8);
    REQUIRE(sectionType == ResourceGroupSectionType);
}

TEST_CASE("Parse canonical resource group", "[resource_group]")
{
    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(ResourceGroupFixture, ResourceGroupSectionType, resourceGroup);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.empty());

    REQUIRE(resourceGroup.node.name == "First");
    REQUIRE(resourceGroup.node.description == "Fiber Optics\n\n");
    REQUIRE(resourceGroup.node.resources.size() == 1);
    REQUIRE(resourceGroup.node.resources.front().uriTemplate == "/resource/{id}");
    REQUIRE(resourceGroup.node.resources.front().name == "My Resource");

    REQUIRE(resourceGroup.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(resourceGroup.sourceMap.name.sourceMap[0].location == 0);
    REQUIRE(resourceGroup.sourceMap.name.sourceMap[0].length == 15);
    REQUIRE(resourceGroup.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(resourceGroup.sourceMap.description.sourceMap[0].location == 15);
    REQUIRE(resourceGroup.sourceMap.description.sourceMap[0].length == 14);
    REQUIRE(resourceGroup.sourceMap.resources.collection.size() == 1);
    REQUIRE(resourceGroup.sourceMap.resources.collection[0].name.sourceMap.size() == 1);
    REQUIRE(resourceGroup.sourceMap.resources.collection[0].name.sourceMap[0].location == 29);
    REQUIRE(resourceGroup.sourceMap.resources.collection[0].name.sourceMap[0].length == 32);
}

TEST_CASE("Parse resource group with empty resource", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group Name\n"\
    "p1\n"\
    "## /resource";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, resourceGroup);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.empty());

    REQUIRE(resourceGroup.node.name == "Name");
    REQUIRE(resourceGroup.node.description == "p1\n");
    REQUIRE(resourceGroup.node.resources.size() == 1);
    REQUIRE(resourceGroup.node.resources.front().uriTemplate == "/resource");

    REQUIRE(resourceGroup.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(resourceGroup.sourceMap.name.sourceMap[0].location == 0);
    REQUIRE(resourceGroup.sourceMap.name.sourceMap[0].length == 13);
    REQUIRE(resourceGroup.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(resourceGroup.sourceMap.description.sourceMap[0].location == 13);
    REQUIRE(resourceGroup.sourceMap.description.sourceMap[0].length == 3);
    REQUIRE(resourceGroup.sourceMap.resources.collection.size() == 1);
    REQUIRE(resourceGroup.sourceMap.resources.collection[0].uriTemplate.sourceMap.size() == 1);
    REQUIRE(resourceGroup.sourceMap.resources.collection[0].uriTemplate.sourceMap[0].location == 16);
    REQUIRE(resourceGroup.sourceMap.resources.collection[0].uriTemplate.sourceMap[0].length == 12);
}

TEST_CASE("Parse multiple resource in anonymous group", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group\n"\
    "## /r1\n"\
    "p1\n"\
    "## /r2\n"\
    "p2\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, resourceGroup);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.empty());

    REQUIRE(resourceGroup.node.name.empty());
    REQUIRE(resourceGroup.node.description.empty());
    REQUIRE(resourceGroup.node.resources.size() == 2);
    REQUIRE(resourceGroup.node.resources[0].uriTemplate == "/r1");
    REQUIRE(resourceGroup.node.resources[0].description == "p1\n");
    REQUIRE(resourceGroup.node.resources[1].uriTemplate == "/r2");
    REQUIRE(resourceGroup.node.resources[1].description == "p2\n");

    REQUIRE(resourceGroup.sourceMap.name.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.description.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.resources.collection.size() == 2);
}

TEST_CASE("Parse multiple resources with payloads", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group\n"\
    "## /1\n"\
    "### GET\n"\
    "+ Request\n\n"\
    "## /2\n"\
    "### GET\n"\
    "+ Request\n\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, resourceGroup);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.size() == 4);

    REQUIRE(resourceGroup.node.name.empty());
    REQUIRE(resourceGroup.node.description.empty());
    REQUIRE(resourceGroup.node.resources.size() == 2);

    Resource resource1 = resourceGroup.node.resources[0];
    REQUIRE(resource1.uriTemplate == "/1");
    REQUIRE(resource1.description.empty());
    REQUIRE(resource1.actions.size() == 1);
    REQUIRE(resource1.actions[0].method == "GET");
    REQUIRE(resource1.actions[0].description.empty());
    REQUIRE(resource1.actions[0].examples.size() == 1);
    REQUIRE(resource1.actions[0].examples[0].requests.size() == 1);
    REQUIRE(resource1.actions[0].examples[0].requests[0].name.empty());
    REQUIRE(resource1.actions[0].examples[0].requests[0].description.empty());
    REQUIRE(resource1.actions[0].examples[0].requests[0].body.empty());
    REQUIRE(resource1.actions[0].examples[0].responses.empty());

    Resource resource2 = resourceGroup.node.resources[1];
    REQUIRE(resource2.uriTemplate == "/2");
    REQUIRE(resource2.description.empty());
    REQUIRE(resource2.actions.size() == 1);
    REQUIRE(resource2.actions[0].method == "GET");
    REQUIRE(resource2.actions[0].description.empty());
    REQUIRE(resource2.actions[0].examples.size() == 1);
    REQUIRE(resource2.actions[0].examples[0].requests[0].name.empty());
    REQUIRE(resource2.actions[0].examples[0].requests[0].description.empty());
    REQUIRE(resource2.actions[0].examples[0].requests[0].body.empty());
    REQUIRE(resource2.actions[0].examples[0].responses.empty());

    REQUIRE(resourceGroup.sourceMap.name.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.description.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.resources.collection.size() == 2);
}

TEST_CASE("Parse multiple resources with the same name", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group\n"\
    "## /r1\n"\
    "## /r1\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, resourceGroup);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.size() == 1);
    REQUIRE(resourceGroup.report.warnings[0].code == DuplicateWarning);

    REQUIRE(resourceGroup.node.name.empty());
    REQUIRE(resourceGroup.node.description.empty());
    REQUIRE(resourceGroup.node.resources.size() == 2);

    REQUIRE(resourceGroup.sourceMap.name.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.description.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.resources.collection.size() == 2);
}

TEST_CASE("Parse resource with list in its description", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group\n"\
    "## /1\n"\
    "### GET\n"\
    "+ Request\n"\
    "    Hello\n"\
    "+ Lorem Ipsum\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, resourceGroup);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.size() == 3);   // preformatted asset & ignoring unrecognized node & no response

    REQUIRE(resourceGroup.node.name.empty());
    REQUIRE(resourceGroup.node.description.empty());
    REQUIRE(resourceGroup.node.resources.size() == 1);
    REQUIRE(resourceGroup.node.resources[0].uriTemplate == "/1");
    REQUIRE(resourceGroup.node.resources[0].description.empty());
    REQUIRE(resourceGroup.node.resources[0].actions.size() == 1);
    REQUIRE(resourceGroup.node.resources[0].actions[0].method == "GET");
    REQUIRE(resourceGroup.node.resources[0].actions[0].description == "");
    REQUIRE(resourceGroup.node.resources[0].actions[0].examples.size() == 1);

    REQUIRE(resourceGroup.sourceMap.name.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.description.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.resources.collection.size() == 1);
}

TEST_CASE("Parse resource groups with hr in description", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group 1\n"\
    "---\n"\
    "A\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, resourceGroup);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.empty());

    REQUIRE(resourceGroup.node.name == "1");
    REQUIRE(resourceGroup.node.description == "---\n\nA\n");
    REQUIRE(resourceGroup.node.resources.empty());

    REQUIRE(resourceGroup.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(resourceGroup.sourceMap.name.sourceMap[0].location == 0);
    REQUIRE(resourceGroup.sourceMap.name.sourceMap[0].length == 10);
    REQUIRE(resourceGroup.sourceMap.description.sourceMap.size() == 1);
    REQUIRE(resourceGroup.sourceMap.description.sourceMap[0].location == 10);
    REQUIRE(resourceGroup.sourceMap.description.sourceMap[0].length == 6);
    REQUIRE(resourceGroup.sourceMap.resources.collection.empty());
}

TEST_CASE("Make sure method followed by a group does not eat the group", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group One\n"\
    "## /1\n"\
    "### POST\n"\
    "# Group Two\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, resourceGroup);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.size() == 1); // no response

    REQUIRE(resourceGroup.node.name == "One");
    REQUIRE(resourceGroup.node.description.empty());
    REQUIRE(resourceGroup.node.resources.size() == 1);
    REQUIRE(resourceGroup.node.resources[0].uriTemplate == "/1");
    REQUIRE(resourceGroup.node.resources[0].actions.size() == 1);
    REQUIRE(resourceGroup.node.resources[0].actions[0].method == "POST");
    REQUIRE(resourceGroup.node.resources[0].actions[0].description.empty());

    REQUIRE(resourceGroup.sourceMap.name.sourceMap.size() == 1);
    REQUIRE(resourceGroup.sourceMap.name.sourceMap[0].location == 0);
    REQUIRE(resourceGroup.sourceMap.name.sourceMap[0].length == 12);
    REQUIRE(resourceGroup.sourceMap.description.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.resources.collection.size() == 1);
}

TEST_CASE("Parse resource method abbreviation followed by a foreign method", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# GET /resource\n"\
    "# POST\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, resourceGroup);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.size() == 2); // no response && unexpected action POST
    REQUIRE(resourceGroup.report.warnings[0].code == EmptyDefinitionWarning);
    REQUIRE(resourceGroup.report.warnings[1].code == IgnoringWarning);

    REQUIRE(resourceGroup.node.name.empty());
    REQUIRE(resourceGroup.node.description.empty());
    REQUIRE(resourceGroup.node.resources.size() == 1);
    REQUIRE(resourceGroup.node.resources[0].name.empty());
    REQUIRE(resourceGroup.node.resources[0].uriTemplate == "/resource");
    REQUIRE(resourceGroup.node.resources[0].model.name.empty());
    REQUIRE(resourceGroup.node.resources[0].model.body.empty());
    REQUIRE(resourceGroup.node.resources[0].actions.size() == 1);
    REQUIRE(resourceGroup.node.resources[0].actions[0].method == "GET");

    REQUIRE(resourceGroup.sourceMap.name.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.description.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.resources.collection.size() == 1);
}

TEST_CASE("Parse resource method abbreviation followed by another", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# GET /resource\n"\
    "# POST /2\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, resourceGroup);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.size() == 2); // 2x no response
    REQUIRE(resourceGroup.report.warnings[0].code == EmptyDefinitionWarning);
    REQUIRE(resourceGroup.report.warnings[1].code == EmptyDefinitionWarning);

    REQUIRE(resourceGroup.node.name.empty());
    REQUIRE(resourceGroup.node.description.empty());
    REQUIRE(resourceGroup.node.resources.size() == 2);
    REQUIRE(resourceGroup.node.resources[0].name.empty());
    REQUIRE(resourceGroup.node.resources[0].uriTemplate == "/resource");
    REQUIRE(resourceGroup.node.resources[0].model.name.empty());
    REQUIRE(resourceGroup.node.resources[0].model.body.empty());
    REQUIRE(resourceGroup.node.resources[0].actions.size() == 1);
    REQUIRE(resourceGroup.node.resources[0].actions[0].method == "GET");
    REQUIRE(resourceGroup.node.resources[1].name.empty());
    REQUIRE(resourceGroup.node.resources[1].uriTemplate == "/2");
    REQUIRE(resourceGroup.node.resources[1].model.name.empty());
    REQUIRE(resourceGroup.node.resources[1].model.body.empty());
    REQUIRE(resourceGroup.node.resources[1].actions.size() == 1);
    REQUIRE(resourceGroup.node.resources[1].actions[0].method == "POST");

    REQUIRE(resourceGroup.sourceMap.name.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.description.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.resources.collection.size() == 2);
    REQUIRE(resourceGroup.sourceMap.resources.collection[0].actions.collection.size() == 1);
    REQUIRE(resourceGroup.sourceMap.resources.collection[1].actions.collection.size() == 1);
}
