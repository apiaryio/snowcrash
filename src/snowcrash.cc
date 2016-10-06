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

// http://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

/**
 *  \brief  Check source for unsupported character \t & \r
 *  \return True if passed (not found), false otherwise
 */
static bool CheckSource(const mdp::ByteBuffer& source, Report& report)
{

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
        replaceAll(source, "\r", "\n");
    }

    return true;
}

int snowcrash::parse(const mdp::ByteBuffer& source,
                     BlueprintParserOptions options,
                     const ParseResultRef<Blueprint>& out)
{
    try {

        // Sanity Check
        if (!CheckSource(source, out.report))
            return out.report.error.code;

        // Do nothing if blueprint is empty
        if (source.empty())
            return out.report.error.code;

        // Parse Markdown
        mdp::MarkdownParser markdownParser;
        mdp::MarkdownNode markdownAST;
        markdownParser.parse(source, markdownAST);

        // Build SectionParserData
        SectionParserData pd(options, source, out.node);
        mdp::BuildCharacterIndex(pd.sourceCharacterIndex, source);

        // Parse Blueprint
        BlueprintParser::parse(markdownAST.children().begin(), markdownAST.children(), pd, out);
    }
    catch (const Error& e) {
        out.report.error = e;
    }
    catch (const std::exception& e) {

        std::stringstream ss;
        ss << "parser exception: '" << e.what() << "'";
        out.report.error = Error(ss.str(), ApplicationError);
    }
    catch (...) {
        out.report.error = Error("parser exception has occured", ApplicationError);
    }

    return out.report.error.code;
}
