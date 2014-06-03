//
//  Platform.h
//  snowcrash
//
//  Created by Zdenek Nemec on 7/2/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_PLATFORM_H
#define SNOWCRASH_PLATFORM_H

#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)

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

#ifndef SC_API
#  ifdef _WIN32
#     if defined(CSNOWCRASH_BUILD_SHARED) /* build dll */
#         define SC_API __declspec(dllimport)
#     elif !defined(CSNOWCRASH_BUILD_STATIC) /* use dll */
#         define SC_API __declspec(dllexport)
#     else /* static library */
#         define SC_API
#     endif
#  else
#     if __GNUC__ >= 4
#         define SC_API /*__attribute__((visibility("default")))*/
#     else
#         define SC_API
#     endif
#  endif
#endif

#endif
