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
#include "RegexMatch.h"
#include "StringUtility.h"
#include "ListBlockUtility.h"
#include "SectionUtility.h"
#include "DescriptionSectionUtility.h"

/** Parameter Value regex */
#define PARAMETER_VALUE "`([^`]+)`"

/** Parameter Identifier */
#define PARAMETER_IDENTIFIER "(([[:alnum:]_.-])*|(%[A-Fa-f0-9]{2})*)+"

/** Lead in and out for comma separated values regex */
#define CSV_LEADINOUT "[[:blank:]]*,?[[:blank:]]*"

namespace snowcrashconst {

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

    const std::string DescriptionIdentifier = "...";
}

namespace snowcrash {
    
    /**
     *  Classifier of internal list items, ParameterCollection context.
     */
    template <>
    FORCEINLINE SectionType ClassifyInternaListBlock<Parameter>(const BlockIterator& begin,
                                                            const BlockIterator& end) {


        if (begin->type != ListBlockBeginType &&
            begin->type != ListItemBlockBeginType)
            return UndefinedSectionType;
        
        SourceData remainingContent;
        SourceData content = GetListItemSignature(begin, end, remainingContent);
        
        content = TrimString(content);
        
        if (RegexMatch(content, snowcrashconst::ParameterValuesRegex))
            return ParameterValuesSectionType;
        
        return UndefinedSectionType;
    }
    
    /** Children blocks classifier */
    template <>
    FORCEINLINE SectionType ClassifyChildrenListBlock<Parameter>(const BlockIterator& begin,
                                                                 const BlockIterator& end) {
        
        SectionType type = ClassifyInternaListBlock<Parameter>(begin, end);
        if (type != UndefinedSectionType)
            return type;
                
        return UndefinedSectionType;
    }
   
    /** Determine if a signature is a valid parameter*/
    FORCEINLINE bool IsValidParameterSignature(const SourceData& signature) {

        SourceData innerSignature = signature;
        innerSignature = TrimString(innerSignature);

        if (innerSignature.length() == 0) {
            return false; // Empty string, invalid
        }

        size_t firstSpace = innerSignature.find(" ");
        if (firstSpace == std::string::npos) {
            return RegexMatch(innerSignature, "^" PARAMETER_IDENTIFIER "$");
        }
        
        std::string paramName = innerSignature.substr(0, firstSpace);
        if (!RegexMatch(paramName, "^" PARAMETER_IDENTIFIER "$")) {
            return false; // Invalid param name
        }
        // Remove param name
        innerSignature = innerSignature.substr(firstSpace + 1);

        size_t descriptionPos = innerSignature.find(snowcrashconst::DescriptionIdentifier);
        // Remove description
        if (descriptionPos != std::string::npos) {
            innerSignature = innerSignature.substr(0, descriptionPos);
            innerSignature = TrimString(innerSignature);
        }

        size_t attributesPos = innerSignature.find("(");
        if (attributesPos != std::string::npos) {
            size_t endOfAttributesPos = innerSignature.find_last_of(")");
            if (endOfAttributesPos == std::string::npos) {
                return false; // Expecting close of attributes
            }
            // Remove attributes
            innerSignature = innerSignature.substr(0, attributesPos);
            innerSignature = TrimString(innerSignature);
        }

        if (innerSignature.length() == 0) {
            return true;
        }

        if (innerSignature.substr(0,1) == "=") {
            innerSignature = innerSignature.substr(1);
            innerSignature = TrimString(innerSignature);
            if (innerSignature.length() == 0) {
                return false; // No default value
            }
            if (innerSignature.substr(0,1) == "`" && innerSignature.substr(innerSignature.length()-1,1) == "`") {
                return true;
            }
        }

        return false;
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
        SectionType listSection = ClassifyInternaListBlock<Parameter>(begin, end);
        if (listSection != UndefinedSectionType)
            return false;
        
        // Or any other reserved keyword
        if (HasParametersSignature(begin, end))
            return false;
        
        SourceData remainingContent;
        SourceData content = GetListItemSignature(begin, end, remainingContent);
        content = TrimString(content);
        return IsValidParameterSignature(content);
    }

    /**
     *  Block Classifier, Parameter context.
     */
    template <>
    FORCEINLINE SectionType ClassifyBlock<Parameter>(const BlockIterator& begin,
                                                 const BlockIterator& end,
                                                 const SectionType& context) {
        
        if (context == UndefinedSectionType) {
            if (HasParameterDefinitionSignature(begin, end))
                return ParameterDefinitionSectionType;
        }
        else if (context == ParameterDefinitionSectionType) {

            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSectionType;
            
            SectionType listSection = ClassifyInternaListBlock<Parameter>(begin, end);
            if (listSection != UndefinedSectionType)
                return listSection;
            
            if (begin->type == ListBlockBeginType)
                return ForeignSectionType; // Foreign nested list-item
            
            if (begin->type == ListItemBlockBeginType)
                return UndefinedSectionType;
        }
        else if (context == ParameterValuesSectionType ||
                 context == ForeignSectionType) {

            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSectionType;
            
            SectionType listSection = ClassifyInternaListBlock<Parameter>(begin, end);
            if (listSection != UndefinedSectionType)
                return listSection;
            
            return ForeignSectionType;
        }

        return (context == ParameterDefinitionSectionType) ? context : UndefinedSectionType;
    }
    
    /**
     *  Parameter section parser.
     */
    template<>
    struct SectionParser<Parameter> {
        
        static ParseSectionResult ParseSection(const BlueprintSection& section,
                                               const BlockIterator& cur,
                                               BlueprintParserCore& parser,
                                               Parameter& parameter) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            switch (section.type) {
                    
                case ParameterDefinitionSectionType:
                    result = HandleParmeterDefinitionSection(section, cur, parser, parameter);
                    break;
                    
                case ParameterValuesSectionType:
                    result = HandleValuesSection(section, cur, parser, parameter);
                    break;
                    
                case ForeignSectionType:
                    result = HandleForeignSection<Parameter>(section, cur, parser.sourceData);
                    break;
                    
                case UndefinedSectionType:
                    result.second = CloseList(cur, section.bounds.second);
                    break;
                    
                default:
                    result.first.error = UnexpectedBlockError(section, cur, parser.sourceData);
                    break;
            }
            
            return result;
        }
        
        
        static void Finalize(const SectionBounds& bounds,
                             BlueprintParserCore& parser,
                             Parameter& parameter,
                             Result& result) {}
        
        /** Parse a parameter definition top-level section blocks. */
        static ParseSectionResult HandleParmeterDefinitionSection(const BlueprintSection& section,
                                                                  const BlockIterator& cur,
                                                                  BlueprintParserCore& parser,
                                                                  Parameter& parameter) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur = cur;

            // Signature
            if (sectionCur == section.bounds.first) {
                ProcessSignature(section, sectionCur, parser.sourceData, result.first, parameter);
                result.second = SkipSignatureBlock(sectionCur, section.bounds.second);
                return result;
            }

            // Description
            result = ParseDescriptionBlock<Parameter>(section,
                                                       sectionCur,
                                                       parser.sourceData,
                                                       parameter);
            return result;
            
        }

      
        
        /**
         *  Retrieve and process parameter definition signature.
         */
        static void ProcessSignature(const BlueprintSection& section,
                                     const BlockIterator& cur,
                                     const SourceData& sourceData,
                                     Result& result,
                                     Parameter& parameter) {
            
            // Set default values
            parameter.use = UndefinedParameterUse;
            
            // Process signature
            SourceData remainingContent;
            SourceData signature = GetListItemSignature(cur, section.bounds.second, remainingContent);
            
            TrimString(signature);

            if (IsValidParameterSignature(signature)) {
                
                SourceData innerSignature = signature;
                innerSignature = TrimString(innerSignature);

                size_t firstSpace = innerSignature.find(" ");
                if (firstSpace == std::string::npos) {
                    // Name
                    parameter.name = signature;
                }
                else {
                    parameter.name = innerSignature.substr(0, firstSpace);
                    innerSignature = innerSignature.substr(firstSpace + 1);
                    size_t descriptionPos = innerSignature.find(snowcrashconst::DescriptionIdentifier);
                    if (descriptionPos != std::string::npos) {
                        // Description
                        parameter.description = innerSignature.substr(descriptionPos);
                        parameter.description = TrimString(parameter.description.replace(0, snowcrashconst::DescriptionIdentifier.length(), ""));
                        innerSignature = innerSignature.substr(0, descriptionPos);
                        innerSignature = TrimString(innerSignature);
                    }

                    size_t attributesPos = innerSignature.find("(");
                    if (attributesPos != std::string::npos) {
                        size_t endOfAttributesPos = innerSignature.find_last_of(")");
                        if (endOfAttributesPos - attributesPos > 1) {
                            std::string attributes = innerSignature.substr(attributesPos, endOfAttributesPos - attributesPos);
                            attributes = attributes.substr(1);
                            ProcessSignatureAdditionalTraits(section, cur, attributes, sourceData, result, parameter);
                            innerSignature = innerSignature.substr(0, attributesPos);
                            innerSignature = TrimString(innerSignature);
                        }
                    }

                    if (innerSignature.length() > 0) {
                        // Remove =
                        parameter.defaultValue = innerSignature;
                        parameter.defaultValue.erase(std::remove(parameter.defaultValue.begin(), parameter.defaultValue.end(), '='), parameter.defaultValue.end());
                        parameter.defaultValue.erase(std::remove(parameter.defaultValue.begin(), parameter.defaultValue.end(), '`'), parameter.defaultValue.end());
                        parameter.defaultValue = TrimString(parameter.defaultValue);
                    }
                }
                // Check possible required vs default clash
                if (parameter.use != OptionalParameterUse &&
                    !parameter.defaultValue.empty()) {
                    
                    // WARN: Required vs default clash
                    std::stringstream ss;
                    ss << "specifying parameter '" << parameter.name << "' as required supersedes its default value"\
                          ", declare the parameter as 'optional' to specify its default value";
                    
                    BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
                    SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, sourceData);
                    result.warnings.push_back(Warning(ss.str(),
                                                      LogicalErrorWarning,
                                                      sourceBlock));
                }
            }
            else {
                // ERR: unable to parse 
                BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, sourceData);
                result.error = (Error("unable to parse parameter specification",
                                      BusinessError,
                                      sourceBlock));
            }
        }

        
        /** Parse additional parameter attributes from abbrev definition bracket */
        static void ProcessSignatureAdditionalTraits(const BlueprintSection& section,
                                                     const BlockIterator& cur,
                                                     const SourceData& additionalTraits,
                                                     const SourceData& sourceData,
                                                     Result& result,
                                                     Parameter& parameter)
        {
            
            // Cherry pick example value, if any
            std::string source = additionalTraits;
            TrimString(source);
            CaptureGroups captureGroups;
            if (RegexCapture(source, snowcrashconst::AdditionalTraitsExampleRegex, captureGroups) &&
                captureGroups.size() > 1) {
                
                parameter.exampleValue = captureGroups[1];
                std::string::size_type pos = source.find(captureGroups[0]);
                if (pos != std::string::npos)
                    source.replace(pos, captureGroups[0].length(), std::string());
            }
            
            // Cherry pick use attribute, if any
            captureGroups.clear();
            if (RegexCapture(source, snowcrashconst::AdditionalTraitsUseRegex, captureGroups) &&
                captureGroups.size() > 1) {
                
                parameter.use = (RegexMatch(captureGroups[1], snowcrashconst::ParameterOptionalRegex)) ? OptionalParameterUse : RequiredParameterUse;

                std::string::size_type pos = source.find(captureGroups[0]);
                if (pos != std::string::npos)
                    source.replace(pos, captureGroups[0].length(), std::string());
            }
            
            // Finish with type
            captureGroups.clear();
            if (RegexCapture(source, snowcrashconst::AdditionalTraitsTypeRegex, captureGroups) &&
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
                std::stringstream ss;
                ss << "unable to parse additional parameter traits";
                ss << ", expected '([required | optional], [<type>], [`<example value>`])'";
                ss << ", e.g. '(optional, string, `Hello World`)'";

                BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, sourceData);
                result.warnings.push_back(Warning(ss.str(),
                                                  FormattingWarning,
                                                  sourceBlock));
                
                parameter.type.clear();
                parameter.exampleValue.clear();
                parameter.use = UndefinedParameterUse;
            }
        }
        
        /** Parse possible values enumeration section blocks. */
        static ParseSectionResult HandleValuesSection(const BlueprintSection& section,
                                                      const BlockIterator& cur,
                                                      BlueprintParserCore& parser,
                                                      Parameter& parameter) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            
            // Check redefinition
            if (!parameter.values.empty()) {
                // WARN: parameter values are already defined
                std::stringstream ss;
                ss << "overshadowing previous 'values' definition";
                ss << " for parameter '" << parameter.name << "'";
                
                BlockIterator nameBlock = ListItemNameBlock(cur, section.bounds.second);
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(nameBlock, cur, section.bounds, parser.sourceData);
                result.first.warnings.push_back(Warning(ss.str(),
                                                        RedefinitionWarning,
                                                        sourceBlock));
            }
            
            // Clear any previous content
            parameter.values.clear();
            
            // Check additional content in signature
            CheckSignatureAdditionalContent(section,
                                            cur,
                                            parser.sourceData,
                                            "'values:' keyword",
                                            snowcrashconst::ExpectedValuesContent,
                                            result.first);

            // Parse inner list of entities
            BlockIterator sectionCur = SkipSignatureBlock(cur, section.bounds.second);
            BlockIterator endCur = cur;
            if (endCur->type == ListBlockBeginType)
                ++endCur;
            endCur = SkipToClosingBlock(endCur, section.bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
            
            if (sectionCur != endCur) {

                // Iterate over list blocks, try to parse any nested lists of possible elements
                for (; sectionCur != endCur; ++sectionCur) {
                    
                    if (sectionCur->type == QuoteBlockBeginType)
                        sectionCur = SkipToClosingBlock(sectionCur, endCur, QuoteBlockBeginType, QuoteBlockEndType);
                    
                    bool entitiesParsed = false;
                    if (sectionCur->type == ListBlockBeginType) {
                        if (parameter.values.empty()) {
                            
                            // Try to parse some values
                            ParseSectionResult valuesResult = ParseValuesEntities(sectionCur,
                                                                                  section.bounds,
                                                                                  parser,
                                                                                  parameter.values);
                            result.first += valuesResult.first;
                            sectionCur = valuesResult.second;
                            if (result.first.error.code != Error::OK)
                                return result;

                            entitiesParsed = true;
                        }
                        else {
                            sectionCur = SkipToClosingBlock(sectionCur, endCur, ListBlockBeginType, ListBlockEndType);
                        }
                    }
                    
                    if (!entitiesParsed) {
                        // WARN: ignoring extraneous content
                        std::stringstream ss;
                        ss << "ignoring additional content in the 'values' attribute of the '";
                        ss << parameter.name << "' parameter";
                        ss << ", " << snowcrashconst::ExpectedValuesContent;
                        
                        SourceCharactersBlock sourceBlock = CharacterMapForBlock(sectionCur, cur, section.bounds, parser.sourceData);
                        result.first.warnings.push_back(Warning(ss.str(),
                                                                IgnoringWarning,
                                                                sourceBlock));
                    }
                }
            }
            
            if (parameter.values.empty()) {
                // WARN: empty definition
                std::stringstream ss;
                ss << "no possible values specified for parameter '" << parameter.name << "'";
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(sectionCur, cur, section.bounds, parser.sourceData);
                result.first.warnings.push_back(Warning(ss.str(),
                                                        EmptyDefinitionWarning,
                                                        sourceBlock));
            }

            if((!parameter.exampleValue.empty() || !parameter.defaultValue.empty()) && !parameter.values.empty()) {
                CheckExampleAndDefaultValue(section, sectionCur, parser, parameter, result);
            }
            
            endCur = CloseList(sectionCur, section.bounds.second);
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
                
                sectionCur = SkipToClosingBlock(sectionCur, bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
                
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
                    
                    SourceCharactersBlock sourceBlock = CharacterMapForBlock(sectionCur, cur, bounds, parser.sourceData);
                    result.first.warnings.push_back(Warning(ss.str(),
                                                            IgnoringWarning,
                                                            sourceBlock));
                }
                
                ++sectionCur;
            }
            
            result.second = sectionCur;
            return result;
        }
        
        static void CheckExampleAndDefaultValue(const BlueprintSection& section,
                                                const BlockIterator& cur,
                                                const BlueprintParserCore& parser,
                                                const Parameter& parameter,
                                                ParseSectionResult& result) {

            bool isExampleFound = false;
            bool isDefaultFound = false;

            for (Collection<Value>::const_iterator it = parameter.values.begin(); it != parameter.values.end(); ++it){
                if(parameter.exampleValue == *it) {
                    isExampleFound = true;
                }
                if(parameter.defaultValue == *it) {
                    isDefaultFound = true;
                }
            }

            if(!parameter.exampleValue.empty() && !isExampleFound) {
                // WARN: missing example in values.
                std::stringstream ss;
                ss << "the example value '" << parameter.exampleValue << "' of parameter '"<< parameter.name <<"' is not in its list of expected values";
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(cur, section.bounds.second, section.bounds, parser.sourceData);
                result.first.warnings.push_back(Warning(ss.str(),
                                                LogicalErrorWarning,
                                                sourceBlock));
            }

            if(!parameter.defaultValue.empty() && !isDefaultFound) {
                // WARN: missing default in values.
                std::stringstream ss;
                ss << "the default value '" << parameter.defaultValue << "' of parameter '"<< parameter.name <<"' is not in its list of expected values";
                SourceCharactersBlock sourceBlock = CharacterMapForBlock(cur, section.bounds.second, section.bounds, parser.sourceData);
                result.first.warnings.push_back(Warning(ss.str(),
                                                LogicalErrorWarning,
                                                sourceBlock));
            }

            return;
        }


    };
    
    typedef BlockParser<Parameter, SectionParser<Parameter> > ParameterDefinitionParser;
}

#endif
