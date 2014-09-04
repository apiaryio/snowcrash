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
    markdownParser.parse(source, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<ResourceGroup>::sectionType(markdownAST.children().begin()) == ResourceGroupSectionType);
    REQUIRE(SectionProcessor<ResourceGroup>::sectionType(markdownAST.children().begin() + 8) == ResourceGroupSectionType);
}

TEST_CASE("Parse canonical resource group", "[resource_group]")
{
    ResourceGroup resourceGroup;
    Report report;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(ResourceGroupFixture, ResourceGroupSectionType, report, resourceGroup);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(resourceGroup.name == "First");
    REQUIRE(resourceGroup.description == "Fiber Optics\n\n");
    REQUIRE(resourceGroup.resources.size() == 1);
    REQUIRE(resourceGroup.resources.front().uriTemplate == "/resource/{id}");
    REQUIRE(resourceGroup.resources.front().name == "My Resource");
}

TEST_CASE("Parse resource group with empty resource", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group Name\n"\
    "p1\n"\
    "## /resource";

    ResourceGroup resourceGroup;
    Report report;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, report, resourceGroup);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(resourceGroup.name == "Name");
    REQUIRE(resourceGroup.description == "p1\n");
    REQUIRE(resourceGroup.resources.size() == 1);
    REQUIRE(resourceGroup.resources.front().uriTemplate == "/resource");
}

TEST_CASE("Parse multiple resource in anonymous group", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group\n"\
    "## /r1\n"\
    "p1\n"\
    "## /r2\n"\
    "p2\n";

    ResourceGroup resourceGroup;
    Report report;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, report, resourceGroup);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 2);
    REQUIRE(resourceGroup.resources[0].uriTemplate == "/r1");
    REQUIRE(resourceGroup.resources[0].description == "p1\n");
    REQUIRE(resourceGroup.resources[1].uriTemplate == "/r2");
    REQUIRE(resourceGroup.resources[1].description == "p2\n");
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

    ResourceGroup resourceGroup;
    Report report;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, report, resourceGroup);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 4);

    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 2);

    Resource resource1 = resourceGroup.resources[0];
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

    Resource resource2 = resourceGroup.resources[1];
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
}

TEST_CASE("Parse multiple resources with the same name", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group\n"\
    "## /r1\n"\
    "## /r1\n";

    ResourceGroup resourceGroup;
    Report report;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, report, resourceGroup);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == DuplicateWarning);

    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 2);
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

    ResourceGroup resourceGroup;
    Report report;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, report, resourceGroup);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 3);   // preformatted asset & ignoring unrecognized node & no response

    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 1);
    REQUIRE(resourceGroup.resources[0].uriTemplate == "/1");
    REQUIRE(resourceGroup.resources[0].description.empty());
    REQUIRE(resourceGroup.resources[0].actions.size() == 1);
    REQUIRE(resourceGroup.resources[0].actions[0].method == "GET");
    REQUIRE(resourceGroup.resources[0].actions[0].description == "");
    REQUIRE(resourceGroup.resources[0].actions[0].examples.size() == 1);
}

TEST_CASE("Parse resource groups with hr in description", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group 1\n"\
    "---\n"\
    "A\n";

    ResourceGroup resourceGroup;
    Report report;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, report, resourceGroup);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(resourceGroup.name == "1");
    REQUIRE(resourceGroup.description == "---\n\nA\n");
    REQUIRE(resourceGroup.resources.empty());
}

TEST_CASE("Make sure method followed by a group does not eat the group", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group One\n"\
    "## /1\n"\
    "### POST\n"\
    "# Group Two\n";

    ResourceGroup resourceGroup;
    Report report;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, report, resourceGroup);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1); // no response

    REQUIRE(resourceGroup.name == "One");
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 1);
    REQUIRE(resourceGroup.resources[0].uriTemplate == "/1");
    REQUIRE(resourceGroup.resources[0].actions.size() == 1);
    REQUIRE(resourceGroup.resources[0].actions[0].method == "POST");
    REQUIRE(resourceGroup.resources[0].actions[0].description.empty());
}

TEST_CASE("Parse resource method abbreviation followed by a foreign method", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# GET /resource\n"\
    "# POST\n";

    ResourceGroup resourceGroup;
    Report report;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, report, resourceGroup);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 2); // no response && unexpected action POST
    REQUIRE(report.warnings[0].code == EmptyDefinitionWarning);
    REQUIRE(report.warnings[1].code == IgnoringWarning);

    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 1);
    REQUIRE(resourceGroup.resources[0].name.empty());
    REQUIRE(resourceGroup.resources[0].uriTemplate == "/resource");
    REQUIRE(resourceGroup.resources[0].model.name.empty());
    REQUIRE(resourceGroup.resources[0].model.body.empty());
    REQUIRE(resourceGroup.resources[0].actions.size() == 1);
    REQUIRE(resourceGroup.resources[0].actions[0].method == "GET");
}

TEST_CASE("Parse resource method abbreviation followed by another", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# GET /resource\n"\
    "# POST /2\n";

    ResourceGroup resourceGroup;
    Report report;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, report, resourceGroup);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 2); // 2x no response
    REQUIRE(report.warnings[0].code == EmptyDefinitionWarning);
    REQUIRE(report.warnings[1].code == EmptyDefinitionWarning);

    REQUIRE(resourceGroup.name.empty());
    REQUIRE(resourceGroup.description.empty());
    REQUIRE(resourceGroup.resources.size() == 2);
    REQUIRE(resourceGroup.resources[0].name.empty());
    REQUIRE(resourceGroup.resources[0].uriTemplate == "/resource");
    REQUIRE(resourceGroup.resources[0].model.name.empty());
    REQUIRE(resourceGroup.resources[0].model.body.empty());
    REQUIRE(resourceGroup.resources[0].actions.size() == 1);
    REQUIRE(resourceGroup.resources[0].actions[0].method == "GET");
    REQUIRE(resourceGroup.resources[1].name.empty());
    REQUIRE(resourceGroup.resources[1].uriTemplate == "/2");
    REQUIRE(resourceGroup.resources[1].model.name.empty());
    REQUIRE(resourceGroup.resources[1].model.body.empty());
    REQUIRE(resourceGroup.resources[1].actions.size() == 1);
    REQUIRE(resourceGroup.resources[1].actions[0].method == "POST");
}

//TEST_CASE("Resource followed by a complete action", "[parser][regression][#185][now]")
//{
//    mdp::ByteBuffer source = \
//    "# Resource [/A]\n"\
//    "# POST /B\n"\
//    "+ Response 201\n";
//    
//    ResourceGroup resourceGroup;
//    Report report;
//    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source, ResourceGroupSectionType, report, resourceGroup);
//    
//    REQUIRE(report.error.code == Error::OK);
//    REQUIRE(report.warnings.empty());
//    
//    REQUIRE(resourceGroup.name.empty());
//    REQUIRE(resourceGroup.description.empty());
//    
//    REQUIRE(resourceGroup.resources.size() == 2);
//    REQUIRE(resourceGroup.resources[0].name == "Resource");
//    REQUIRE(resourceGroup.resources[0].uriTemplate == "/A");
//    REQUIRE(resourceGroup.resources[1].name.empty());
//    REQUIRE(resourceGroup.resources[1].uriTemplate == "/B");
//    REQUIRE(resourceGroup.resources[1].actions.size() == 1);
//    REQUIRE(resourceGroup.resources[1].actions[0].method == "POST");
//}
