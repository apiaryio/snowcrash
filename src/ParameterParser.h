//
//  ParameterParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 9/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_PARAMETERPARSER_H
#define SNOWCRASH_PARAMETERPARSER_H

#include "SectionParser.h"
#include "RegexMatch.h"
#include "StringUtility.h"

/** Parameter Value regex */
#define PARAMETER_VALUE "`([^`]+)`"

/** Parameter Identifier */
#define PARAMETER_IDENTIFIER "([[:alnum:]_.-]+)"

/** Lead in and out for comma separated values regex */
#define CSV_LEADINOUT "[[:blank:]]*,?[[:blank:]]*"

namespace snowcrash {

    /** Parameter Abbreviated definition matching regex */
    const char* const ParameterAbbrevDefinitionRegex = "^" PARAMETER_IDENTIFIER \
                                                        "([[:blank:]]*=[[:blank:]]*`([^`]*)`[[:blank:]]*)?([[:blank:]]*\\(([^)]*)\\)[[:blank:]]*)?([[:blank:]]*\\.\\.\\.[[:blank:]]*(.*))?$";

    /** Parameter Required matching regex */
    const char* const ParameterRequiredRegex = "^[[:blank:]]*[Rr]equired[[:blank:]]*$";

    /** Parameter Optional matching regex */
    const char* const ParameterOptionalRegex = "^[[:blank:]]*[Oo]ptional[[:blank:]]*$";

    /** Additonal Parameter Traits Example matching regex */
    const char* const AdditionalTraitsExampleRegex = CSV_LEADINOUT "`([^`]*)`" CSV_LEADINOUT;

    /** Additonal Parameter Traits Use matching regex */
    const char* const AdditionalTraitsUseRegex = CSV_LEADINOUT "([Oo]ptional|[Rr]equired)" CSV_LEADINOUT;

    /** Additonal Parameter Traits Type matching regex */
    const char* const AdditionalTraitsTypeRegex = CSV_LEADINOUT "([^,]*)" CSV_LEADINOUT;

    /** Parameter Values matching regex */
    const char* const ParameterValuesRegex = "^[[:blank:]]*[Vv]alues[[:blank:]]*$";

    /** Values expected content */
    const char* const ExpectedValuesContent = "nested list of possible parameter values, one element per list item e.g. '`value`'";

    /**
     * Parameter section processor
     */
    template<>
    struct SectionProcessor<Parameter> : public SectionProcessorBase<Parameter> {

        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     Report& report,
                                                     Parameter& out) {

            return ++MarkdownNodeIterator(node);
        }

        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       SectionParserData& pd,
                                                       Report& report,
                                                       Parameter& out) {

            return ++MarkdownNodeIterator(node);
        }

        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      SectionType sectionType) {

            return (node->type == mdp::ListItemMarkdownNodeType && node->children().size() > 1);
        }

        static SectionType sectionType(const MarkdownNodeIterator& node) {
            if (node->type == mdp::ListItemMarkdownNodeType
                && !node->children().empty()) {
                return ParameterDefinitionSectionType;
            }

            return UndefinedSectionType;
        }
    };

    /** Parameter Section Parser */
    typedef SectionParser<Parameter, ListSectionAdapter> ParameterParser;
}

#endif
