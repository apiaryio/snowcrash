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
#define PARAMETER_VALUE "`(.+)`"

/** Parameter Definition matching regex */
static const std::string ParameterDefinitionRegex("^[ \\t]*(" SYMBOL_IDENTIFIER ")[ \\t]*$");

/** Parameter Type matching regex */
static const std::string ParameterTypeRegex("^[ \\t]*[Tt]ype:[ \\t]*(" SYMBOL_IDENTIFIER ")$");

/** Parameter Required matching regex */
static const std::string ParameterRequiredRegex("^[ \\t]*[Rr]equired[ \\t]*$");

/** Parameter Optional matching regex */
static const std::string ParameterOptionalRegex("^[ \\t]*[Oo]ptional[ \\t]*$");

/** Parameter Default matching regex */
static const std::string ParameterDefaultRegex("^[ \\t]*[Dd]efault:[ \\t]*" PARAMETER_VALUE "[ \\t]*$");

/** Parameter Example matching regex */
static const std::string ParameterExampleRegex("^[ \\t]*[Ee]xample:[ \\t]*" PARAMETER_VALUE "[ \\t]*$");

/** Parameter Values matching regex */
static const std::string ParameterValuesRegex("^[ \\t]*[Vv]alues:[ \\t]*$");

/** List of expected keywords */
static const std::string ExpectedDefinitionItems = "`Type: <type>`, `Optional`, `Required`, "\
"`Default: `<default value>``, `Example: `<example value>`` or `Values:`";


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
        
        if (RegexMatch(content, ParameterTypeRegex))
            return ParameterTypeSection;
        
        if (RegexMatch(content, ParameterRequiredRegex))
            return ParameterRequiredSection;
        
        if (RegexMatch(content, ParameterOptionalRegex))
            return ParameterOptionalSection;
        
        if (RegexMatch(content, ParameterDefaultRegex))
            return ParameterDefaultSection;
        
        if (RegexMatch(content, ParameterExampleRegex))
            return ParameterExampleSection;
        
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
        return RegexMatch(content, ParameterDefinitionRegex);
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
        else if (context == ParameterTypeSection ||
                 context == ParameterRequiredSection ||
                 context == ParameterOptionalSection ||
                 context == ParameterDefaultSection ||
                 context == ParameterExampleSection ||
                 context == ParameterValuesSection ||
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
                    
                case ParameterTypeSection:
                case ParameterDefaultSection:
                case ParameterExampleSection:
                    result = HandleKeywordValueSection(section, cur, bounds, parser, parameter);
                    break;
                    
                case ParameterOptionalSection:
                case ParameterRequiredSection:
                    result = HandleOptionSection(section, cur, bounds, parser, parameter);
                    break;
                    
                case ParameterValuesSection:
                    result = HandleValuesSection(cur, bounds, parser, parameter);
                    break;
                    
                case ForeignSection:
                    result = HandleForeignSection(cur, bounds, ExpectedDefinitionItems);
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
            
            SourceData remainingContent;
            SourceData signature = GetListItemSignature(begin, end, remainingContent);

            // FIXME: For now consider complete signature as the parameter name
            parameter.name = TrimString(signature);
            
            // Set default values
            parameter.use = UndefinedParameterUse;
        }
        
        /** Parse parameter type section blocks. */
        static ParseSectionResult HandleKeywordValueSection(const Section& section,
                                                            const BlockIterator& cur,
                                                            const SectionBounds& bounds,
                                                            BlueprintParserCore& parser,
                                                            Parameter& parameter) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);

            SourceData remainingContent;
            SourceData content = GetListItemSignature(cur, bounds.second, remainingContent);
            content = TrimString(content);
            // TODO: check remainingContent for remnants

            // Retrieve type
            CaptureGroups captureGroups;
            
            // Section-specific retrieval
            switch (section) {
                case ParameterTypeSection:
                {
                    RegexCapture(content, ParameterTypeRegex, captureGroups);
                    if (captureGroups.size() > 1) {
                        std::string type = captureGroups[1];
                        parameter.type = TrimString(type);
                    }
                }
                    break;
                    
                case ParameterDefaultSection:
                {
                    RegexCapture(content, ParameterDefaultRegex, captureGroups);
                    if (captureGroups.size() > 1) {
                        std::string type = captureGroups[1];
                        parameter.defaultValue = TrimString(type);
                    }
                }
                    break;
                    
                case ParameterExampleSection:
                {
                    RegexCapture(content, ParameterExampleRegex, captureGroups);
                    if (captureGroups.size() > 1) {
                        std::string type = captureGroups[1];
                        parameter.exampleValue = TrimString(type);
                    }
                }
                    break;

                    
                default:
                    break;
            }
            
            // TODO: check superluous nested blocks
            
            // Close list item
            BlockIterator endCur = cur;
            if (endCur->type == ListBlockBeginType)
                ++endCur;

            endCur = SkipToSectionEnd(endCur, bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
            endCur = CloseListItemBlock(endCur, bounds.second);
            result.second = endCur;
            return result;
        }
        
        /** Parse optional & required section blocks. */
        static ParseSectionResult HandleOptionSection(const Section& section,
                                                      const BlockIterator& cur,
                                                      const SectionBounds& bounds,
                                                      BlueprintParserCore& parser,
                                                      Parameter& parameter) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            
            // TODO: check superfluous content in signature
            
            parameter.use = (section == ParameterRequiredSection) ? RequiredParameterUse : OptionalParameterUse;
            
            // Close list item
            BlockIterator endCur = cur;
            if (endCur->type == ListBlockBeginType)
                ++endCur;
            
            endCur = SkipToSectionEnd(endCur, bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
            endCur = CloseListItemBlock(endCur, bounds.second);
            result.second = endCur;
            
            return result;
        }
        
        /** Parse possible values enumeration section blocks. */
        static ParseSectionResult HandleValuesSection(const BlockIterator& cur,
                                                      const SectionBounds& bounds,
                                                      BlueprintParserCore& parser,
                                                      Parameter& parameter) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            
            // TODO: check existing values in parameter
            parameter.values.clear();
            
            // TODO: check superfluous content in signature
            
            BlockIterator sectionCur = FirstContentBlock(cur, bounds.second);
            
            if (sectionCur == bounds.second ||
                sectionCur->type != ListBlockBeginType) {
                // WARN: expected nested elements
                BlockIterator nameBlock = ListItemNameBlock(cur, bounds.second);
                std::stringstream ss;
                ss << "expected nested list of possible parameter values, one element per list item";
                result.first.warnings.push_back(Warning(ss.str(),
                                                        FormattingWarning,
                                                        nameBlock->sourceMap));
                
                result.second = nameBlock;
                return result;
            }
            
            ++sectionCur;
            if (sectionCur == bounds.second ||
                sectionCur->type != ListItemBlockBeginType) {
                // ERR: unexpected block
                BlockIterator nameBlock = ListItemNameBlock(cur, bounds.second);
                result.first.error = (Error("unexpected block",
                                            BusinessError,
                                            nameBlock->sourceMap));
            }
            
            while (sectionCur != bounds.second &&
                   sectionCur->type == ListItemBlockBeginType) {
                
                sectionCur = SkipToSectionEnd(sectionCur, bounds.second, ListItemBlockBeginType, ListItemBlockEndType);
                
                CaptureGroups captureGroups;
                RegexCapture(sectionCur->content, PARAMETER_VALUE, captureGroups);
                if (captureGroups.size() > 1) {
                    parameter.values.push_back(captureGroups[1]);
                }
                
                ++sectionCur;
            }
            
            if (!parameter.values.empty())
                sectionCur = CloseListItemBlock(sectionCur, bounds.second);
            
            sectionCur = CloseListItemBlock(sectionCur, bounds.second);
            result.second = sectionCur;
            return result;
        }
    };
    
    typedef BlockParser<Parameter, SectionParser<Parameter> > ParameterDefinitionParser;
}

#endif
