//
//  version.h
//  snowcrash
//
//  Created by Zdenek Nemec on 4/14/14.
//
//  Attribution Notice:
//  This work might use parts of Node.js `node_version.h`
//  https://github.com/joyent/node
//
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_VERSION_H
#define SNOWCRASH_VERSION_H

#define SNOWCRASH_MAJOR_VERSION 0
#define SNOWCRASH_MINOR_VERSION 12
#define SNOWCRASH_PATCH_VERSION 0

#define SNOWCRASH_VERSION_IS_RELEASE 1

#ifndef SNOWCRASH_STRINGIFY
#   define SNOWCRASH_STRINGIFY(n) SNOWCRASH_STRINGIFY_HELPER(n)
#   define SNOWCRASH_STRINGIFY_HELPER(n) #n
#endif

#define SNOWCRASH_VERSION_STRING_HELPER "v" \
                                        SNOWCRASH_STRINGIFY(SNOWCRASH_MAJOR_VERSION) "." \
                                        SNOWCRASH_STRINGIFY(SNOWCRASH_MINOR_VERSION) "." \
                                        SNOWCRASH_STRINGIFY(SNOWCRASH_PATCH_VERSION)

#if SNOWCRASH_VERSION_IS_RELEASE
#   define SNOWCRASH_VERSION_STRING SNOWCRASH_VERSION_STRING_HELPER
#else
#   define SNOWCRASH_VERSION_STRING SNOWCRASH_VERSION_STRING_HELPER "-pre"
#endif

#endif
