//
//  Platform.h
//  snowcrash
//
//  Created by Zdenek Nemec on 7/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_PLATFORM_H
#define SNOWCRASH_PLATFORM_H

#if defined(_MSC_VER)
#   define FORCEINLINE __forceinline
#elif defined(__clang__) || defined(__GNUC__)
#   define FORCEINLINE inline
#else 
#   define FORCEINLINE inline
#endif

#endif
