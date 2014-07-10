//
//  snowcrash.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrash.h"
#include "MarkdownParser.h"
#include "BlueprintParser.h"

const int snowcrash::SourceAnnotation::OK = 0;

using namespace snowcrash;

/** 
 *  \brief  Check source for unsupported character \t & \r
 *  \return True if passed (not found), false otherwise
 */
static bool CheckSource(const mdp::ByteBuffer& source, Report& report)
{
    std::string::size_type pos = source.find("\t");

    if (pos != std::string::npos) {
        
        mdp::BytesRangeSet rangeSet;
        rangeSet.push_back(mdp::BytesRange(0, source.length()));
        report.error = Error("the use of tab(s) '\\t' in source data isn't currently supported, please contact makers",
                             BusinessError,
                             mdp::BytesRangeSetToCharactersRangeSet(mdp::BytesRangeSet(), source));
        return false;
    }
    
    pos = source.find("\r");

    if (pos != std::string::npos) {

        mdp::BytesRangeSet rangeSet;
        rangeSet.push_back(mdp::BytesRange(0, source.length()));
        report.error = Error("the use of carriage return(s) '\\r' in source data isn't currently supported, please contact makers",
                             BusinessError,
                             mdp::BytesRangeSetToCharactersRangeSet(mdp::BytesRangeSet(), source));
        return false;
    }
    
    return true;
}

int snowcrash::parse(const mdp::ByteBuffer& source,
                     BlueprintParserOptions options,
                     Report& report,
                     Blueprint& blueprint)
{
    try {
        
        // Sanity Check
        if (!CheckSource(source, report))
            return report.error.code;
        
        // Parse Markdown
        mdp::MarkdownParser markdownParser;
        mdp::MarkdownNode markdownAST;
        markdownParser.parse(source, markdownAST);

        // Build SectionParserData
        SectionParserData pd(options, source, blueprint);

        // Parse Blueprint
        BlueprintParser::parse(markdownAST.children().begin(), markdownAST.children(), pd, report, pd.blueprint);
    }
    catch (const std::exception& e) {
        
        std::stringstream ss;
        ss << "parser exception: '" << e.what() << "'";
        report.error = Error(ss.str(), 1);
    }
    catch (...) {
        
        report.error = Error("parser exception has occured", 1);
    }

    return report.error.code;
}
