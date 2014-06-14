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
#         define CSNOWCRASH __attribute__((visibility("default")))
#     else
#         define CSNOWCRASH
#     endif
#  endif
#endif

#include <vector>
#include <string>
#include <utility>
#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif
    
CSNOWCRASH typedef std::string C_BluePrint_Name;

CSNOWCRASH typedef std::string C_BluePrint_Description;

CSNOWCRASH typedef std::string C_BluePrint_URI;

CSNOWCRASH typedef std::string C_BluePrint_URITemplate;

CSNOWCRASH typedef std::string C_BluePrint_HTTPMethod;

CSNOWCRASH typedef std::string C_BluePrint_Type;

CSNOWCRASH typedef std::string C_BluePrint_Value;
 
CSNOWCRASH typedef std::pair<std::string, std::string> C_BluePrint_KeyValuePair;

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
    typedef std::vector<C_BluePrint_Value> type;
    typedef std::vector<C_BluePrint_Value>::iterator iterator;
    typedef std::vector<C_BluePrint_Value>::const_iterator const_iterator;
};

CSNOWCRASH typedef std::string C_BluePrint_Asset;

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
    
    C_BluePrint_Value_Collection::type values;
};

struct C_BluePrint_Parameter_Collection {
    typedef std::vector<C_BluePrint_Parameter> type;
    typedef std::vector<C_BluePrint_Parameter>::iterator iterator;
    typedef std::vector<C_BluePrint_Parameter>::const_iterator const_iterator;
};

struct C_BluePrint_Header_Collection {
    typedef std::vector<C_BluePrint_Header> type;
    typedef std::vector<C_BluePrint_Header>::iterator iterator;
    typedef std::vector<C_BluePrint_Header>::const_iterator const_iterator;
};

struct C_BluePrint_Payload {
    
    C_BluePrint_Name name;
    
    C_BluePrint_Description description;
    
    C_BluePrint_Parameter_Collection::type parameters;
    
    C_BluePrint_Header_Collection::type headers;
    
    C_BluePrint_Asset body;
    
    C_BluePrint_Asset schema;
};

CSNOWCRASH typedef C_BluePrint_Payload C_BluePrint_ResourceModel;

CSNOWCRASH typedef C_BluePrint_Payload C_BluePrint_Request;

CSNOWCRASH typedef C_BluePrint_Payload C_BluePrint_Response;

struct C_BluePrint_Request_Collection {
    typedef std::vector<C_BluePrint_Request> type;
    typedef std::vector<C_BluePrint_Request>::iterator iterator;
    typedef std::vector<C_BluePrint_Request>::const_iterator const_iterator;
};

struct C_BluePrint_Response_Collection {
    typedef std::vector<C_BluePrint_Response> type;
    typedef std::vector<C_BluePrint_Response>::iterator iterator;
    typedef std::vector<C_BluePrint_Response>::const_iterator const_iterator;
};

struct C_BluePrint_TransactionExample {
    
    C_BluePrint_Name name;
    
    C_BluePrint_Description description;
    
    C_BluePrint_Request_Collection::type requests;
    
    C_BluePrint_Response_Collection::type responses;
};

struct C_BluePrint_TransactionExample_Collection {
    typedef std::vector<C_BluePrint_TransactionExample> type;
    typedef std::vector<C_BluePrint_TransactionExample>::iterator iterator;
    typedef std::vector<C_BluePrint_TransactionExample>::const_iterator const_iterator;
};

struct C_BluePrint_Action {
    
    C_BluePrint_HTTPMethod method;
    
    C_BluePrint_Name name;
    
    C_BluePrint_Description description;
    
    C_BluePrint_Parameter_Collection::type parameters;
    
    C_BluePrint_Header_Collection::type headers;
    
    C_BluePrint_TransactionExample_Collection::type examples;
};

struct C_BluePrint_Action_Collection {
    typedef std::vector<C_BluePrint_Action> type;
    typedef std::vector<C_BluePrint_Action>::iterator iterator;
    typedef std::vector<C_BluePrint_Action>::const_iterator const_iterator;
};

struct C_BluePrint_Resource {
    
    C_BluePrint_URITemplate uriTemplate;
    
    C_BluePrint_Name name;
    
    C_BluePrint_Description description;
    
    C_BluePrint_ResourceModel model;
    
    C_BluePrint_Parameter_Collection::type parameters;
    
    C_BluePrint_Header_Collection::type headers;
    
    C_BluePrint_Action_Collection::type actions;
};

struct C_BluePrint_Resource_Collection {
    typedef std::vector<C_BluePrint_Resource> type;
    typedef std::vector<C_BluePrint_Resource>::iterator iterator;
    typedef std::vector<C_BluePrint_Resource>::const_iterator const_iterator;
};

struct C_BluePrint_ResourceGroup {
    
    C_BluePrint_Name name;
    
    C_BluePrint_Description description;
    
    C_BluePrint_Resource_Collection::type resources;
};

struct C_BluePrint_Metadata_Collection {
    typedef std::vector<C_BluePrint_Metadata> type;
    typedef std::vector<C_BluePrint_Metadata>::iterator iterator;
    typedef std::vector<C_BluePrint_Metadata>::const_iterator const_iterator;
};

struct C_BluePrint_ResourceGroup_Collection {
    typedef std::vector<C_BluePrint_ResourceGroup> type;
    typedef std::vector<C_BluePrint_ResourceGroup>::iterator iterator;
    typedef std::vector<C_BluePrint_ResourceGroup>::const_iterator const_iterator;
};

struct C_Blueprint {
    
    C_BluePrint_Metadata_Collection::type metadata;
    
    C_BluePrint_Name name;

    C_BluePrint_Description description;
    
    C_BluePrint_ResourceGroup_Collection::type resourceGroups;
};

#ifdef __cplusplus
}
#endif

#endif
