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
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(ResourceGroupFixture,
                                                                   ResourceGroupSectionType,
                                                                   resourceGroup,
                                                                   ExportSourcemapOption);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.empty());

    REQUIRE(resourceGroup.node.attributes.name == "First");
    REQUIRE(resourceGroup.node.element == Element::CategoryElement);
    REQUIRE(resourceGroup.node.content.elements().size() == 2);

    REQUIRE(resourceGroup.node.content.elements().at(0).element == Element::CopyElement);
    REQUIRE(resourceGroup.node.content.elements().at(0).content.copy == "Fiber Optics\n\n");

    REQUIRE(resourceGroup.node.content.elements().at(1).element == Element::ResourceElement);
    REQUIRE(resourceGroup.node.content.elements().at(1).content.resource.uriTemplate == "/resource/{id}");
    REQUIRE(resourceGroup.node.content.elements().at(1).content.resource.name == "My Resource");

    SourceMapHelper::check(resourceGroup.sourceMap.attributes.name.sourceMap, 0, 15);
    SourceMapHelper::check(resourceGroup.sourceMap.content.elements().collection[0].content.copy.sourceMap, 15, 14);
    SourceMapHelper::check(resourceGroup.sourceMap.content.elements().collection[1].content.resource.name.sourceMap, 29, 32);
}

TEST_CASE("Parse resource group with empty resource", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group Name\n"\
    "p1\n"\
    "## /resource";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source,
                                                                   ResourceGroupSectionType,
                                                                   resourceGroup,
                                                                   ExportSourcemapOption);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.empty());

    REQUIRE(resourceGroup.node.attributes.name == "Name");
    REQUIRE(resourceGroup.node.element == Element::CategoryElement);
    REQUIRE(resourceGroup.node.content.elements().size() == 2);

    REQUIRE(resourceGroup.node.content.elements().at(0).element == Element::CopyElement);
    REQUIRE(resourceGroup.node.content.elements().at(0).content.copy == "p1\n");

    REQUIRE(resourceGroup.node.content.elements().at(1).element == Element::ResourceElement);
    REQUIRE(resourceGroup.node.content.elements().at(1).content.resource.uriTemplate == "/resource");

    SourceMapHelper::check(resourceGroup.sourceMap.attributes.name.sourceMap, 0, 13);
    SourceMapHelper::check(resourceGroup.sourceMap.content.elements().collection[0].content.copy.sourceMap, 13, 3);
    SourceMapHelper::check(resourceGroup.sourceMap.content.elements().collection[1].content.resource.uriTemplate.sourceMap, 16, 12);
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
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source,
                                                                   ResourceGroupSectionType,
                                                                   resourceGroup,
                                                                   ExportSourcemapOption);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.empty());

    REQUIRE(resourceGroup.node.attributes.name.empty());
    REQUIRE(resourceGroup.node.element == Element::CategoryElement);
    REQUIRE(resourceGroup.node.content.elements().size() == 2);

    REQUIRE(resourceGroup.node.content.elements().at(0).element == Element::ResourceElement);
    REQUIRE(resourceGroup.node.content.elements().at(0).content.resource.uriTemplate == "/r1");
    REQUIRE(resourceGroup.node.content.elements().at(0).content.resource.description == "p1\n");

    REQUIRE(resourceGroup.node.content.elements().at(1).element == Element::ResourceElement);
    REQUIRE(resourceGroup.node.content.elements().at(1).content.resource.uriTemplate == "/r2");
    REQUIRE(resourceGroup.node.content.elements().at(1).content.resource.description == "p2\n");

    REQUIRE(resourceGroup.sourceMap.attributes.name.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.content.elements().collection.size() == 2);
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
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source,
                                                                   ResourceGroupSectionType,
                                                                   resourceGroup,
                                                                   ExportSourcemapOption);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.size() == 4);

    REQUIRE(resourceGroup.node.attributes.name.empty());
    REQUIRE(resourceGroup.node.element == Element::CategoryElement);
    REQUIRE(resourceGroup.node.content.elements().size() == 2);

    REQUIRE(resourceGroup.node.content.elements().at(0).element == Element::ResourceElement);
    Resource resource1 = resourceGroup.node.content.elements().at(0).content.resource;

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

    Resource resource2 = resourceGroup.node.content.elements().at(1).content.resource;
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

    REQUIRE(resourceGroup.sourceMap.attributes.name.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.content.elements().collection.size() == 2);
}

TEST_CASE("Parse multiple resources with the same name", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group\n"\
    "## /r1\n"\
    "## /r1\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source,
                                                                   ResourceGroupSectionType,
                                                                   resourceGroup,
                                                                   ExportSourcemapOption);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.size() == 1);
    REQUIRE(resourceGroup.report.warnings[0].code == DuplicateWarning);

    REQUIRE(resourceGroup.node.attributes.name.empty());
    REQUIRE(resourceGroup.node.element == Element::CategoryElement);
    REQUIRE(resourceGroup.node.content.elements().size() == 2);

    REQUIRE(resourceGroup.sourceMap.attributes.name.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.content.elements().collection.size() == 2);
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
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source,
                                                                   ResourceGroupSectionType,
                                                                   resourceGroup,
                                                                   ExportSourcemapOption);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.size() == 3);   // preformatted asset & ignoring unrecognized node & no response

    REQUIRE(resourceGroup.node.attributes.name.empty());
    REQUIRE(resourceGroup.node.element == Element::CategoryElement);
    REQUIRE(resourceGroup.node.content.elements().size() == 1);
    REQUIRE(resourceGroup.node.content.elements().at(0).element == Element::ResourceElement);

    Resource resource = resourceGroup.node.content.elements().at(0).content.resource;
    REQUIRE(resource.uriTemplate == "/1");
    REQUIRE(resource.description.empty());
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions[0].method == "GET");
    REQUIRE(resource.actions[0].description == "");
    REQUIRE(resource.actions[0].examples.size() == 1);

    REQUIRE(resourceGroup.sourceMap.attributes.name.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.content.elements().collection.size() == 1);
}

TEST_CASE("Parse resource groups with hr in description", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group 1\n"\
    "---\n"\
    "A\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source,
                                                                   ResourceGroupSectionType,
                                                                   resourceGroup,
                                                                   ExportSourcemapOption);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.empty());

    REQUIRE(resourceGroup.node.attributes.name == "1");
    REQUIRE(resourceGroup.node.element == Element::CategoryElement);
    REQUIRE(resourceGroup.node.content.elements().size() == 1);
    REQUIRE(resourceGroup.node.content.elements().at(0).element == Element::CopyElement);
    REQUIRE(resourceGroup.node.content.elements().at(0).content.copy == "---\n\nA\n");

    SourceMapHelper::check(resourceGroup.sourceMap.attributes.name.sourceMap, 0, 10);
    SourceMapHelper::check(resourceGroup.sourceMap.content.elements().collection[0].content.copy.sourceMap, 10, 6);
}

TEST_CASE("Make sure method followed by a group does not eat the group", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# Group One\n"\
    "## /1\n"\
    "### POST\n"\
    "# Group Two\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source,
                                                                   ResourceGroupSectionType,
                                                                   resourceGroup,
                                                                   ExportSourcemapOption);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.size() == 1); // no response

    REQUIRE(resourceGroup.node.attributes.name == "One");
    REQUIRE(resourceGroup.node.element == Element::CategoryElement);
    REQUIRE(resourceGroup.node.content.elements().size() == 1);
    REQUIRE(resourceGroup.node.content.elements().at(0).element == Element::ResourceElement);

    Resource resource = resourceGroup.node.content.elements().at(0).content.resource;
    REQUIRE(resource.uriTemplate == "/1");
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions[0].method == "POST");
    REQUIRE(resource.actions[0].description.empty());

    SourceMapHelper::check(resourceGroup.sourceMap.attributes.name.sourceMap, 0, 12);
    REQUIRE(resourceGroup.sourceMap.content.elements().collection.size() == 1);
}

TEST_CASE("Parse resource method abbreviation followed by a foreign method", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# GET /resource\n"\
    "# POST\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source,
                                                                   ResourceGroupSectionType,
                                                                   resourceGroup,
                                                                   ExportSourcemapOption);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.size() == 2); // no response && unexpected action POST
    REQUIRE(resourceGroup.report.warnings[0].code == EmptyDefinitionWarning);
    REQUIRE(resourceGroup.report.warnings[1].code == IgnoringWarning);

    REQUIRE(resourceGroup.node.attributes.name.empty());
    REQUIRE(resourceGroup.node.element == Element::CategoryElement);
    REQUIRE(resourceGroup.node.content.elements().size() == 1);
    REQUIRE(resourceGroup.node.content.elements().at(0).element == Element::ResourceElement);

    Resource resource = resourceGroup.node.content.elements().at(0).content.resource;
    REQUIRE(resource.name.empty());
    REQUIRE(resource.uriTemplate == "/resource");
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions[0].method == "GET");

    REQUIRE(resourceGroup.sourceMap.attributes.name.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.content.elements().collection.size() == 1);
}

TEST_CASE("Parse resource method abbreviation followed by another", "[resource_group]")
{
    mdp::ByteBuffer source = \
    "# GET /resource\n"\
    "# POST /2\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source,
                                                                   ResourceGroupSectionType,
                                                                   resourceGroup,
                                                                   ExportSourcemapOption);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.size() == 2); // 2x no response
    REQUIRE(resourceGroup.report.warnings[0].code == EmptyDefinitionWarning);
    REQUIRE(resourceGroup.report.warnings[1].code == EmptyDefinitionWarning);

    REQUIRE(resourceGroup.node.attributes.name.empty());
    REQUIRE(resourceGroup.node.element == Element::CategoryElement);
    REQUIRE(resourceGroup.node.content.elements().size() == 2);
    REQUIRE(resourceGroup.node.content.elements().at(0).element == Element::ResourceElement);
    REQUIRE(resourceGroup.node.content.elements().at(1).element == Element::ResourceElement);

    Resource resource = resourceGroup.node.content.elements().at(0).content.resource;
    REQUIRE(resource.name.empty());
    REQUIRE(resource.uriTemplate == "/resource");
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions[0].method == "GET");

    resource = resourceGroup.node.content.elements().at(1).content.resource;
    REQUIRE(resource.name.empty());
    REQUIRE(resource.uriTemplate == "/2");
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions[0].method == "POST");

    REQUIRE(resourceGroup.sourceMap.attributes.name.sourceMap.empty());
    REQUIRE(resourceGroup.sourceMap.content.elements().collection.size() == 2);
}

TEST_CASE("Resource followed by a complete action", "[resource_group][regression][#185]")
{
    mdp::ByteBuffer source = \
    "# Resource [/A]\n"\
    "# POST /B\n"\
    "+ Response 201\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source,
                                                                   ResourceGroupSectionType,
                                                                   resourceGroup,
                                                                   ExportSourcemapOption);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.empty());

    REQUIRE(resourceGroup.node.attributes.name.empty());
    REQUIRE(resourceGroup.node.element == Element::CategoryElement);
    REQUIRE(resourceGroup.node.content.elements().size() == 2);
    REQUIRE(resourceGroup.node.content.elements().at(0).element == Element::ResourceElement);
    REQUIRE(resourceGroup.node.content.elements().at(1).element == Element::ResourceElement);

    Resource resource = resourceGroup.node.content.elements().at(0).content.resource;
    REQUIRE(resource.name == "Resource");
    REQUIRE(resource.uriTemplate == "/A");

    resource = resourceGroup.node.content.elements().at(1).content.resource;
    REQUIRE(resource.name.empty());
    REQUIRE(resource.uriTemplate == "/B");
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions[0].method == "POST");

    SourceMap<Resource> resourceSM = resourceGroup.sourceMap.content.elements().collection[0].content.resource;
    SourceMapHelper::check(resourceSM.name.sourceMap, 0, 16);
    SourceMapHelper::check(resourceSM.uriTemplate.sourceMap, 0, 16);

    resourceSM = resourceGroup.sourceMap.content.elements().collection[1].content.resource;
    REQUIRE(resourceSM.name.sourceMap.empty());
    SourceMapHelper::check(resourceSM.uriTemplate.sourceMap, 16, 10);
    REQUIRE(resourceSM.actions.collection.size() == 1);
    REQUIRE(resourceSM.actions.collection[0].method.sourceMap.size() == 1);
    REQUIRE(resourceSM.actions.collection[0].method.sourceMap[0].location == 16);
    REQUIRE(resourceSM.actions.collection[0].method.sourceMap[0].length == 10);
}

TEST_CASE("Too eager complete action processing", "[resource_group][regression][#187]")
{
    mdp::ByteBuffer source = \
    "# Group A\n"\
    "\n"\
    "```\n"\
    "GET /A\n"\
    "```\n"\
    "\n"\
    "Lorem Ipsum\n";

    ParseResult<ResourceGroup> resourceGroup;
    SectionParserHelper<ResourceGroup, ResourceGroupParser>::parse(source,
                                                                   ResourceGroupSectionType,
                                                                   resourceGroup,
                                                                   ExportSourcemapOption);

    REQUIRE(resourceGroup.report.error.code == Error::OK);
    REQUIRE(resourceGroup.report.warnings.empty());

    REQUIRE(resourceGroup.node.attributes.name == "A");
    REQUIRE(resourceGroup.node.element == Element::CategoryElement);
    REQUIRE(resourceGroup.node.content.elements().size() == 1);
    REQUIRE(resourceGroup.node.content.elements().at(0).element == Element::CopyElement);
    REQUIRE(resourceGroup.node.content.elements().at(0).content.copy == "```\nGET /A\n```\n\nLorem Ipsum\n");

    SourceMapHelper::check(resourceGroup.sourceMap.content.elements().collection[0].content.copy.sourceMap, 11, 28);
}
