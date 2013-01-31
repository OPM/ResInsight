/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_types.c' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more details. 
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ert/util/util.h>

#include <ert/enkf/enkf_types.h>


/*****************************************************************/


const char * enkf_types_get_var_name(enkf_var_type var_type) {
  switch(var_type) {
  case(INVALID):
    return "INVALID";
    break;
  case PARAMETER:
    return "PARAMETER";
    break;
  case STATIC_STATE:
    return "STATIC_STATE";
    break;
  case DYNAMIC_STATE:
    return "DYNAMIC_STATE";
    break;
  case DYNAMIC_RESULT:
    return "DYNAMIC_RESULT";
    break;
  default:
    util_abort("%s: internal error - unrecognized var type: %d - aborting \n",__func__ , var_type);
    return NULL;
  }
}



const char * enkf_types_get_impl_name(ert_impl_type impl_type) {
  switch(impl_type) {
  case(INVALID):
    return "INVALID";
    break;
  case STATIC:
    return "STATIC";
    break;
  case FIELD:
    return "FIELD";
    break;
  case GEN_KW:
    return "GEN_KW";
    break;
  case SUMMARY:
    return "SUMMARY";
    break;
  case GEN_DATA:
    return "GEN_DATA";
    break;
  default:
    util_abort("%s: internal error - unrecognized implementation type: %d - aborting \n",__func__ , impl_type);
    return NULL;
  }
}


#define if_strcmp(s) if (strcmp(impl_type_string , #s) == 0) impl_type = s
static ert_impl_type enkf_types_get_impl_type__(const char * impl_type_string) {
  ert_impl_type impl_type;
  if_strcmp(STATIC);
  else if_strcmp(SUMMARY);
  else if_strcmp(FIELD);
  else if_strcmp(GEN_KW);
  else if_strcmp(GEN_DATA);
  else impl_type = INVALID;
  return impl_type;
}
#undef if_strcmp


ert_impl_type enkf_types_get_impl_type(const char * __impl_type_string) {
  char * impl_type_string = util_alloc_string_copy(__impl_type_string);
  util_strupr(impl_type_string);  
  ert_impl_type impl_type = enkf_types_get_impl_type__(impl_type_string);
  if (impl_type == INVALID) 
    util_abort("%s: enkf_type: %s not recognized - aborting \n",__func__ , __impl_type_string);
  
  free(impl_type_string);
  return impl_type;
}


/*
  This will return INVALIID if given an invalid
  input string - not fail.
*/
  
ert_impl_type enkf_types_check_impl_type(const char * impl_type_string) {
  return enkf_types_get_impl_type__(impl_type_string);
}


/*****************************************************************/
/* 
   These two functions update the truncation variable to ensure that
   it applies truncate_min and truncate_max respectively. The somewhat
   involved implementation is to ensure that the functions can be
   called many times.
*/


void enkf_types_set_truncate_min(truncation_type * __trunc) {
  truncation_type trunc = *__trunc;

  if (!(trunc & TRUNCATE_MIN))
    trunc += TRUNCATE_MIN;

  *__trunc = trunc;
}


void enkf_types_set_truncate_max(truncation_type * __trunc) {
  truncation_type trunc = *__trunc;

  if (!(trunc & TRUNCATE_MAX))
    trunc += TRUNCATE_MAX;

  *__trunc = trunc;
}


/*****************************************************************/


const char * enkf_state_enum_iget( int index, int * value) {
  return util_enum_iget( index , ENKF_STATE_ENUM_SIZE , (const util_enum_element_type []) { ENKF_STATE_ENUM_DEFS }, value);
}


const char * enkf_run_enum_iget( int index, int * value) {
  return util_enum_iget( index , ENKF_RUN_ENUM_SIZE , (const util_enum_element_type []) { ENKF_RUN_ENUM_DEFS }, value);
}




