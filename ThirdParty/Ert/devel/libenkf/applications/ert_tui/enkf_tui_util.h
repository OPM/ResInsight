/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_tui_util.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ENKF_TUI_UTIL_H__
#define __ENKF_TUI_UTIL_H__


#include <ert/enkf/enkf_types.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_config_node.h>
#include <ert/enkf/ensemble_config.h>


void                          enkf_tui_util_scanf_report_steps(int  , int  , int *  , int * );
const enkf_config_node_type * enkf_tui_util_scanf_key(const ensemble_config_type *  , int , ert_impl_type ,  enkf_var_type);
state_enum                    enkf_tui_util_scanf_state(const char * , int , bool);
int                           enkf_tui_util_scanf_ijk(const field_config_type * , int);
void                          enkf_tui_util_scanf_ijk__(const field_config_type * , int  , int * , int * , int *);
bool                        * enkf_tui_util_scanf_alloc_report_active(int , int );
bool                        * enkf_tui_util_scanf_alloc_iens_active(int , int , int * , int *);
void                          enkf_tui_util_get_time(enkf_fs_type * , const enkf_config_node_type * , enkf_node_type * , state_enum , int  , int  , int  , int  , double *  , double *  );
void                          enkf_tui_util_scanf_iens_range(const char * , int  , int  , int *  , int * );
int                           enkf_tui_util_scanf_report_step(int , const char *  , int );
char *                        enkf_tui_util_scanf_report_step_as_char(int , const char *  , int );
void                          enkf_tui_util_msg(const char * , ...);
int                           enkf_tui_util_scanf_int_with_default(const char * prompt , int prompt_len , bool * default_used);
int                           enkf_tui_util_scanf_int_with_default_return_to_menu(const char * prompt , int prompt_len , bool * default_used);
#endif
