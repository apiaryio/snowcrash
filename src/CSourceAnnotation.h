//
//  CSourceAnnotation.h
//  snowcrash
//  C Implementation of SourceAnnotation.h for binding purposes
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_C_SOURCEANNOTATION_H
#define SNOWCRASH_C_SOURCEANNOTATION_H

#ifndef CSNOWCRASH
#  ifdef _WIN32
#     if defined(CSNOWCRASH_BUILD_SHARED) /* build dll */
#         define CSNOWCRASH __declspec(dllexport)
#     elif !defined(CSNOWCRASH_BUILD_STATIC) /* use dll */
#         define CSNOWCRASH __declspec(dllimport)
#     else /* static library */
#         define CSNOWCRASH
#     endif
#  else
#     if __GNUC__ >= 4
#         define CSNOWCRASH /*__attribute__((visibility("default")))*/
#     else
#         define CSNOWCRASH
#     endif
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct C_SourceCharactersRange {
    unsigned int location;
    unsigned int length;
};

struct C_SourceCharactersBlock{
    C_SourceCharactersRange* SourceCharactersRange_array;
    int size;
};

struct C_SourceAnnotation {
    
    static const int OK;
    
    C_SourceCharactersBlock location;
    
    int code;
    
    char* message;
};

typedef struct C_SourceAnnotation C_SourceAnnotation_t;

CSNOWCRASH typedef C_SourceAnnotation C_Error;

enum C_ErrorCode {
    C_NoError = 0,
    C_ApplicationError = 1,
    C_BusinessError = 2,
    C_SymbolError = 3
};

CSNOWCRASH typedef C_SourceAnnotation C_Warning;

enum C_WarningCode {
    C_NoWarning = 0,
    C_APINameWarning = 1,
    C_DuplicateWarning = 2,
    C_FormattingWarning = 3,
    C_RedefinitionWarning = 4,
    C_IgnoringWarning = 5,
    C_EmptyDefinitionWarning = 6,
    C_NotEmptyDefinitionWarning = 7,
    C_LogicalErrorWarning = 8,
    C_DeprecatedWarning = 9,
    C_IndentationWarning = 10,
    C_AmbiguityWarning = 11,
    C_URIWarning = 12
};

struct C_Warnings
{
    C_Warning* warnings_array;
    int size;
};

struct C_Result {

    C_Error error;

    C_Warnings warnings;
};

#ifdef __cplusplus
}
#endif

#endif
