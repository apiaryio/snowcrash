//
//  CBluePrint.h
//  snowcrash
//  C Implementation of BluePrint.h for binding purposes
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_C_BLUEPRINT_H
#define SNOWCRASH_C_BLUEPRINT_H

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

CSNOWCRASH typedef char* CHARS;
    
CSNOWCRASH typedef CHARS C_BluePrint_Name;

CSNOWCRASH typedef CHARS C_BluePrint_Description;

CSNOWCRASH typedef CHARS C_BluePrint_URI;

CSNOWCRASH typedef CHARS C_BluePrint_URITemplate;

CSNOWCRASH typedef CHARS C_BluePrint_HTTPMethod;

CSNOWCRASH typedef CHARS C_BluePrint_Type;

CSNOWCRASH typedef CHARS C_BluePrint_Value;
 
//CSNOWCRASH typedef std::pair<CHARS, CHARS> C_BluePrint_KeyValuePair;
struct C_BluePrint_KeyValuePair 
{
    CHARS key;
    CHARS value;
};

/**
 * Default Container for collections.
 *
 *  FIXME: Use C++11 template aliases when migrating to C++11.
 */
//template<typename T>
//struct Collection {
//    typedef std::vector<T> type;
//    typedef typename std::vector<T>::iterator iterator;
//    typedef typename std::vector<T>::const_iterator const_iterator;
//};

struct C_BluePrint_Value_Collection {
    C_BluePrint_Value* Blueprint_Value_Array;
    unsigned int size;
};

CSNOWCRASH typedef CHARS C_BluePrint_Asset;

CSNOWCRASH typedef C_BluePrint_KeyValuePair C_BluePrint_Metadata;

CSNOWCRASH typedef C_BluePrint_KeyValuePair C_BluePrint_Header;

enum C_BluePrint_ParameterUse {
    C_UndefinedParameterUse,
    C_OptionalParameterUse,
    C_RequiredParameterUse
};

struct C_BluePrint_Parameter {
    
    C_BluePrint_Name name;
    
    C_BluePrint_Description description;
    
    C_BluePrint_Type type;
    
    C_BluePrint_ParameterUse use;
    
    C_BluePrint_Value defaultValue;
    
    C_BluePrint_Value exampleValue;
    
    C_BluePrint_Value_Collection values;
};

struct C_BluePrint_Parameter_Collection {
    C_BluePrint_Parameter* Blueprint_Parameter_Array;
    unsigned int size;
};

struct C_BluePrint_Header_Collection {
    C_BluePrint_Header* Blueprint_Header_array;
    unsigned int size;
};

struct C_BluePrint_Payload {
    
    C_BluePrint_Name name;
    
    C_BluePrint_Description description;
    
    C_BluePrint_Parameter_Collection parameters;
    
    C_BluePrint_Header_Collection headers;
    
    C_BluePrint_Asset body;
    
    C_BluePrint_Asset schema;
};

CSNOWCRASH typedef C_BluePrint_Payload C_BluePrint_ResourceModel;

CSNOWCRASH typedef C_BluePrint_Payload C_BluePrint_Request;

CSNOWCRASH typedef C_BluePrint_Payload C_BluePrint_Response;

struct C_BluePrint_Request_Collection {
    C_BluePrint_Request* Blueprint_Request_Array;
    unsigned int size;
};

struct C_BluePrint_Response_Collection {
    C_BluePrint_Response* Blueprint_Response_Array;
    unsigned int size;
};

struct C_BluePrint_TransactionExample {
    
    C_BluePrint_Name name;
    
    C_BluePrint_Description description;
    
    C_BluePrint_Request_Collection requests;
    
    C_BluePrint_Response_Collection responses;
};

struct C_BluePrint_TransactionExample_Collection {
    C_BluePrint_TransactionExample* Blueprint_TransactionExample_Array;
    unsigned int size;
};

struct C_BluePrint_Action {
    
    C_BluePrint_HTTPMethod method;
    
    C_BluePrint_Name name;
    
    C_BluePrint_Description description;
    
    C_BluePrint_Parameter_Collection parameters;
    
    C_BluePrint_Header_Collection headers;
    
    C_BluePrint_TransactionExample_Collection examples;
};

struct C_BluePrint_Action_Collection {
    C_BluePrint_Action* Blueprint_Action_Array;
    unsigned int size;
};

struct C_BluePrint_Resource {
    
    C_BluePrint_URITemplate uriTemplate;
    
    C_BluePrint_Name name;
    
    C_BluePrint_Description description;
    
    C_BluePrint_ResourceModel model;
    
    C_BluePrint_Parameter_Collection parameters;
    
    C_BluePrint_Header_Collection headers;
    
    C_BluePrint_Action_Collection actions;
};

struct C_BluePrint_Resource_Collection {
    C_BluePrint_Resource* Blueprint_Resource_Array;
    unsigned int size;
};

struct C_BluePrint_ResourceGroup {
    
    C_BluePrint_Name name;
    
    C_BluePrint_Description description;
    
    C_BluePrint_Resource_Collection resources;
};

struct C_BluePrint_Metadata_Collection {
    C_BluePrint_Metadata* Blueprint_Metadata_Array;
    unsigned int size;
};

struct C_BluePrint_ResourceGroup_Collection {
    C_BluePrint_ResourceGroup* Blueprint_ResourceGroup_Array;
    unsigned int size;
};

struct C_Blueprint {
    
    C_BluePrint_Metadata_Collection metadata;
    
    C_BluePrint_Name name;

    C_BluePrint_Description description;
    
    C_BluePrint_ResourceGroup_Collection resourceGroups;
};

#ifdef __cplusplus
}
#endif

#endif
