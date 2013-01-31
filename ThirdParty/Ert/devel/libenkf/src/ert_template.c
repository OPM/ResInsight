/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ert_template.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <stdlib.h>

#include <ert/util/template.h>
#include <ert/util/hash.h>
#include <ert/util/util.h>
#include <ert/util/subst_list.h>

#include <ert/enkf/ert_template.h>
#include <ert/enkf/config_keys.h>
#include <ert/enkf/enkf_defaults.h>


#define ERT_TEMPLATE_TYPE_ID  7731963
#define ERT_TEMPLATES_TYPE_ID 6677330

/* Singular - one template. */
struct ert_template_struct {
  UTIL_TYPE_ID_DECLARATION;
  template_type * template;
  char          * target_file;
};



/* Plural - many templates. */
struct ert_templates_struct {
  UTIL_TYPE_ID_DECLARATION;
  subst_list_type      * parent_subst; 
  hash_type            * templates;
};



void ert_template_set_target_file( ert_template_type * ert_template , const char * target_file ) {
  ert_template->target_file = util_realloc_string_copy( ert_template->target_file , target_file );
}


void ert_template_set_template_file( ert_template_type * ert_template , const char * template_file ) {
  template_set_template_file( ert_template->template , template_file );
}


const char * ert_template_get_template_file( const ert_template_type * ert_template) {
  return template_get_template_file( ert_template->template );
}

const char * ert_template_get_target_file( const ert_template_type * ert_template) {
  return ert_template->target_file;
}

const char * ert_template_get_args_as_string( const ert_template_type * ert_template ) {
  return template_get_args_as_string( ert_template->template );
}



ert_template_type * ert_template_alloc( const char * template_file , const char * target_file , subst_list_type * parent_subst) {
  ert_template_type * template = util_malloc( sizeof * template);
  UTIL_TYPE_ID_INIT(template , ERT_TEMPLATE_TYPE_ID);
  template->template    = template_alloc( template_file , false , parent_subst);  /* The templates are instantiated with internalize_template == false;
                                                                                     this means that substitutions are performed on the filename of the 
                                                                                     template itself .*/
  
  template->target_file = NULL;
  ert_template_set_target_file( template , target_file );
  return template;
}


void ert_template_free( ert_template_type * template ) {
  util_safe_free( template->target_file );
  template_free( template->template );
  free( template );
}


void ert_template_instantiate( ert_template_type * template , const char * path , const subst_list_type * arg_list) {
  char * target_file = util_alloc_filename( path , template->target_file , NULL );
  template_instantiate( template->template , target_file , arg_list , true );
  free( target_file );
}


void ert_template_add_arg( ert_template_type * template , const char * key , const char * value ) {
  template_add_arg( template->template , key , value );
}

void ert_template_set_args_from_string( ert_template_type * template, const char * arg_string ) {
  template_clear_args( template->template );
  template_add_args_from_string( template->template , arg_string );
}


UTIL_SAFE_CAST_FUNCTION( ert_template , ERT_TEMPLATE_TYPE_ID )
     
void ert_template_free__(void * arg) {
  ert_template_free( ert_template_safe_cast( arg ));
}


static void ert_template_fprintf_config( const ert_template_type * template , FILE * stream ) {
  fprintf(stream , CONFIG_KEY_FORMAT   , RUN_TEMPLATE_KEY );
  fprintf(stream , CONFIG_VALUE_FORMAT , ert_template_get_template_file( template ));
  fprintf(stream , CONFIG_VALUE_FORMAT , template->target_file );
  {
    const char * arg_string = ert_template_get_args_as_string( template );
    if (arg_string != NULL)
      fprintf(stream , CONFIG_ENDVALUE_FORMAT , arg_string );
    else
      fprintf(stream , "\n");
  }
}


/*****************************************************************/


ert_templates_type * ert_templates_alloc( subst_list_type * parent_subst  ) {
  ert_templates_type * templates = util_malloc( sizeof * templates );
  UTIL_TYPE_ID_INIT( templates , ERT_TEMPLATES_TYPE_ID );
  templates->templates       = hash_alloc();
  templates->parent_subst    = parent_subst; 
  return templates;
}


void ert_templates_free( ert_templates_type * ert_templates ) {
  hash_free( ert_templates->templates );
  free( ert_templates );
}


void ert_templates_del_template( ert_templates_type * ert_templates , const char * key) {
  hash_del( ert_templates->templates , key );
}


ert_template_type * ert_templates_add_template( ert_templates_type * ert_templates , const char * key , const char * template_file , const char * target_file, const char * arg_string) {
  ert_template_type * template = ert_template_alloc( template_file ,  target_file , ert_templates->parent_subst);
  ert_template_set_args_from_string( template , arg_string ); /* Arg_string can be NULL */
  
  /** 
      If key == NULL the function will generate a key after the following algorithm:

      1. It tries with the basename of the template file.
      2. It tries with the basename of the template file, and a counter.
  */
  
  if (key == NULL) {
    char * new_key = NULL;
    char * base_name;
    int    counter = 1;
    util_alloc_file_components( template_file , NULL , &base_name , NULL);
    do {
      if (counter == 1)
        new_key = util_realloc_string_copy( new_key , base_name );
      else
        new_key = util_realloc_sprintf( new_key , "%s.%d" , base_name , counter );
      counter++;
    } while (hash_has_key( ert_templates->templates , new_key));
    hash_insert_hash_owned_ref( ert_templates->templates , new_key , template , ert_template_free__);
    free( new_key );
    free( base_name );
  } else
    hash_insert_hash_owned_ref( ert_templates->templates , key , template , ert_template_free__);

  return template;
}


void ert_templates_instansiate( ert_templates_type * ert_templates , const char * path , const subst_list_type * arg_list) {
  hash_iter_type * iter = hash_iter_alloc( ert_templates->templates );
  while (!hash_iter_is_complete( iter )) {
    ert_template_type * ert_template = hash_iter_get_next_value( iter );
    ert_template_instantiate( ert_template , path , arg_list);
  }
  hash_iter_free( iter );
}



void ert_templates_clear( ert_templates_type * ert_templates ) {
  hash_clear( ert_templates->templates );
}

ert_template_type * ert_templates_get_template( ert_templates_type * ert_templates , const char * key) {
  return hash_get( ert_templates->templates , key );
}

stringlist_type * ert_templates_alloc_list( ert_templates_type * ert_templates) {
  return hash_alloc_stringlist( ert_templates->templates );
}



void ert_templates_fprintf_config( const ert_templates_type * ert_templates , FILE * stream ) {
  if (hash_get_size( ert_templates->templates ) > 0 ) {
    fprintf( stream , CONFIG_COMMENTLINE_FORMAT );
    fprintf( stream , CONFIG_COMMENT_FORMAT , "Here comes configuration information about RUN-TIME templates instantiated by ERT.");
    
    {
      hash_iter_type * iter = hash_iter_alloc( ert_templates->templates );
      while( !hash_iter_is_complete( iter )) {
        const char * key                   = hash_iter_get_next_key( iter );
        const ert_template_type * template = hash_get( ert_templates->templates , key );
        ert_template_fprintf_config( template , stream );
      }
      hash_iter_free( iter );
    }
    fprintf( stream , "\n\n" );
  }
}
