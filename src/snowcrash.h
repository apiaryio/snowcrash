//
//  snowcrash.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_H
#define SNOWCRASH_H

#include "BlueprintSourcemap.h"
#include "SourceAnnotation.h"
#include "SectionParser.h"

/**
 *  API Blueprint Parser Interface
 *  ------------------------------
 *
 *  This is the parser's entry point.
 *
 *  For Snow Crash users, this is the only interface to use.
 *
 *  For binding writers, this is the point to start wrapping.
 *  Refer to https://github.com/apiaryio/snowcrash/wiki/Writing-a-binding
 *  for details on how to write a Snow Crash binding.
 */

namespace snowcrash {

    /**
     *  \brief Parse the source data into a blueprint abstract source tree (AST).
     *
     *  \param source       A textual source data to be parsed.
     *  \param options      Parser options. Use 0 for no additional options.
     *  \param out          Output buffer to store parsing result into.
     *  \return Error status code. Zero represents success, non-zero a failure.
     */
    int parse(const mdp::ByteBuffer& source,
              BlueprintParserOptions options,
              const ParseResultRef<Blueprint>& out);
}

#endif
