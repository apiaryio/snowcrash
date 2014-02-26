//
//  Platform.h
//  snowcrash
//
//  Created by Zdenek Nemec on 7/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_PLATFORM_H
#define SNOWCRASH_PLATFORM_H

// TODO:
#define DEPRECATED

#if defined(_MSC_VER)
#   define FORCEINLINE __forceinline
//#   define DEPRECATED __declspec(deprecated)
#elif defined(__clang__) || defined(__GNUC__)
#   define FORCEINLINE inline
//#   define DEPRECATED __attribute__((deprecated))
#else
#   define FORCEINLINE inline
//#   define DEPRECATED
#endif

#endif
