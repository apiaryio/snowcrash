//
//  CSnowcrash.c
//  snowcrash
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//
#include "csnowcrash.h"
#include "Parser.h"

void Copy_String_CharS(std::string str , char** chr)
{
    if (!str.empty()){
        *chr = new char[str.size() + 1];
        std::copy(str.begin(), str.end(), *chr);

        char* t_chr = *chr;
        t_chr[str.size()] = '\0';
    }
    else{
        *chr = new char[1];
        char* t_chr = *chr;
        t_chr[0] = '\0';
    }
}

void cast_header(snowcrash::Collection<snowcrash::Header>::type header , C_BluePrint_Header_Collection& c_header)
{
    int count = 0;
    c_header.size = header.size();
    if (c_header.size){
        c_header.Blueprint_Header_array = (C_BluePrint_Header*)malloc( sizeof(C_BluePrint_Header) * c_header.size );

        for (snowcrash::Collection<snowcrash::Header>::const_iterator it = header.begin(); it != header.end(); ++it){
            Copy_String_CharS(it->first , &c_header.Blueprint_Header_array[count].key);
            Copy_String_CharS(it->second , &c_header.Blueprint_Header_array[count].value);

            count++;
        }
    } 
    else{
        c_header.Blueprint_Header_array = NULL;
    }
}

void cast_parameters(snowcrash::Collection<snowcrash::Parameter>::type parameter , C_BluePrint_Parameter_Collection& c_parameter)
{
    unsigned int count = 0;
    c_parameter.size = parameter.size();
    if (c_parameter.size){
        c_parameter.Blueprint_Parameter_Array = (C_BluePrint_Parameter*)malloc( sizeof(C_BluePrint_Parameter) * c_parameter.size );

        for (snowcrash::Collection<snowcrash::Parameter>::const_iterator it = parameter.begin(); it != parameter.end(); ++it){
            Copy_String_CharS(it->name , &c_parameter.Blueprint_Parameter_Array[count].name);
            Copy_String_CharS(it->description , &c_parameter.Blueprint_Parameter_Array[count].description);
            Copy_String_CharS(it->type , &c_parameter.Blueprint_Parameter_Array[count].type);

            //c_parameter.Blueprint_Parameter_Array[count].use = it->use;

            Copy_String_CharS(it->defaultValue , &c_parameter.Blueprint_Parameter_Array[count].defaultValue);
            Copy_String_CharS(it->exampleValue , &c_parameter.Blueprint_Parameter_Array[count].exampleValue);

            unsigned int count_l2 = 0;
            c_parameter.Blueprint_Parameter_Array[count].values.size = it->values.size();
            if (c_parameter.Blueprint_Parameter_Array[count].values.size){
                for (snowcrash::Collection<snowcrash::Value>::const_iterator it_l2 = it->values.begin(); it_l2 != it->values.end(); ++it_l2){
                    Copy_String_CharS(it_l2->c_str() , &c_parameter.Blueprint_Parameter_Array[count].values.Blueprint_Value_Array[count_l2]);
                    count_l2++;
                }
            } 
            else{
                c_parameter.Blueprint_Parameter_Array[count].values.Blueprint_Value_Array = NULL;
            }

            count++;
        }
    } 
    else{
        c_parameter.Blueprint_Parameter_Array = NULL;
    }
}

void cast_payload(snowcrash::Payload payload ,C_BluePrint_Payload& c_payload)
{
    Copy_String_CharS(payload.name , &c_payload.name);
    Copy_String_CharS(payload.description , &c_payload.description);
    Copy_String_CharS(payload.body , &c_payload.body);
    Copy_String_CharS(payload.schema , &c_payload.schema);

    cast_header(payload.headers , c_payload.headers);

    cast_parameters(payload.parameters , c_payload.parameters);
    
}

void cast_requests(snowcrash::Collection<snowcrash::Request>::type request,C_BluePrint_Request_Collection& c_request)
{
    unsigned int count = 0;

    c_request.size = request.size();
    if (c_request.size){
        c_request.Blueprint_Request_Array = new C_BluePrint_Request[c_request.size];
        for (snowcrash::Collection<snowcrash::Request>::const_iterator it = request.begin(); it != request.end(); ++it){
            cast_payload(*it , c_request.Blueprint_Request_Array[count]);
            count++;
        }
    } 
    else{
        c_request.Blueprint_Request_Array = NULL;
    }
}

void cast_responses(snowcrash::Collection<snowcrash::Response>::type response,C_BluePrint_Response_Collection& c_response)
{
    unsigned int count = 0;

    c_response.size = response.size();
    if (c_response.size){
        c_response.Blueprint_Response_Array = new C_BluePrint_Request[c_response.size];
        for (snowcrash::Collection<snowcrash::Response>::const_iterator it = response.begin(); it != response.end(); ++it){
            cast_payload(*it , c_response.Blueprint_Response_Array[count]);
            count++;
        }
    } 
    else{
        c_response.Blueprint_Response_Array = NULL;
    }
}


void cast_resource_example(snowcrash::Collection<snowcrash::TransactionExample>::type example, C_BluePrint_TransactionExample_Collection& c_example)
{
    unsigned int count = 0;

    c_example.size = example.size();
    if (c_example.size){
        c_example.Blueprint_TransactionExample_Array = new C_BluePrint_TransactionExample[c_example.size];
        for (snowcrash::Collection<snowcrash::TransactionExample>::const_iterator it = example.begin(); it != example.end(); ++it){
            Copy_String_CharS(it->name , &c_example.Blueprint_TransactionExample_Array[count].name);
            Copy_String_CharS(it->description , &c_example.Blueprint_TransactionExample_Array[count].description);

            cast_requests(it->requests , c_example.Blueprint_TransactionExample_Array[count].requests);
            cast_responses(it->responses , c_example.Blueprint_TransactionExample_Array[count].responses);

            count++;
        }
    } 
    else{
    }
}

void cast_resource_action(snowcrash::Collection<snowcrash::Action>::type action, C_BluePrint_Action_Collection& c_action)
{
    unsigned int count = 0;

    c_action.size = action.size();
    if (c_action.size){
        c_action.Blueprint_Action_Array  = new C_BluePrint_Action[c_action.size];
        for (snowcrash::Collection<snowcrash::Action>::const_iterator it = action.begin(); it != action.end(); ++it){
            Copy_String_CharS(it->method , &c_action.Blueprint_Action_Array[count].method);
            Copy_String_CharS(it->name , &c_action.Blueprint_Action_Array[count].name);
            Copy_String_CharS(it->description , &c_action.Blueprint_Action_Array[count].description);

            cast_parameters(it->parameters , c_action.Blueprint_Action_Array[count].parameters);
            cast_header(it->headers , c_action.Blueprint_Action_Array[count].headers);
            cast_resource_example(it->examples ,c_action.Blueprint_Action_Array[count].examples);

            count++;            
        }
    } 
    else{
        c_action.Blueprint_Action_Array = NULL;
    }
}
void cast_resource_group(snowcrash::Collection<snowcrash::ResourceGroup>::const_iterator it, C_BluePrint_Resource** c_resource)
{
    *c_resource = new C_BluePrint_Resource[it->resources.size()];

    C_BluePrint_Resource* t_resource = *c_resource;

    unsigned int count = 0;
    for (snowcrash::Collection<snowcrash::Resource>::const_iterator it_l2 = it->resources.begin(); it_l2 != it->resources.end(); ++it_l2){
        Copy_String_CharS(it_l2->uriTemplate , &t_resource[count].uriTemplate);
        Copy_String_CharS(it_l2->name , &t_resource[count].name);
        Copy_String_CharS(it_l2->description , &t_resource[count].description);

        cast_payload(it_l2->model , t_resource->model);

        cast_parameters(it_l2->parameters , t_resource->parameters);
        cast_header(it_l2->headers , t_resource->headers);

        cast_resource_action(it_l2->actions ,t_resource->actions);

        count++;
    }
}

C_Blueprint i_blueprint;

C_Blueprint* C_parse(const char* i_source, C_BlueprintParserOptions i_options)
{
    C_Result i_result;
    snowcrash::BlueprintParserOptions options = i_options;
    snowcrash::Result result;
    snowcrash::Blueprint blueprint;
    const snowcrash::SourceData blueprintSource = i_source;
    int count =0;

    snowcrash::Parser p;
    p.parse(blueprintSource, options, result, blueprint);

    i_blueprint.name = (C_BluePrint_Name)blueprint.name.c_str();
    
    ////////////////////////////Blueprint-casting/////////////////////////////
    count = 0;

    i_blueprint.metadata.size = blueprint.metadata.size();
    if (i_blueprint.metadata.size)
    {
        i_blueprint.metadata.Blueprint_Metadata_Array = (C_BluePrint_Metadata*)malloc( sizeof(C_BluePrint_Metadata) * i_blueprint.metadata.size );
        for (snowcrash::Collection<snowcrash::Metadata>::const_iterator it = blueprint.metadata.begin(); it != blueprint.metadata.end(); ++it){
            Copy_String_CharS(it->first , &i_blueprint.metadata.Blueprint_Metadata_Array[count].key);
            Copy_String_CharS(it->second , &i_blueprint.metadata.Blueprint_Metadata_Array[count].value);

            count++;
        }
    } 
    else
    {
        i_blueprint.metadata.Blueprint_Metadata_Array = NULL;
    }

    Copy_String_CharS(blueprint.name , &i_blueprint.name);
    Copy_String_CharS(blueprint.description , &i_blueprint.description);

    count = 0;
    i_blueprint.resourceGroups.size = blueprint.resourceGroups.size();
    if (i_blueprint.resourceGroups.size)
    {
        i_blueprint.resourceGroups.Blueprint_ResourceGroup_Array = new C_BluePrint_ResourceGroup[i_blueprint.resourceGroups.size];
        for (snowcrash::Collection<snowcrash::ResourceGroup>::const_iterator it = blueprint.resourceGroups.begin(); it != blueprint.resourceGroups.end(); ++it){

            Copy_String_CharS(it->name , &i_blueprint.resourceGroups.Blueprint_ResourceGroup_Array[count].name);
            Copy_String_CharS(it->description , &i_blueprint.resourceGroups.Blueprint_ResourceGroup_Array[count].description);

            i_blueprint.resourceGroups.Blueprint_ResourceGroup_Array[count].resources.size = it->resources.size();
            if (i_blueprint.resourceGroups.Blueprint_ResourceGroup_Array[count].resources.size){
                cast_resource_group(it , &i_blueprint.resourceGroups.Blueprint_ResourceGroup_Array[count].resources.Blueprint_Resource_Array);
            }
            else{
                i_blueprint.resourceGroups.Blueprint_ResourceGroup_Array[count].resources.Blueprint_Resource_Array = NULL;
            }

            count++;
        }
    } 
    else
    {
        i_blueprint.resourceGroups.Blueprint_ResourceGroup_Array = NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    ////////////////////////////result-casting////////////////////////////////
    count = 0;

    i_result.warnings.size = result.warnings.size();
    if(i_result.warnings.size){
        i_result.warnings.warnings_array = new C_Warning[result.warnings.size()];
        for (std::vector<snowcrash::Warning>::const_iterator it = result.warnings.begin(); it != result.warnings.end(); ++it){
            i_result.warnings.warnings_array[count].code = it->code;
            i_result.warnings.warnings_array[count].location.size = it->location.size();
            i_result.warnings.warnings_array[count].location.SourceCharactersRange_array = new C_SourceCharactersRange[it->location.size()];
            int count_l2 = 0;
            for (std::vector<snowcrash::SourceCharactersRange>::const_iterator itsc = it->location.begin(); itsc != it->location.end(); ++itsc){
                i_result.warnings.warnings_array[count].location.SourceCharactersRange_array[count_l2].length = itsc->length;
                i_result.warnings.warnings_array[count].location.SourceCharactersRange_array[count_l2].location = itsc->location;
                count_l2++;
            }
            if(it->message.size()){
                Copy_String_CharS(it->message , &i_result.warnings.warnings_array[count].message);
            }
            else{
                i_result.warnings.warnings_array[count].message = NULL;
            }
            count ++;
        }
    }
    else{
        i_result.warnings.warnings_array = NULL;
    }

    i_result.error.code = result.error.code;

    i_result.error.location.size = result.error.location.size();
    if (i_result.error.location.size){
        i_result.error.location.SourceCharactersRange_array = new C_SourceCharactersRange[i_result.error.location.size];
        count = 0;
        for (std::vector<snowcrash::SourceCharactersRange>::const_iterator itsc = result.error.location.begin(); itsc != result.error.location.end(); ++itsc){
            i_result.error.location.SourceCharactersRange_array[count].length = itsc->length;
            i_result.error.location.SourceCharactersRange_array[count].location = itsc->location;
            count++;
        }
    } 
    else{
        i_result.error.location.SourceCharactersRange_array = NULL;
    }
    
    Copy_String_CharS(result.error.message , &i_result.error.message);

    //////////////////////////////////////////////////////////////////////////

    return &i_blueprint;
};