//
//  snowcrash.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 4/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#include "snowcrash.h"
#include "BlueprintParser.h"

const int snowcrash::SourceAnnotation::OK = 0;

using namespace snowcrash;

/** 
 *  \brief  Check source for unsupported character \t & \r
 *  \return True if passed (not found), false otherwise
 */
static bool CheckSource(const mdp::ByteBuffer& source, Report& report) {

    std::string::size_type pos = source.find("\t");

    if (pos != std::string::npos) {
        
        mdp::BytesRangeSet rangeSet;
        rangeSet.push_back(mdp::BytesRange(pos, 1));
        report.error = Error("the use of tab(s) '\\t' in source data isn't currently supported, please contact makers",
                             BusinessError,
                             mdp::BytesRangeSetToCharactersRangeSet(rangeSet, source));
        return false;
    }
    
    pos = source.find("\r");

    if (pos != std::string::npos) {

        mdp::BytesRangeSet rangeSet;
        rangeSet.push_back(mdp::BytesRange(pos, 1));
        report.error = Error("the use of carriage return(s) '\\r' in source data isn't currently supported, please contact makers",
                             BusinessError,
                             mdp::BytesRangeSetToCharactersRangeSet(rangeSet, source));
        return false;
    }
    
    return true;
}

int snowcrash::parse(const mdp::ByteBuffer& source,
                     BlueprintParserOptions options,
                     Report& report,
                     Blueprint& blueprint,
                     BlueprintSM& blueprintSM) {

    try {
        
        // Sanity Check
        if (!CheckSource(source, report))
            return report.error.code;
        
        // Do nothing if blueprint is empty
        if (source.empty())
            return report.error.code;

        // Parse Markdown
        mdp::MarkdownParser markdownParser;
        mdp::MarkdownNode markdownAST;
        markdownParser.parse(source, markdownAST);

        // Build SectionParserData
        SectionParserData pd(options, source, blueprint);

        // Parse Blueprint
        BlueprintParser::parse(markdownAST.children().begin(), markdownAST.children(), pd, report, blueprint, blueprintSM);
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
