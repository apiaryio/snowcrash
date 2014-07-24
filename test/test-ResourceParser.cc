//
//  test-ResourceParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "ResourceParser.h"

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
    markdownParser.parse(ResourceFixture, markdownAST);

    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<Resource>::sectionType(markdownAST.children().begin()) == ResourceSectionType);

    // Nameless resource: "/resource"
    markdownAST.children().front().text = "/resource";
    REQUIRE(SectionProcessor<Resource>::sectionType(markdownAST.children().begin()) == ResourceSectionType);

    // Keyword "group"
    markdownAST.children().front().text = "Group A";
    REQUIRE(SectionProcessor<Resource>::sectionType(markdownAST.children().begin()) == UndefinedSectionType);

    // Resource Method
    markdownAST.children().front().text = "GET /resource";
    REQUIRE(SectionProcessor<Resource>::sectionType(markdownAST.children().begin()) == ResourceMethodSectionType);
}

TEST_CASE("Parse resource", "[resource]")
{
    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(ResourceFixture, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(resource.name == "My Resource");
    REQUIRE(resource.uriTemplate == "/resource/{id}{?limit}");
    REQUIRE(resource.description == "Awesome description\n\n");
    REQUIRE(resource.headers.empty());

    REQUIRE(resource.model.name == "Resource");
    REQUIRE(resource.model.body == "X.O.\n");

    REQUIRE(resource.parameters.size() == 2);
    REQUIRE(resource.parameters[0].name == "id");
    REQUIRE(resource.parameters[0].description == "Lorem ipsum\n");
    REQUIRE(resource.parameters[1].name == "limit");

    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions.front().method == "GET");
}

TEST_CASE("Parse partially defined resource", "[resource]")
{
    mdp::ByteBuffer source = \
    "# /1\n"\
    "## GET\n"\
    "+ Request\n"\
    "p1\n";

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 2); // no response & preformatted asset
    REQUIRE(report.warnings[0].code == IndentationWarning);
    REQUIRE(report.warnings[1].code == EmptyDefinitionWarning);

    REQUIRE(resource.name.empty());
    REQUIRE(resource.uriTemplate == "/1");
    REQUIRE(resource.description.empty());
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions.front().method == "GET");
    REQUIRE(resource.actions.front().description.empty());
    REQUIRE(!resource.actions.front().examples.empty());
    REQUIRE(resource.actions.front().examples.front().requests.size() == 1);
    REQUIRE(resource.actions.front().examples.front().requests.front().name.empty());
    REQUIRE(resource.actions.front().examples.front().requests.front().description.empty());
    REQUIRE(resource.actions.front().examples.front().requests.front().body == "p1\n\n");
}

TEST_CASE("Parse multiple method descriptions", "[resource]")
{
    mdp::ByteBuffer source = \
    "# /1\n"\
    "# GET\n"\
    "p1\n"\
    "# POST\n"\
    "p2\n";

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 2); // 2x no response

    REQUIRE(resource.uriTemplate == "/1");
    REQUIRE(resource.description.empty());
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.size() == 2);
    REQUIRE(resource.actions[0].method == "GET");
    REQUIRE(resource.actions[0].description == "p1\n");
    REQUIRE(resource.actions[1].method == "POST");
    REQUIRE(resource.actions[1].description == "p2\n");
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

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 2); // empty reuqest asset & no response

    REQUIRE(resource.uriTemplate == "/1");
    REQUIRE(resource.description == "A\n");
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.size() == 3);

    REQUIRE(resource.actions[0].method == "GET");
    REQUIRE(resource.actions[0].description == "B\n");
    REQUIRE(resource.actions[0].examples.size() == 1);
    REQUIRE(resource.actions[0].examples[0].requests.empty());
    REQUIRE(resource.actions[0].examples[0].responses.size() == 1);
    REQUIRE(resource.actions[0].examples[0].responses[0].name == "200");
    REQUIRE(resource.actions[0].examples[0].responses[0].description.empty());
    REQUIRE(resource.actions[0].examples[0].responses[0].body == "Code 1\n");

    REQUIRE(resource.actions[1].method == "POST");
    REQUIRE(resource.actions[1].description == "C\n");
    REQUIRE(resource.actions[1].examples.size() == 1);
    REQUIRE(resource.actions[1].examples[0].requests.size() == 1);
    REQUIRE(resource.actions[1].examples[0].requests[0].name == "D");
    REQUIRE(resource.actions[1].examples[0].requests[0].description.empty());
    REQUIRE(resource.actions[1].examples[0].requests[0].description.empty());
    REQUIRE(resource.actions[1].examples[0].responses.size() == 1);
    REQUIRE(resource.actions[1].examples[0].responses[0].name == "200");
    REQUIRE(resource.actions[1].examples[0].responses[0].description.empty());
    REQUIRE(resource.actions[1].examples[0].responses[0].body == "{}\n");

    REQUIRE(resource.actions[2].method == "PUT");
    REQUIRE(resource.actions[2].description == "E\n");
    REQUIRE(resource.actions[2].examples.empty());
}

TEST_CASE("Parse description with list", "[resource]")
{
    mdp::ByteBuffer source = \
    "# /1\n"\
    "+ A\n"\
    "+ B\n\n"\
    "p1\n";

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(resource.uriTemplate == "/1");
    REQUIRE(resource.description == "+ A\n\n+ B\n\np1\n");
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.empty());
}

TEST_CASE("Parse resource with a HR", "[resource][block]")
{
    mdp::ByteBuffer source = \
    "# /1\n"\
    "A\n"\
    "---\n"\
    "B\n";

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(resource.uriTemplate == "/1");
    REQUIRE(resource.description == "A\n---\n\nB\n");
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.empty());
}

TEST_CASE("Parse resource method abbreviation", "[resource]")
{
    mdp::ByteBuffer source = \
    "# GET /resource\n"\
    "Description\n"\
    "+ Response 200\n"\
    "    + Body\n\n"\
    "            {}\n";

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(resource.name.empty());
    REQUIRE(resource.uriTemplate == "/resource");
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions[0].method == "GET");
    REQUIRE(resource.actions[0].description == "Description\n");

    REQUIRE(resource.actions[0].examples[0].responses.size() == 1);
    REQUIRE(resource.actions[0].examples[0].responses[0].description.empty());
    REQUIRE(resource.actions[0].examples[0].responses[0].body == "{}\n");
}

TEST_CASE("Parse resource without name", "[resource]")
{
    mdp::ByteBuffer source = "# /resource\n";

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(resource.uriTemplate == "/resource");
    REQUIRE(resource.name.empty());
    REQUIRE(resource.model.name.empty());
    REQUIRE(resource.model.body.empty());
    REQUIRE(resource.actions.size() == 0);
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

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 2);
    REQUIRE(report.warnings[0].code == LogicalErrorWarning);
    REQUIRE(report.warnings[1].code == LogicalErrorWarning);

    REQUIRE(resource.parameters.size() == 1);
    REQUIRE(resource.parameters[0].name == "olive");
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions[0].examples.size() == 1);
    REQUIRE(resource.actions[0].examples[0].responses.size() == 1);
}

TEST_CASE("Parse nameless resource with named model", "[resource][model][source]")
{
    mdp::ByteBuffer source = \
    "# /message\n"\
    "+ Super Model\n"\
    "\n"\
    "        AAA\n"\
    "\n";

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(resource.model.name == "Super");
    REQUIRE(resource.model.body == "AAA\n");
    REQUIRE(resource.actions.empty());
}

TEST_CASE("Parse nameless resource with nameless model", "[resource][model][source]")
{
    mdp::ByteBuffer source = \
    "# /message\n"\
    "+ Model\n"\
    "\n"\
    "        AAA\n"\
    "\n";

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == SymbolError);
    REQUIRE(report.warnings.empty());

    REQUIRE(resource.model.name.empty());
}

TEST_CASE("Parse named resource with nameless model", "[resource][model][source]")
{
    mdp::ByteBuffer source = \
    "# Message [/message]\n"\
    "+ Model\n"\
    "  \n"\
    "        AAA\n"\
    "\n"\
    "## Retrieve a message [GET]\n"\
    "+ Response 200\n"\
    "    \n"\
    "        [Message][]\n\n";

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(resource.model.name == "Message");
    REQUIRE(resource.model.body == "AAA\n");
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions[0].name == "Retrieve a message");
    REQUIRE(resource.actions[0].method == "GET");
    REQUIRE(resource.actions[0].examples.size() == 1);
    REQUIRE(resource.actions[0].examples[0].responses.size() == 1);
    REQUIRE(resource.actions[0].examples[0].responses[0].name == "200");
    REQUIRE(resource.actions[0].examples[0].responses[0].body == "AAA\n");
}

TEST_CASE("Parse root resource", "[resource]")
{
    mdp::ByteBuffer source = "# API Root [/]\n";

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(resource.name == "API Root");
    REQUIRE(resource.uriTemplate == "/");
    REQUIRE(resource.actions.empty());
}

TEST_CASE("Parse resource with invalid URI Tempalte", "[resource]")
{
    mdp::ByteBuffer source = "# Resource [/id{? limit}]\n";
    
    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 1);
    REQUIRE(report.warnings[0].code == URIWarning);
    
    REQUIRE(resource.name == "Resource");
    REQUIRE(resource.uriTemplate == "/id{? limit}");
    REQUIRE(resource.actions.empty());
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

    Resource resource;
    Report report;
    SectionParserHelper<Resource, ResourceParser>::parse(source, ResourceSectionType, report, resource);

    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.size() == 2);
    REQUIRE(report.warnings[0].code == DeprecatedWarning);
    REQUIRE(report.warnings[1].code == DeprecatedWarning);

    REQUIRE(resource.headers.empty());
    REQUIRE(resource.actions.size() == 1);
    REQUIRE(resource.actions[0].headers.empty());
    REQUIRE(resource.actions[0].examples.size() == 1);
    REQUIRE(resource.actions[0].examples[0].responses.size() == 1);
    REQUIRE(resource.actions[0].examples[0].responses[0].headers.size() == 3);
    REQUIRE(resource.actions[0].examples[0].responses[0].headers[0].first == "header1");
    REQUIRE(resource.actions[0].examples[0].responses[0].headers[0].second == "value1");
    REQUIRE(resource.actions[0].examples[0].responses[0].headers[1].first == "header2");
    REQUIRE(resource.actions[0].examples[0].responses[0].headers[1].second == "value2");
    REQUIRE(resource.actions[0].examples[0].responses[0].headers[2].first == "header3");
    REQUIRE(resource.actions[0].examples[0].responses[0].headers[2].second == "value3");
}
