//
//  MSONUtility.h
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/23/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONUTILITY_H
#define SNOWCRASH_MSONUTILITY_H

#include "MSONSourcemap.h"

namespace mson {

    /**
     * \brief Parse Value from a value string
     *
     * \param subject String which represents the value
     *
     * \return MSON Value
     */
    inline Value parseValue(std::string& subject) {

        Value value;
        size_t len = subject.length();

        if ((subject[0] == '*' && subject[len - 1] == '*') ||
            (subject[0] == '_' && subject[len - 1] == '_')) {

            std::string escapedString = snowcrash::RetrieveEscaped(subject, 0, true);

            value.literal = escapedString;
            value.variable = true;
        }

        if (value.literal.empty()) {

            value.literal = subject;
            value.variable = false;
        }

        return value;
    }
}

#endif
