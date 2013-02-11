/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'trans_func.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ert/util/arg_pack.h> 
#include <ert/util/util.h>
#include <ert/util/stringlist.h>

#include <ert/enkf/trans_func.h>






struct trans_func_struct {
  char            * name;          /* The name this function is registered as. */
  arg_pack_type   * params;        /* The parameter values registered for this function. */
  transform_ftype * func;          /* A pointer to the actual transformation function. */
  validate_ftype  * validate;      /* A pointer to a a function which can be used to validate the parameters - can be NULL. */
  stringlist_type * param_names;   /* A list of the parameter names. */
};






/**
   Width  = 1 => uniform
   Width  > 1 => unimodal peaked
   Width  < 1 => bimoal peaks


   Skewness < 0 => shifts towards the left
   Skewness = 0 => symmetric
   Skewness > 0 => Shifts towards the right

   The width is a relavant scale for the value of skewness.
*/

static double trans_errf(double x, const arg_pack_type * arg) { 
  double min      = arg_pack_iget_double(arg , 0);
  double max      = arg_pack_iget_double(arg , 1);
  double skewness = arg_pack_iget_double(arg , 2);
  double width    = arg_pack_iget_double(arg , 3);
  double y;

  y = 0.5*(1 + erf((x + skewness)/(width * sqrt(2.0))));
  return min + y * (max - min);
}


static void trans_errf_check(const char * func_name , const arg_pack_type * arg) {
  double width    = arg_pack_iget_double(arg , 3);
  if (width <= 0)
    util_exit("In function:%s the witdh must be > 0.",func_name);
}





static double trans_const(double x , const arg_pack_type * arg) { 
  return arg_pack_iget_double(arg , 0); 
}


/* Observe that the argument of the shift should be "+" */
static double trans_derrf(double x , const arg_pack_type * arg) {
  int    steps    = arg_pack_iget_int(arg , 0);
  double min      = arg_pack_iget_double(arg , 1);
  double max      = arg_pack_iget_double(arg , 2);
  double skewness = arg_pack_iget_double(arg , 3);
  double width    = arg_pack_iget_double(arg , 4);
  double y;
  
  y = floor( steps * 0.5*(1 + erf((x + skewness)/(width * sqrt(2.0)))) / (steps - 1) );
  return min + y * (max - min);
}


static void trans_derrf_check(const char * func_name , const arg_pack_type * arg) {
  int    steps    = arg_pack_iget_int(arg , 0);
  double width    = arg_pack_iget_double(arg , 4);
  if (width <= 0)
    util_exit("In function:%s the witdh must be > 0.",func_name);

  if (steps <= 1)
    util_exit("In function:%s the number of steps must be greater than 1.",func_name);
}





static double trans_unif(double x , const arg_pack_type * arg) {
  double y;
  double min   = arg_pack_iget_double(arg , 0);
  double max   = arg_pack_iget_double(arg , 1);
  y = 0.5*(1 + erf(x/sqrt(2.0))); /* 0 - 1 */
  return y * (max - min) + min;
}



static double trans_dunif(double x , const arg_pack_type * arg) {
  double y;
  int    steps = arg_pack_iget_int(arg , 0);
  double min   = arg_pack_iget_double(arg , 1);
  double max   = arg_pack_iget_double(arg , 2);
  
  y = 0.5*(1 + erf(x/sqrt(2.0))); /* 0 - 1 */
  return (floor( y * steps) / (steps - 1)) * (max - min) + min;
}


static void trans_dunif_check(const char * func_name , const arg_pack_type * arg) {
  int    steps = arg_pack_iget_int(arg , 0);
  
  if (steps <= 1)
    util_exit("When using function:%s steps must be > 1 \n",func_name);
}



static double trans_normal(double x , const arg_pack_type * arg) {
  double mu , std;
  mu  = arg_pack_iget_double(arg , 0 );
  std = arg_pack_iget_double(arg , 1 );
  return x * std + mu;
}



static double trans_lognormal(double x, const arg_pack_type * arg) {
  double mu, std;
  mu  = arg_pack_iget_double(arg , 0 );   /* The expectation of log( y ) */
  std = arg_pack_iget_double(arg , 1 );   
  return exp(x * std + mu);
}



/**
   Used to sample values between min and max - BUT it is the logarithm
   of y which is uniformly distributed. Relates to the uniform
   distribution in the same manner as the lognormal distribution
   relates to the normal distribution.
*/
static double trans_logunif(double x , const arg_pack_type * arg) {
  double log_min = log(arg_pack_iget_double(arg , 0));
  double log_max = log(arg_pack_iget_double(arg , 1));
  double log_y;
  {
    double tmp = 0.5*(1 + erf(x/sqrt(2.0)));           /* 0 - 1 */
    log_y      = log_min + tmp * (log_max - log_min);  /* Shift according to max / min */
  } 
  return exp(log_y);
}


static void trans_logunif_check(const char * func_name , const arg_pack_type * arg) {

  double min = arg_pack_iget_double(arg , 0);
  double max = arg_pack_iget_double(arg , 1);
  if (min >= max )
    util_exit("When using:%s frist argument must be less than second.\n",func_name);
  if(min <= 0)  
    util_exit("When using:%s both arguments must be greater than zero.\n",func_name);
}

/*****************************************************************/

static trans_func_type * trans_func_alloc_empty( const char * func_name ) {
  trans_func_type * trans_func = util_malloc( sizeof * trans_func );
  

  trans_func->params      = arg_pack_alloc();
  trans_func->func        = NULL;
  trans_func->validate    = NULL;
  trans_func->name        = util_alloc_string_copy( func_name );
  trans_func->param_names = stringlist_alloc_new();
  
  return trans_func;
}


const char * trans_func_get_name( const trans_func_type * trans_func ) {
  return trans_func->name;
}


const stringlist_type * trans_func_get_param_names( const trans_func_type * trans_func ) {
  return trans_func->param_names;
}

node_ctype trans_func_iget_param_ctype( const trans_func_type * trans_func , int param_index) {
  return arg_pack_iget_ctype( trans_func->params , param_index);
}


void trans_func_iset_double_param(trans_func_type  * trans_func , int param_index , double value ) {
  if (arg_pack_iget_ctype( trans_func->params , param_index) == CTYPE_DOUBLE_VALUE)
    arg_pack_iset_double( trans_func->params , param_index , value );
  else
    util_abort("%s: type mismatch - the does not expect double as argument:%d \n",__func__ , param_index );
}

/**
   Return true if the _set operation suceeded (i.e. the name was
   recognized), and false otherwise.
*/
bool trans_func_set_double_param( trans_func_type  * trans_func , const char * param_name , double value ) {
  int param_index = stringlist_find_first( trans_func->param_names , param_name);
  if (param_index >= 0) {
    arg_pack_iset_double( trans_func->params , param_index , value );
    return true;
  } else
    return false;
}


void trans_func_iset_int_param(trans_func_type  * trans_func , int param_index , int value ) {
  if (arg_pack_iget_ctype( trans_func->params , param_index) == CTYPE_INT_VALUE)
    arg_pack_iset_int( trans_func->params , param_index , value );
  else
    util_abort("%s: type mismatch - the does not expect int as argument:%d \n",__func__ , param_index );
}

/**
   Return true if the _set operation suceeded (i.e. the name was
   recognized), and false otherwise.
*/
bool trans_func_set_int_param( trans_func_type  * trans_func , const char * param_name , int value ) {
  int param_index = stringlist_find_first( trans_func->param_names , param_name);
  if (param_index >= 0) {
    arg_pack_iset_int( trans_func->params , param_index , value );
    return true;
  } else
    return false;
}



void trans_func_free( trans_func_type * trans_func ) {
  stringlist_free( trans_func->param_names );
  arg_pack_free( trans_func->params );
  util_safe_free( trans_func->name );
  free( trans_func );
}




/**
   It is import to append all the parameters (with arbitrary values),
   to ensure that the arg_pack registers the right type.
*/


trans_func_type * trans_func_alloc( const char * func_name ) {
  trans_func_type * trans_func = trans_func_alloc_empty( func_name );
  {
    if (util_string_equal(func_name , "NORMAL")) {
      stringlist_append_ref( trans_func->param_names , "MEAN");
      stringlist_append_ref( trans_func->param_names , "STD" );
      arg_pack_append_double( trans_func->params , 0 );
      arg_pack_append_double( trans_func->params , 0 );
      trans_func->func = trans_normal;
    }  
    
    if (util_string_equal( func_name , "LOGNORMAL")) {
      stringlist_append_ref( trans_func->param_names , "MEAN");
      stringlist_append_ref( trans_func->param_names , "STD" );
      arg_pack_append_double( trans_func->params , 0 );
      arg_pack_append_double( trans_func->params , 0 );
      trans_func->func = trans_lognormal;
    }
    
    if (util_string_equal( func_name , "UNIFORM")) {
      stringlist_append_ref( trans_func->param_names , "MIN");
      stringlist_append_ref( trans_func->param_names , "MAX" );
      arg_pack_append_double( trans_func->params , 0 );
      arg_pack_append_double( trans_func->params , 0 );
      trans_func->func = trans_unif;
    }


    if (util_string_equal( func_name , "DUNIF")) {
      stringlist_append_ref( trans_func->param_names , "STEPS");
      stringlist_append_ref( trans_func->param_names , "MIN");
      stringlist_append_ref( trans_func->param_names , "MAX" );
      arg_pack_append_int( trans_func->params , 0 );
      arg_pack_append_double( trans_func->params , 0 );
      arg_pack_append_double( trans_func->params , 0 );
      
      trans_func->func = trans_dunif;
    }


    if (util_string_equal( func_name , "ERRF")) {
      stringlist_append_ref( trans_func->param_names , "MIN");
      stringlist_append_ref( trans_func->param_names , "MAX" );
      stringlist_append_ref( trans_func->param_names , "SKEWNESS");
      stringlist_append_ref( trans_func->param_names , "WIDTH" );
      arg_pack_append_double( trans_func->params , 0 );
      arg_pack_append_double( trans_func->params , 0 );
      arg_pack_append_double( trans_func->params , 0 );
      arg_pack_append_double( trans_func->params , 0 );

      trans_func->func = trans_errf;
    }
    

    if (util_string_equal( func_name , "DERRF")) {
      stringlist_append_ref( trans_func->param_names , "STEPS");
      stringlist_append_ref( trans_func->param_names , "MIN");
      stringlist_append_ref( trans_func->param_names , "MAX" );
      stringlist_append_ref( trans_func->param_names , "SKEWNESS");
      stringlist_append_ref( trans_func->param_names , "WIDTH" );
      arg_pack_append_int( trans_func->params , 0 );
      arg_pack_append_double( trans_func->params , 0 );
      arg_pack_append_double( trans_func->params , 0 );
      arg_pack_append_double( trans_func->params , 0 );
      arg_pack_append_double( trans_func->params , 0 );

      trans_func->func = trans_derrf;
    }


    if (util_string_equal( func_name , "LOGUNIF")) {
      stringlist_append_ref( trans_func->param_names , "MIN");
      stringlist_append_ref( trans_func->param_names , "MAX" );
      
      arg_pack_append_double( trans_func->params , 0 );
      arg_pack_append_double( trans_func->params , 0 );
      trans_func->func = trans_logunif;
    }


    if (util_string_equal( func_name , "CONST")) {
      stringlist_append_ref( trans_func->param_names , "VALUE");
      arg_pack_append_double( trans_func->params , 0 );
      trans_func->func = trans_const;
    }

    if (trans_func->func == NULL) 
      util_exit("%s: Sorry: function name:%s not recognized \n",__func__ , func_name);
  }
  return trans_func;
}



double trans_func_eval( const trans_func_type * trans_func , double x) {
  double y = trans_func->func( x , trans_func->params );
  return y;
}





trans_func_type * trans_func_fscanf_alloc( FILE * stream ) {
  trans_func_type * trans_func;
  char            * func_name;

  func_name = util_fscanf_alloc_token(stream);
  if (func_name == NULL) {
    char * filename = "????";
#ifdef HAVE_FORK
      filename = util_alloc_filename_from_stream( stream );
#endif
    fprintf(stderr,"Problem at file:line: %s:%d \n", filename, util_get_current_linenr( stream ));
    util_abort("%s: could not locate name of transformation - aborting \n",__func__);
  }
  
  trans_func = trans_func_alloc( func_name );
  arg_pack_fscanf( trans_func->params , stream );
  
  free( func_name );
  return trans_func;
}

