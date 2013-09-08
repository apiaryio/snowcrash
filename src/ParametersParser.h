//
//  ParametersParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 9/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_PARAMETERSPARSER_H
#define SNOWCRASH_PARAMETERSPARSER_H

#include <sstream>
#include "BlueprintParserCore.h"
#include "Blueprint.h"
#include "ListUtility.h"
#include "RegexMatch.h"
#include "StringUtility.h"
#include "ParameterDefinitonParser.h"

/** Parameters matching regex */
static const std::string ParametersRegex("^[ \\t]*[Pp]arameters?[ \\t]*$");

/** Expected parameters content */
static const std::string ExpectedParametersContent = "'parameters' only followed by a nested list of parameters, "\
                                                     "one parameter per item";

namespace snowcrash {
    
    /** Internal type alias for Collection of Paramaeter */
    typedef Collection<Parameter>::type ParameterCollection;

    /**
     *  Returns true if given block has parameters signature, false otherwise.
     */
    FORCEINLINE bool HasParametersSignature(const BlockIterator& begin,
                                            const BlockIterator& end) {

        if (begin->type != ListBlockBeginType &&
            begin->type != ListItemBlockBeginType)
            return false;
        
        SourceData remainingContent;
        SourceData content = GetListItemSignature(begin, end, remainingContent);
        TrimString(content);
        return RegexMatch(content, ParametersRegex);
    }
    
    /**
     *  Block Classifier, ParameterCollection context.
     */
    template <>
    FORCEINLINE Section ClassifyBlock<ParameterCollection>(const BlockIterator& begin,
                                                           const BlockIterator& end,
                                                           const Section& context) {
        
        if (context == UndefinedSection) {
            if (HasParametersSignature(begin, end))
                return ParametersSection;
        }
        else if (context == ParametersSection) {
            
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSection;
            
            if (HasParameterDefinitionSignature(begin, end))
                return ParameterDefinitionSection;
            
            if (begin->type == ListBlockBeginType)
                return ForeignSection; // Foreign nested list-item
            
            if (begin->type == ListItemBlockBeginType)
                return UndefinedSection;
        }
        else if (context == ParameterDefinitionSection ||
                 context == ForeignSection) {
            
            if (begin->type == ListItemBlockEndType ||
                begin->type == ListBlockEndType)
                return UndefinedSection;
            
            if (HasParameterDefinitionSignature(begin, end))
                return ParameterDefinitionSection;
            
            return ForeignSection;
        }
        
        return (context == ParametersSection) ? context : UndefinedSection;
    }

    /**
     *  Parameters section parser.
     */
    template<>
    struct SectionParser<ParameterCollection> {
        
        static ParseSectionResult ParseSection(const Section& section,
                                               const BlockIterator& cur,
                                               const SectionBounds& bounds,
                                               BlueprintParserCore& parser,
                                               ParameterCollection& parameters) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            switch (section) {
                case ParametersSection:
                    result = HandleParmetersSection(cur, bounds, parser, parameters);
                    break;
                    
                case ParameterDefinitionSection:
                    result = HandleParmeterDefinitionSection(cur, bounds, parser, parameters);
                    break;
                    
                case ForeignSection:
                    result = HandleForeignSection(cur, bounds, "'<parameter identifier>'");
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
        
        /**
         *  \brief  Parse Parameters top-level section blocks.
         *  \param  cur     The actual position within Markdown block buffer.
         *  \param  bounds  Boundaries of Markdown block buffer.
         *  \param  parser  Parser's instance.
         *  \param  payload An output buffer to write parameters into.
         *  \return A block parser section result.
         */
        static ParseSectionResult HandleParmetersSection(const BlockIterator& cur,
                                                         const SectionBounds& bounds,
                                                         BlueprintParserCore& parser,
                                                         ParameterCollection& parameters) {
            
            ParseSectionResult result = std::make_pair(Result(), cur);
            BlockIterator sectionCur = cur;
            
            // Signature
            if (sectionCur == bounds.first) {
                
                CheckSignatureAdditionalContent(sectionCur, bounds, "'parameters' keyword", ExpectedParametersContent, result.first);
                result.second = SkipSignatureBlock(sectionCur, bounds.second);
                return result;
            }
            
            // Description
            if (sectionCur->type == QuoteBlockBeginType) {
                sectionCur = SkipToSectionEnd(sectionCur, bounds.second, QuoteBlockBeginType, QuoteBlockEndType);
            }
            else if (sectionCur->type == ListBlockBeginType) {
                sectionCur = SkipToSectionEnd(sectionCur, bounds.second, ListBlockBeginType, ListItemBlockEndType);
            }
            
            if (!CheckCursor(sectionCur, bounds, cur, result.first))
                return result;
            
            // WARN: on ignoring additional content
            std::stringstream ss;
            ss << "ignoring additional content in the 'parameters' definition, expected " << ExpectedParametersContent;
            result.first.warnings.push_back(Warning(ss.str(),
                                                    IgnoringWarning,
                                                    sectionCur->sourceMap));
            
            if (sectionCur != bounds.second)
                result.second = ++sectionCur;

            return result;
        }
        
        /** Parse a parameter definition top-level section blocks. */
        static ParseSectionResult HandleParmeterDefinitionSection(const BlockIterator& cur,
                                                                  const SectionBounds& bounds,
                                                                  BlueprintParserCore& parser,
                                                                  ParameterCollection& parameters) {
            Parameter parameter;
            ParseSectionResult result = ParameterDefinitionParser::Parse(cur, bounds.second, parser, parameter);
            if (result.first.error.code != Error::OK)
                return result;
            
            // TODO: check duplicates & elegibility
            parameters.push_back(parameter);
            
            return result;
        }
    };
    
    typedef BlockParser<ParameterCollection, SectionParser<ParameterCollection> > ParametersParser;
}


#endif
