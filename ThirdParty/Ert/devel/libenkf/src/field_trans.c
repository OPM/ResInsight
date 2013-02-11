/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'field_trans.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

/*
  This file implements a number of functions used for init and output
  transformations of fields. The prototype for these functions is very
  simple: "one float in - one float out". 

  It is mainly implemented in this file, so that it will be easy to
  adde new transformation functions without diving into the the full
  field / field_config complexity.

  Documentation on how to add a new transformation function is at the
  bottom of the file.
*/
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <ert/util/hash.h>
#include <ert/util/util.h>

#include <ert/enkf/field_trans.h>
/*****************************************************************/

struct field_trans_table_struct {
  bool        case_sensitive;
  hash_type * function_table;
};



typedef struct {
  char            * key;
  char            * description;
  field_func_type * func;
} field_func_node_type;

/*****************************************************************/

static field_func_node_type * field_func_node_alloc(const char * key , const char * description , field_func_type * func) {
  field_func_node_type * node = util_malloc( sizeof * node );

  node->key         = util_alloc_string_copy( key );
  node->description = util_alloc_string_copy( description );
  node->func        = func;
  
  return node;
}


static void field_func_node_free(field_func_node_type * node) {
  free(node->key);
  util_safe_free( node->description );
  free(node);
}


static void field_func_node_free__(void * node) {
  field_func_node_free( (field_func_node_type *) node);
}


static void field_func_node_fprintf(const field_func_node_type * node , FILE * stream) {
  if (node->description != NULL)
    fprintf(stream , "%16s: %s \n",node->key , node->description);
  else
    fprintf(stream , "%16s: No description \n",node->key  );
}






/*****************************************************************/

void field_trans_table_add(field_trans_table_type * table , const char * _key , const char * description , field_func_type * func) {
  char * key;
  
  if (table->case_sensitive)
    key = util_alloc_string_copy( _key );
  else
    key = util_alloc_strupr_copy( _key );

  {
    field_func_node_type * node = field_func_node_alloc( key , description , func );
    hash_insert_hash_owned_ref(table->function_table , key , node , field_func_node_free__);
  }
  free(key);
}


void field_trans_table_fprintf(const field_trans_table_type * table , FILE * stream) {
  hash_iter_type * iter = hash_iter_alloc(table->function_table);
  const char * key = hash_iter_get_next_key(iter); 
  fprintf(stream,"==========================================================================================\n");
  while (key != NULL) {
    field_func_node_type * func_node = hash_get(table->function_table , key);
    field_func_node_fprintf(func_node , stream);
    key = hash_iter_get_next_key(iter);
  }
  fprintf(stream,"==========================================================================================\n");
  hash_iter_free(iter);
}



/*
  This function takes a key input, and returns a pointer to the
  corresponding function. The function will fail if the key is not
  recognized.
*/


field_func_type * field_trans_table_lookup(field_trans_table_type * table , const char * _key) {
  field_func_type * func;
  char * key;

  if (table->case_sensitive)
    key = util_alloc_string_copy(_key);
  else
    key = util_alloc_strupr_copy(_key);

  if (hash_has_key(table->function_table , key)) {
    field_func_node_type * func_node = hash_get(table->function_table , key);
    func = func_node->func;
  } else {
    fprintf(stderr , "Sorry: the field transformation function:%s is not recognized \n\n",key);
    field_trans_table_fprintf(table , stderr);
    util_exit("Exiting ... \n");
    func = NULL; /* Compiler shut up. */
  }
  free( key );
  return func;
}


/**
   Will return false if _key == NULL 
*/
bool field_trans_table_has_key(field_trans_table_type * table , const char * _key) {
  bool has_key = false;

  if (_key != NULL) {
    char * key;
    if (table->case_sensitive)
      key = util_alloc_string_copy(_key);
    else
      key = util_alloc_strupr_copy(_key);
    
    has_key = hash_has_key( table->function_table , key);
    free(key);
  }
  
  return has_key;
}


void field_trans_table_free(field_trans_table_type * table ) {
  hash_free( table->function_table );
  free( table );
}




/*****************************************************************/
/*****************************************************************/
/* Here comes the actual functions. To add a new function:       */
/*                                                               */
/*  1. Write the function - as a float in - float out.           */
/*  2. Register the function in field_trans_table_alloc().       */
/*                                                               */ 
/*****************************************************************/

/*****************************************************************/
/* Rubakumar specials: start  */
#define PERMX_MEAN 100
#define PERMX_STD    1

#define PERMZ_MEAN 100
#define PERMZ_STD    1

#define PORO_MEAN  100
#define PORO_STD     1


static float normalize(float x , float mean , float std) {
  return (x - mean) / std;
}

static float denormalize(float x , float mean , float std) {
  return (x * std) + mean;
}

static float normalize_permx(float x) {
  return normalize(x , PERMX_MEAN , PERMX_STD);
}

static float denormalize_permx(float x) {
  return denormalize(x , PERMX_MEAN , PERMX_STD);
}

static float normalize_permz(float x) {
  return normalize(x , PERMZ_MEAN , PERMZ_STD);
}

static float denormalize_permz(float x) {
  return denormalize(x , PERMZ_MEAN , PERMZ_STD);
}

static float normalize_poro(float x) {
  return normalize(x , PORO_MEAN , PORO_STD);
}

static float denormalize_poro(float x) {
  return denormalize(x , PORO_MEAN , PORO_STD);
}

/* Rubakumar specials: end  */
/*****************************************************************/


static float field_trans_pow10(float x) {
  return powf(10.0 , x);
}


static float trunc_pow10f(float x) {
  return util_float_max(powf(10.0 , x) , 0.001);
}

#define LN_SHIFT 0.0000001
static float field_trans_ln0( float x ) {
  return logf( x + LN_SHIFT );
}

static float field_trans_exp0( float x ) {
  return expf( x ) - LN_SHIFT;
}
#undef LN_SHIFT



field_trans_table_type * field_trans_table_alloc() {
  field_trans_table_type * table = util_malloc( sizeof * table);
  table->function_table = hash_alloc();
  field_trans_table_add( table , "POW10"       , "This function will raise x to the power of 10: y = 10^x." ,                            field_trans_pow10);
  field_trans_table_add( table , "TRUNC_POW10" , "This function will raise x to the power of 10 - and truncate lower values at 0.001." , trunc_pow10f);
  field_trans_table_add( table , "LOG"         , "This function will take the NATURAL logarithm of x: y = ln(x)" , logf);
  field_trans_table_add( table , "LN"          , "This function will take the NATURAL logarithm of x: y = ln(x)" , logf);
  field_trans_table_add( table , "LOG10"       , "This function will take the log10 logarithm of x: y = log10(x)" , log10f);
  field_trans_table_add( table , "EXP"         , "This function will calculate y = exp(x) " , expf);
  field_trans_table_add( table , "LN0"         , "This function will calculate y = ln(x + 0.000001)" , field_trans_ln0);
  field_trans_table_add( table , "EXP0"        , "This function will calculate y = exp(x) - 0.000001" , field_trans_exp0);
  
  //-----------------------------------------------------------------
  // Rubakumar specials:
  field_trans_table_add( table , "NORMALIZE_PERMX"    , "..." , normalize_permx);
  field_trans_table_add( table , "DENORMALIZE_PERMX"  , "..." , denormalize_permx);

  field_trans_table_add( table , "NORMALIZE_PERMZ"    , "..." , normalize_permz);
  field_trans_table_add( table , "DENORMALIZE_PERMZ"  , "..." , denormalize_permz);

  field_trans_table_add( table , "NORMALIZE_PORO"    , "..." , normalize_poro);
  field_trans_table_add( table , "DENORMALIZE_PORO"  , "..." , denormalize_poro);
  //-----------------------------------------------------------------

  table->case_sensitive = false;
  return table;
}

