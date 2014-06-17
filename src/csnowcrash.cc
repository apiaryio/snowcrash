//
//  CSnowcrash.c
//  snowcrash
//
//  Created by Ali Khoramshahi on 13/6/14.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//
#include "csnowcrash.h"
#include "Parser.h"

int C_parse(const C_SourceData& i_source, C_BlueprintParserOptions i_options, C_Result& i_result, C_Blueprint& i_blueprint)
{
    snowcrash::BlueprintParserOptions options = i_options;
    snowcrash::Result result;
    snowcrash::Blueprint blueprint;
    const snowcrash::SourceData blueprintSource = i_source;
    int count =0;

    snowcrash::Parser p;
    p.parse(blueprintSource, options, result, blueprint);

    i_blueprint.name = (C_BluePrint_Name)blueprint.name.c_str();
    
    //i_blueprint = *reinterpret_cast<C_Blueprint*>(&blueprint);
    //i_result = *reinterpret_cast<C_Result*>(&result);
    //////////////////////////////////////////////////////////////////////////

    i_result.warnings.size = result.warnings.size();

    count =0;
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
                i_result.warnings.warnings_array[count].message = new char[it->message.size() + 1];
                std::copy(it->message.begin(), it->message.end(), i_result.warnings.warnings_array[count].message);
                i_result.warnings.warnings_array[count].message[it->message.size()] = '\0';
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
    i_result.error.location = *reinterpret_cast<const C_SourceCharactersBlock*>(&result.error.location);
    if(result.error.message.size()){
        i_result.error.message = new char[result.error.message.size() + 1];
        std::copy(result.error.message.begin(), result.error.message.end(), i_result.error.message);
        i_result.error.message[result.error.message.size()] = '\0';
    }
    else{
        i_result.error.message = NULL;
    }

    C_Warning* temp = i_result.warnings.warnings_array;

    //////////////////////////////////////////////////////////////////////////
    return result.error.code;
};