#include "HeadersParser.h"

using namespace snowcrash;

/** Finds a header in its containment group by its key (first) */
static HeaderIterator findHeader(const Headers& headers,
                                 const Header& header) {

    return std::find_if(headers.begin(),
        headers.end(),
        std::bind2nd(MatchFirsts<Header, IEqual<Header::first_type> >(), header));
}

typedef std::vector<std::string> HeadersKeyCollection;

/** Get collection of allowed keywords - workarround due to C++98 restriction - static initialization of vector */
static const HeadersKeyCollection& getAllowedMultipleDefinitions() {

    static std::string keys[] = {
        HTTPHeaderName::SetCookie,
        HTTPHeaderName::Link,
    };

    static const HeadersKeyCollection allowedMultipleDefinitions(keys, keys + (sizeof(keys)/sizeof(keys[0])));

    return allowedMultipleDefinitions;
}

/** Check if Header name has allowed multiple definitions */
static bool isAllowedMultipleDefinition(const Header& header) {

    const HeadersKeyCollection& keys = getAllowedMultipleDefinitions();

    return std::find_if(keys.begin(),
        keys.end(),
        std::bind1st(MatchFirstWith<Header, std::string, IEqual<std::string> >(), header)) != keys.end();
}

bool ColonChecker::operator()() const {

    return captures[2].size() >= 1 && captures[2][0] == ':';
}

std::string ColonChecker::getMessage() const {

    return "missing colon after header name";
}

bool HeadersDuplicateChecker::operator()() const {

    return findHeader(headers, header) == headers.end() || 
        isAllowedMultipleDefinition(header);
}

std::string HeadersDuplicateChecker::getMessage() const {

    std::stringstream ss;
    ss << "duplicate definition of '" << header.first << "' header";

    return ss.str();
}

bool HeaderParserValidator::operator()(const ValidateFunctorBase& rule) {
    bool rc = rule();

    if(!rc) {
        out.report.warnings.push_back(Warning(rule.getMessage(),
            HTTPWarning,
            sourceMap));
    }

    return rc;
}

