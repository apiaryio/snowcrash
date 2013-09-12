//
//  ParameterDefinitonParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 9/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_PARAMETERDEFINITIONPARSER_H
#define SNOWCRASH_PARAMETERDEFINITIONPARSER_H

#include <sstream>
#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "ListUtility.h"
#include "RegexMatch.h"
#include "StringUtility.h"


/** Parameter Value regex */
#define PARAMETER_VALUE "`([^`]+)`"

/** Parameter Definition matching regex */
static const std::string ParameterDefinitionRegex("^[ \\t]*(" SYMBOL_IDENTIFIER ")[ \\t]*$");

/** Parameter Abbreviated definition matching regex */
static const std::string ParameterAbbrevDefinitionRegex("^([[:alnum:][:blank:]_\\-]+)([[:blank:]]*=[[:blank:]]*`([^`]*)`[[:blank:]]*)?([[:blank:]]*\\((.*)\\)[[:blank:]]*)?([[:blank:]]*\\.\\.\\.[[:blank:]]*(.*))?$");

/** Parameter Required matching regex */
static const std::string ParameterRequiredRegex("^[ \\t]*[Rr]equired[ \\t]*$");

/** Parameter Optional matching regex */
static const std::string ParameterOptionalRegex("^[ \\t]*[Oo]ptional[ \\t]*$");

/** Lead in and out for comma separated values regex */
# define CSV_LEADINOUT "[[:blank:]]*,?[[:blank:]]*"

/** Additonal Parameter Traits Example matching regex */
static const std::string AdditionalTraitsExampleRegex(CSV_LEADINOUT "`([^`]*)`" CSV_LEADINOUT);

/** Additonal Parameter Traits Use matching regex */
static const std::string AdditionalTraitsUseRegex(CSV_LEADINOUT "([Oo]ptional|[Rr]equired)" CSV_LEADINOUT);

/** Additonal Parameter Traits Type matching regex */
static const std::string AdditionalTraitsTypeRegex(CSV_LEADINOUT "([^,]*)" CSV_LEADINOUT);

/** Parameter Values matching regex */
static const std::string ParameterValuesRegex("^[ \\t]*[Vv]alues[ \\t]*$");

/** List of expected nested attributes */
static const std::string ExpectedTraitItems = "'Values' followed by a nested list of possbile values";

/** Expected parameter definiton */
static const std::string ExpectedParameterDefinition = "'<parameter identifier>' or a full parameter specification"\
                                                       " e.g. 'id (required, number, `42`) ... Id of the resource'";

/** Values expected content */
static const std::string ExpectedValuesContent = "nested list of possible parameter values, one element per list item e.g. '`value`'";

namespace snowcrash {
    
    /**
     *  Classifier of internal list items, ParameterCollection context.
     */
    template <>
    FORCEINLINE Section ClassifyInternaListBlock<Parameter>(const BlockIterator& begin,
                                                            const BlockIterator& end) {


        if (begin->type != ListBlockBeginType &&
            begin->type != ListItemBlockBeginType)
            return UndefinedSection;
        
        SourceData remainingContent;
        SourceData content = GetListItemSignature(begin, end, remainingContent);
        
        content = TrimString(content);
        
        if (RegexMatch(content, ParameterValuesRegex))
            return ParameterValuesSection;
        
        return UndefinedSection;
    }
    
    /**
     *  Returns true if given block has a parameter definition signature, false otherwise.
     */
    FORCEINLINE bool HasParameterDefinitionSignature(const BlockIterator& begin,
                                                     const BlockIterator& end) {
        
        if (begin->type != ListBlockBeginType &&
            begin->type != ListItemBlockBeginType)
            return false;
        
        // Since we are too generic make sure the signature is not inner list
        Section listSection = ClassifyInternaListBlock<Parameter>(begin, end);
        if (listSection != UndefinedSection)
            return false;
        
        // Or any other reserved keyword
        if (HasParametersSignature(begin, end))
            return false;
        
        SourceData remainingContent;
        SourceData content = GetListItemSignature(begin, end, remainingContent);
        content = TrimString(content);
        return RegexMatch(content, ParameterAbbrevDefinitionRegex);
    }
    
    /**
     *  Block Classifier, Parameter context.
     */
    template <>
    FORCEINLINE Section ClassifyBlock<Parameter>(const BlockIterator& begin,
                                                 const BlockIterator& end,
                                                 const Section& context) {
        
        if (context == UndefinedSection) {
            if (HasParameterDefinitionSignature(begin, end))
                return ParameterDefinitionSection;
        }
        else if (context == ParameterDefinitionSection) {

            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSection;
            
            Section listSection = ClassifyInternaListBlock<Parameter>(begin, end);
            if (listSection != UndefinedSection)
                return listSection;
            
            if (begin->type == ListBlockBeginType)
                return ForeignSection; // Foreign nested list-item
            
            if (begin->type == ListItemBlockBeginType)
                return UndefinedSection;
        }
        else if (context == ParameterValuesSection ||
                 context == ForeignSection) {

            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSection;
            
            Section listSection = ClassifyInternaListBlock<Parameter>(begin, end);
            if (listSection != UndefinedSection)
                return listSection;
            
            return ForeignSection;
        }

        return (context == ParameterDefinitionSection) ? context : UndefinedSection;
    }
    
    
    /**
     *  Parameter section parser.
     */
    template<>
    struct SectionParser<Parameter> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               BlueprintParserCore& parser,
                                               Parameter& parameter) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            switch (section) {
                    
                case ParameterDefinitionSection:
                    result = HandleParmeterDefinitionSection(cur, bounds, parser, parameter);
                    break;
                    
                case ParameterValuesSection:
                    result = HandleValuesSection(cur, bounds, parser, parameter);
                    break;
                    
                case ForeignSection:
                    result = HandleForeignSection(cur, bounds, ExpectedTraitItems);
                    break;
                    
                case UndefinedSection:
                    result.second = CloseListItemBlock(cur, bounds.second);
                    break;
                    
                default:
                    result.first.error = UnexpectedBlockError(*cur);
                    break;
            }
            
            return result;
        }
        
        /** Parse a parameter definition top-level section blocks. */
        static ParseSectionResult HandleParmeterDefinitionSection(const BlockIterator& cur,
                                                                  const SectionBounds& bounds,
                                                                  BlueprintParserCore& parser,
                                                                  Parameter& parameter) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur = cur;

            // Signature
            if (sectionCur == bounds.first) {
                ProcessSignature(sectionCur, bounds.second, parser.sourceData, result.first, parameter);
                result.second = SkipSignatureBlock(sectionCur, bounds.second);
                return result;
            }

            // Description
            if (sectionCur->type == QuoteBlockBeginType) {
                sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
            }
            else if (sectionCur->type == ListBlockBeginType) {
                
                SourceDataBlock descriptionMap;
                sectionCur = SkipToDescriptionListEnd<Parameter>(sectionCur, bounds.second, descriptionMap);
                
                if (sectionCur->type != ListBlockEndType) {
                    if (!descriptionMap.empty())
                        parameter.description += MapSourceData(parser.sourceData, descriptionMap);
                    
                    result.second = sectionCur;
                    return result;
                }
            }
            
            if (!CheckCursor(sectionCur, bounds, cur, result.first))
                return result;

            parameter.description += MapSourceData(parser.sourceData, sectionCur->sourceMap);
            
            if (sectionCur != bounds.second)
                result.second = ++sectionCur;
            
            return result;
        }
        
        /**
         *  Retrieve and process parameter definition signature.
         */
        static void ProcessSignature(const BlockIterator& begin,
                                     const BlockIterator& end,
                                     const SourceData& sourceData,
                                     Result& result,
                                     Parameter& parameter) {
            
            
            // Set default values
            parameter.use = UndefinedParameterUse;
            
            // Process signature
            SourceData remainingContent;
            SourceData signature = GetListItemSignature(begin, end, remainingContent);

            TrimString(signature);
            CaptureGroups captureGroups;
            if (RegexCapture(signature, ParameterAbbrevDefinitionRegex, captureGroups) &&
                captureGroups.size() == 8) {
                
                // Name
                parameter.name = captureGroups[1];
                TrimString(parameter.name);
                
                // Default value
                if (!captureGroups[3].empty())
                    parameter.defaultValue = captureGroups[3];
                
                // Additional Attributes
                if (!captureGroups[5].empty())
                    ProcessSignatureAdditionalTraits(begin, end, captureGroups[5], result, parameter);
                
                // Description
                if (!captureGroups[7].empty())
                    parameter.description = captureGroups[7];
                
                if (!remainingContent.empty()) {
                    parameter.description += "\n";
                    parameter.description += remainingContent;
                    parameter.description += "\n";
                }
                
                
                // Check possible required vs default clash
                if (parameter.use != OptionalParameterUse &&
                    !parameter.defaultValue.empty()) {
                    
                    // WARN: Required vs default clash
                    BlockIterator nameBlock = ListItemNameBlock(begin, end);
                    std::stringstream ss;
                    ss << "specifying parameter '" << parameter.name << "' as required supersedes its default value"\
                          ", declare the parameter as 'optional' to specify its default value";
                    result.warnings.push_back(Warning(ss.str(),
                                                      LogicalErrorWarning,
                                                      nameBlock->sourceMap));
                }

            }
            else {
                // ERR: unable to parse 
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                std::stringstream ss;
                result.error = (Error("unable to parse parameter specification",
                                      BusinessError,
                                      nameBlock->sourceMap));
            }
        }
        
        /** Parse additional parameter attributes from abbrev definition bracket */
        static void ProcessSignatureAdditionalTraits(const BlockIterator& begin,
                                                     const BlockIterator& end,
                                                     const SourceData& additionalTraits,
                                                     Result& result,
                                                     Parameter& parameter)
        {
            
            // Cherry pick example value, if any
            std::string source = additionalTraits;
            TrimString(source);
            CaptureGroups captureGroups;
            if (RegexCapture(source, AdditionalTraitsExampleRegex, captureGroups) &&
                captureGroups.size() > 1) {
                
                parameter.exampleValue = captureGroups[1];
                std::string::size_type pos = source.find(captureGroups[0]);
                if (pos != std::string::npos)
                    source.replace(pos, captureGroups[0].length(), std::string());
            }
            
            // Cherry pick use attribute, if any
            captureGroups.clear();
            if (RegexCapture(source, AdditionalTraitsUseRegex, captureGroups) &&
                captureGroups.size() > 1) {
                
                parameter.use = (RegexMatch(captureGroups[1], ParameterOptionalRegex)) ? OptionalParameterUse : RequiredParameterUse;

                std::string::size_type pos = source.find(captureGroups[0]);
                if (pos != std::string::npos)
                    source.replace(pos, captureGroups[0].length(), std::string());
            }
            
            // Finish with type
            captureGroups.clear();
            if (RegexCapture(source, AdditionalTraitsTypeRegex, captureGroups) &&
                captureGroups.size() > 1) {
                
                parameter.type = captureGroups[1];
                
                std::string::size_type pos = source.find(captureGroups[0]);
                if (pos != std::string::npos)
                    source.replace(pos, captureGroups[0].length(), std::string());
            }
            
            // Check whats left
            TrimString(source);
            if (!source.empty()) {
                // WARN: Additional parameters traits warning
                BlockIterator nameBlock = ListItemNameBlock(begin, end);
                std::stringstream ss;
                ss << "unable to parse additional parameter traits";
                ss << ", expected '([required | optional], [<type>], [`<example value>`])'";
                ss << ", e.g. '(optional, string, `Hello World`)'";

                result.warnings.push_back(Warning(ss.str(),
                                                  FormattingWarning,
                                                  nameBlock->sourceMap));
                
                parameter.type.clear();
                parameter.exampleValue.clear();
                parameter.use = UndefinedParameterUse;
            }
        }
        
        /** Parse possible values enumeration section blocks. */
        static ParseSectionResult HandleValuesSection(const BlockIterator& cur,
                                                      const SectionBounds& bounds,
                                                      BlueprintParserCore& parser,
                                                      Parameter& parameter) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            
            // Check redefinition
            if (!parameter.values.empty()) {
                // WARN: parameter values are already defined
                BlockIterator nameBlock = ListItemNameBlock(cur, bounds.second);
                std::stringstream ss;
                ss << "overshadowing previous 'values' definition";
                ss << " for parameter '" << parameter.name << "'";
                result.first.warnings.push_back(Warning(ss.str(),
                                                        RedefinitionWarning,
                                                        nameBlock->sourceMap));
            }
            
            // Clear any previous content
            parameter.values.clear();
            
            // Check additional content in signature
            CheckSignatureAdditionalContent(cur, bounds, "'values:' keyword", ExpectedValuesContent, result.first);

            // Parse inner list of entities
            BlockIterator sectionCur = SkipSignatureBlock(cur, bounds.second);
            BlockIterator endCur = cur;
            if (endCur->type == ListBlockBeginType)
                ++endCur;
            endCur = SkipToSectionEnd(endCur, bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
            
            if (sectionCur != endCur) {

                // Iterate over list blocks, try to parse any nested lists of possible elements
                for (; sectionCur != endCur; ++sectionCur) {
                    
                    if (sectionCur->type == QuoteBlockBeginType)
                        sectionCur = SkipToSectionEnd(sectionCur, endCur, QuoteBlockBeginType, QuoteBlockEndType);
                    
                    bool entitiesParsed = false;
                    if (sectionCur->type == ListBlockBeginType) {
                        if (parameter.values.empty()) {
                            
                            // Try to parse some values
                            ParseSectionResult valuesResult = ParseValuesEntities(sectionCur,
                                                                                  bounds,
                                                                                  parser,
                                                                                  parameter.values);
                            result.first += valuesResult.first;
                            sectionCur = valuesResult.second;
                            if (result.first.error.code != Error::OK)
                                return result;

                            entitiesParsed = true;
                        }
                        else {
                            sectionCur = SkipToSectionEnd(sectionCur, endCur, ListBlockBeginType, ListBlockEndType);
                        }
                    }
                    
                    if (!entitiesParsed) {
                        // WARN: ignoring extraneous content
                        std::stringstream ss;
                        ss << "ignoring additional content in the 'values' attribute of the '";
                        ss << parameter.name << "' parameter";
                        ss << ", " << ExpectedValuesContent;
                        result.first.warnings.push_back(Warning(ss.str(), IgnoringWarning, sectionCur->sourceMap));
                    }
                }
            }
            
            if (parameter.values.empty()) {
                // WARN: empty definition
                std::stringstream ss;
                ss << "no possible values specified for parameter '" << parameter.name << "'";
                result.first.warnings.push_back(Warning(ss.str(), EmptyDefinitionWarning, sectionCur->sourceMap));
            }
            
            endCur = CloseListItemBlock(sectionCur, bounds.second);
            result.second = endCur;
            return result;
        }
        
        /** Parse entities in values attribute */
        static ParseSectionResult ParseValuesEntities(const BlockIterator& cur,
                                                      const SectionBounds& bounds,
                                                      BlueprintParserCore& parser,
                                                      Collection<Value>::type& values) {
         
            ParseSectionResult result = std::make_pair(Result(), cur);
            
            if (cur->type != ListBlockBeginType)
                return result;
            
            BlockIterator sectionCur = ContentBlock(cur, bounds.second);
            
            while (sectionCur != bounds.second &&
                   sectionCur->type == ListItemBlockBeginType) {
                
                sectionCur = SkipToSectionEnd(sectionCur, bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
                
                CaptureGroups captureGroups;
                std::string content = sectionCur->content;
                if (content.empty()) {
                    // Not inline list, map from source
                    content = MapSourceData(parser.sourceData, sectionCur->sourceMap);
                }

                RegexCapture(content, PARAMETER_VALUE, captureGroups);
                if (captureGroups.size() > 1) {
                    values.push_back(captureGroups[1]);
                }
                else {
                    // WARN: Ignoring unexpected content
                    TrimString(content);
                    std::stringstream ss;
                    ss << "ignoring the '" << content << "' element";
                    ss << ", expected '`" << content << "`'";
                    result.first.warnings.push_back(Warning(ss.str(), IgnoringWarning, sectionCur->sourceMap));
                }
                
                ++sectionCur;
            }
            
            result.second = sectionCur;
            return result;
        }

    };
    
    typedef BlockParser<Parameter, SectionParser<Parameter> > ParameterDefinitionParser;
}

#endif
