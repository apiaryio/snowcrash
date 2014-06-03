//
//  Platform.h
//  snowcrash
//
//  Created by Zdenek Nemec on 7/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_PLATFORM_H
#define SNOWCRASH_PLATFORM_H

#if defined(BUILDING_SNOWCRASH)
#   define DEPRECATED
#endif

#if defined(_MSC_VER)
#   if !defined(DEPRECATED)
#       define DEPRECATED __declspec(deprecated)
#   endif
#elif defined(__clang__) || defined(__GNUC__)
#   if !defined(DEPRECATED)
#       define DEPRECATED __attribute__((deprecated))
#   endif
#else
#   if !defined(DEPRECATED)
#       define DEPRECATED
#   endif
#endif

#endif
