/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_tui_util.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <ert/util/util.h>
#include <ert/util/string_util.h>
#include <ert/util/menu.h>
#include <ert/util/arg_pack.h>

#include <ert/enkf/enkf_node.h>
#include <ert/enkf/field.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/enkf_state.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/enkf_types.h>


/** 
    This file implements various small utility functions for the (text
    based) EnKF user interface.
*/



/**
   This functions displays the user with a prompt, and reads 'A' or
   'F' (lowercase OK), to check whether the user is interested in the
   forecast or the analyzed state.
*/


state_enum enkf_tui_util_scanf_state(const char * prompt, int prompt_len, bool accept_both) {
  char analyzed_string[64];
  bool OK;
  state_enum state;
  do {
    OK = true;
    util_printf_prompt(prompt , prompt_len , '=' , "=> ");
    //scanf("%s" , analyzed_string);
    fgets(analyzed_string, prompt_len, stdin);
    char *newline = strchr(analyzed_string,'\n');
    if(newline)
      *newline = 0;
    //getchar(); /* Discards trailing <RETURN> from standard input buffer? */
    if (strlen(analyzed_string) == 0){
      OK = true;
      state = UNDEFINED;
    }
    else if (strlen(analyzed_string) == 1) {
      char c = toupper(analyzed_string[0]);
      if (c == 'A')
        state = ANALYZED;
      else if (c == 'F')
        state = FORECAST;
      else {
        if (accept_both) {
          if (c == 'B') 
            state = BOTH;
          else
            OK = false;
        } else
          OK = false;
      }
    } else
      OK = false;
  } while ( !OK );
  return state;
}



/**
   Very simple function which is in interactive functions. Used to
   query the user:

     - Key identifying a field.
     - An integer report step.
     - Whether we are considering the analyzed state or the forecast.

   The config_node is returned, and in addition the report_step, iens
   and analysis_state are returned by reference. It is OK the pass in
   NULL for these pointers; in that case the user is not queried for
   these values.

   The keyword is checked for existence; but it is not checked whether
   the report_step actually exists. If impl_type == INVALID, any
   implementation type will be accepted, otherwise we loop until the
   keyword is of type impl_type.
*/

const enkf_config_node_type * enkf_tui_util_scanf_key(const ensemble_config_type * config , int prompt_len , ert_impl_type impl_type ,  enkf_var_type var_type) {
  char * kw;
  bool OK;
  const enkf_config_node_type * config_node = NULL;
  do {
    OK = true;
    util_printf_prompt("Keyword" , prompt_len , '=' , "=> ");
    kw = util_alloc_stdin_line();
    if(kw==NULL){
      OK = true;
  }
    else if (ensemble_config_has_key(config , kw)) {
      config_node = ensemble_config_get_node(config , kw);
      
      if (impl_type != INVALID) 
        if (enkf_config_node_get_impl_type(config_node) != impl_type) 
          OK = false;
      
      if (var_type != INVALID_VAR)
        if (enkf_config_node_get_var_type(config_node) != var_type) 
          OK = false;
    } else OK = false;
    free(kw);
  } while (!OK);
  return config_node;
}


/**
   Present the user with the queries:
   
      First ensemble member ==>
      Last ensemble member ===>
  
    It then allocates (bool *) pointer [0..ens_size-1], where the
    interval gven by the user is true (i.e. actve), and the rest is
    false. It s the responsiibility of the calling scope to free this.
*/


bool * enkf_tui_util_scanf_alloc_iens_active(int ens_size, int prompt_len , int * _iens1 , int * _iens2) {
  bool * iactive = util_calloc(ens_size , sizeof * iactive );
  int iens1 = util_scanf_int_with_limits("First ensemble member" , prompt_len , 0 , ens_size - 1);
  int iens2 = util_scanf_int_with_limits("Last ensemble member" , prompt_len , iens1 , ens_size - 1);
  int iens;

  for (iens = 0; iens < ens_size; iens++) 
    iactive[iens] = false;

  for (iens = iens1; iens <= iens2; iens++) 
    iactive[iens] = true;


  *_iens1 = iens1;
  *_iens2 = iens2;
  return iactive;
}



/**
   Presents the reader with a prompt, and reads a string containing
   two integers separated by a character(s) in the set: " ,-:". If the
   user enters a blank string that is interpreted as "all
   realizations", and the return variabels are set to:

      iens1 = 0     iens2 = ens_size - 1

   Will not return before the user has actually presented a valid
   string.
*/

  
void enkf_tui_util_scanf_iens_range(const char * prompt_fmt , int ens_size , int prompt_len , int * iens1 , int * iens2) {
  char * prompt = util_alloc_sprintf(prompt_fmt , ens_size - 1);
  bool OK = false;

  util_printf_prompt(prompt , prompt_len , '=' , "=> ");
  
  while (!OK) {
    char * input = util_alloc_stdin_line();
    const char * current_ptr = input;
    OK = true;

    if (input != NULL) {
      current_ptr = util_parse_int(current_ptr , iens1 , &OK);
      current_ptr = util_skip_sep(current_ptr , " ,-:" , &OK);
      current_ptr = util_parse_int(current_ptr , iens2 , &OK);
      
      if (!OK) 
        printf("Failed to parse two integers from: \"%s\". Example: \"0 - 19\" to get the 20 first members.\n",input);
      free(input);
    } else {
      *iens1 = 0;
      *iens2 = ens_size - 1;
    }
  }
  free(prompt);
}


void enkf_tui_util_scanf_report_steps(int last_report , int prompt_len , int * __step1 , int * __step2) {
  char * prompt = util_alloc_sprintf("Report steps (0 - %d)" , last_report);
  bool OK = false;

  util_printf_prompt(prompt , prompt_len , '=' , "=> ");
  
  while (!OK) {
    char * input = util_alloc_stdin_line();
    const char * current_ptr = input;
    int step1 , step2;
    OK = true;
    if(input == NULL){
      step1=0;
      step2=last_report;
    }
    else{
      current_ptr = util_parse_int(current_ptr , &step1 , &OK);
      current_ptr = util_skip_sep(current_ptr , " ,-:" , &OK);
      current_ptr = util_parse_int(current_ptr , &step2 , &OK);
    }
    if (!OK) 
      printf("Failed to parse two integers from: \"%s\". Example: \"0 - 19\" to get the 20 first report steps.\n",input);
    free(input);

    step1 = util_int_min(step1 , last_report);
    step2 = util_int_min(step2 , last_report);
    if (step1 > step2) 
      util_exit("%s: ohh come on - must have a finite interval forward in time - no plots for you.\n",__func__);
    *__step1 = step1;
    *__step2 = step2;
    
  }
  free(prompt);
}



/**
   Similar to enkf_tui_util_scanf_alloc_iens_active(), but based on report steps.
*/

bool * enkf_tui_util_scanf_alloc_report_active(int last_step, int prompt_len) {
  bool * iactive = util_calloc((last_step + 1) , sizeof * iactive );
  int step1 = util_scanf_int_with_limits("First report step" , prompt_len , 0 , last_step);
  int step2 = util_scanf_int_with_limits("Last report step" , prompt_len , step1 , last_step);
  int step;

  for (step = 0; step <= last_step; step++) 
    iactive[step] = false;

  for (step = step1; step <= step2; step++) 
    iactive[step] = true;

  return iactive;
}


/** 
    This functions reads i,j,k and returns them be reference; if the
    reference pointer is NULL, that coordinate is skipped. I.e.

    enkf_tui_util_scanf_ijk__(config , 100 , &i , &j , NULL);

    Will read i and j. If your are interested in all three coordinates
    you should use enkf_tui_util_scanf_ijk() which has a more flexible
    parser.
*/


void enkf_tui_util_scanf_ijk__(const field_config_type * config, int prompt_len , int *i , int *j , int *k) {
  int nx,ny,nz;

  field_config_get_dims(config , &nx , &ny , &nz);
  if (i != NULL) (*i) = util_scanf_int_with_limits("Give i-index" , prompt_len , 1 , nx) - 1;
  if (j != NULL) (*j) = util_scanf_int_with_limits("Give j-index" , prompt_len , 1 , ny) - 1;
  if (k != NULL) (*k) = util_scanf_int_with_limits("Give k-index" , prompt_len , 1 , nz) - 1;
}




/**
   The function reads ijk, but it returns a global 1D index. Observe
   that the user is supposed to enter an index starting at one - whichs
   is immediately shifted down to become zero based.

   The function will loop until the user has entered ijk corresponding
   to an active cell.
*/
   
int enkf_tui_util_scanf_ijk(const field_config_type * config, int prompt_len) {
  int global_index;
  field_config_scanf_ijk(config , true , "Give (i,j,k) indices" , prompt_len , NULL , NULL , NULL , &global_index);
  return global_index;
}









/**
   This function runs through all the report steps [step1:step2] for
   member iens, and gets the value of the cell 'get_index'. Current
   implementation assumes that the config_node/node combination are of
   field type - this should be generalized to use the enkf_node_iget()
   function.

   The value is returned (by reference) in y, and the corresponding
   time (currently report_step) is returned in 'x'.
*/
   

void enkf_tui_util_get_time(enkf_fs_type * fs , const enkf_config_node_type * config_node, enkf_node_type * node , state_enum analysis_state , int get_index , int step1 , int step2 , int iens , double * x , double * y ) {
  const char * key = enkf_config_node_get_key(config_node);
  int report_step;
  int index = 0;
  for (report_step = step1; report_step <= step2; report_step++) {
    
    if (analysis_state & FORECAST) {
      node_id_type node_id = {.report_step = report_step , .iens = iens , .state = FORECAST };
      if (enkf_node_try_load(node , fs , node_id)) {
        const field_type * field = enkf_node_value_ptr( node );
        y[index] = field_iget_double(field , get_index);
      } else {
        fprintf(stderr," ** Warning field:%s is missing for member,report: %d,%d \n",key  , iens , report_step);
        y[index] = -1;
      }
      x[index] = report_step;
      index++;
    }
    
    
    if (analysis_state & ANALYZED) {
      node_id_type node_id = {.report_step = report_step , .iens = iens , .state = ANALYZED };
      if (enkf_node_try_load(node , fs , node_id)) {
        const field_type * field = enkf_node_value_ptr( node );
        y[index] = field_iget_double(field , get_index);
      } else {
        fprintf(stderr," ** Warning field:%s is missing for member,report: %d,%d \n",key , iens , report_step);
        y[index] = -1;
      }
      x[index] = report_step;
      index++;
    }
  }
}


int enkf_tui_util_scanf_report_step(int last_report, const char * prompt , int prompt_len) {
  int report_step                         = util_scanf_int_with_limits(prompt , prompt_len , 0 , last_report);
  return report_step;
}

char *  enkf_tui_util_scanf_report_step_as_char(int last_report, const char * prompt , int prompt_len) {
  char * report_step = util_scanf_int_with_limits_return_char(prompt , prompt_len , 0 , last_report);
  return report_step;
}

int enkf_tui_util_scanf_int_with_default(const char * prompt , int prompt_len , bool * default_used) {
  bool        OK;
  int value;
  *default_used = false;
  do {
    char * input;

    util_printf_prompt(prompt , prompt_len , '=' , "=> ");
    input = util_alloc_stdin_line();
    if (input == NULL) {
      *default_used = true;
      OK = true;
      value = -1;
    } else {
      OK = util_sscanf_int( input , &value ); 
      free( input );
    }
  } while (!OK);
  return value;
}

int enkf_tui_util_scanf_int_with_default_return_to_menu(const char * prompt , int prompt_len , bool * default_used) {
  bool        OK;
  int value;
  *default_used = false;
  do {
    char * input;

    util_printf_prompt(prompt , prompt_len , '=' , "=> ");
    input = util_alloc_stdin_line();
    if (input == NULL) {
      *default_used = true;
      OK = true;
      value = -1;
    }
    else if (strcmp(input,"M")==0 || strcmp(input,"m")==0){
      OK = true;
      value = -2;
    }
    else {
      OK = util_sscanf_int( input , &value ); 
      free( input );
    }
  } while (!OK);
  return value;
}

bool enkf_tui_util_sscanf_active_list( bool_vector_type * iactive , const char * select_string , int ens_size ) {
  if (select_string == NULL) {
    bool_vector_set_default( iactive , true );
    bool_vector_iset( iactive , ens_size - 1 , true );
    return true;
  } else {
    bool OK;
    OK = string_util_init_active_mask( select_string , iactive );
    
    if (bool_vector_size( iactive ) < ens_size) 
      bool_vector_iset( iactive , ens_size - 1 , false );
    
    return OK;
  }
}


/*****************************************************************/


/* Minimum wrapping of vfprintf */
void enkf_tui_util_msg(const char * fmt , ...) {
  va_list ap;
  va_start(ap , fmt);
  vfprintf(stdout , fmt , ap);
  va_end(ap);
}


