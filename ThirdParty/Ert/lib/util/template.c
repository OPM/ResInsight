/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'template.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/ert_api_config.h>

#include <ert/util/util.h>
#include <ert/util/subst_list.h>
#include <ert/util/subst_func.h>
#include <ert/util/template.h>
#include <ert/util/template_type.h>
#include <ert/util/stringlist.h>

/**
   Iff the template is set up with internaliz_template == false the
   template content is loaded at instantiation time, and in that case
   the name of the template file can contain substitution characters -
   i.e. in this case different instance can use different source
   templates.

   To avoid race issues this function does not set actually update the
   state of the template object.
*/

static char * template_load( const template_type * template , const subst_list_type * ext_arg_list) {
  int buffer_size;
  char * template_file = util_alloc_string_copy( template->template_file );
  char * template_buffer;
  
  subst_list_update_string( template->arg_list , &template_file);
  if (ext_arg_list != NULL)
    subst_list_update_string( ext_arg_list , &template_file);
  
  template_buffer = util_fread_alloc_file_content( template_file , &buffer_size );
  free( template_file );
  
  return template_buffer;
}



void template_set_template_file( template_type * template , const char * template_file) {
  template->template_file = util_realloc_string_copy( template->template_file , template_file );
  if (template->internalize_template) {
    util_safe_free( template->template_buffer );
    template->template_buffer = template_load( template , NULL );  
  }
}

/** This will not instantiate */
const char * template_get_template_file( const template_type * template ) {
  return template->template_file; 
}



/**
   This function allocates a template object based on the source file
   'template_file'. If @internalize_template is true the template
   content will be read and internalized at boot time, otherwise that
   is deferred to template instantiation time (in which case the
   template file can change dynamically).
*/


template_type * template_alloc( const char * template_file , bool internalize_template , subst_list_type * parent_subst) {
  template_type * template = util_malloc( sizeof * template );
  UTIL_TYPE_ID_INIT(template , TEMPLATE_TYPE_ID);
  template->arg_list             = subst_list_alloc( parent_subst );
  template->template_buffer      = NULL;
  template->template_file        = NULL;
  template->internalize_template = internalize_template;
  template->arg_string           = NULL;
  template_set_template_file( template , template_file );

#ifdef ERT_HAVE_REGEXP
  template_init_loop_regexp( template );
#endif
  return template;
}







void template_free( template_type * template ) {
  subst_list_free( template->arg_list );
  util_safe_free( template->template_file );
  util_safe_free( template->template_buffer );
  util_safe_free( template->arg_string );

#ifdef ERT_HAVE_REGEXP
  regfree( &template->start_regexp );
  regfree( &template->end_regexp );
#endif
  
  free( template );
}



/**
   This function will create the file @__target_file based on the
   template instance. Before the target file is written all the
   internal substitutions and then subsequently the subsititutions in
   @arg_list will be performed. The input @arg_list can be NULL - in
   which case this is more like copy operation.

   Observe that:
   
    1. Substitutions will be performed on @__target_file

    2. @__target_file can contain path components.

    3. If internalize_template == false subsititions will be performed
       on the filename of the file with template content.
  
    4. If the parameter @override_symlink is true the function will
       have the following behaviour:

         If the target_file already exists as a symbolic link, the
         symbolic link will be removed prior to creating the instance,
         ensuring that a remote file is not updated.

*/
   


void template_instantiate( const template_type * template , const char * __target_file , const subst_list_type * arg_list , bool override_symlink) {
  char * target_file = util_alloc_string_copy( __target_file );

  /* Finding the name of the target file. */
  subst_list_update_string( template->arg_list , &target_file);
  if (arg_list != NULL) subst_list_update_string( arg_list , &target_file );

  {
    char * char_buffer;
    /* Loading the template - possibly expanding keys in the filename */
    if (template->internalize_template)
      char_buffer = util_alloc_string_copy( template->template_buffer);
    else
      char_buffer = template_load( template , arg_list );
    
    /* Substitutions on the content. */
    subst_list_update_string( template->arg_list , &char_buffer );
    if (arg_list != NULL) subst_list_update_string( arg_list , &char_buffer );

    
#ifdef ERT_HAVE_REGEXP
    {
      buffer_type * buffer = buffer_alloc_private_wrapper( char_buffer , strlen( char_buffer ) + 1);
      template_eval_loops( template , buffer );
      char_buffer = buffer_get_data( buffer );
      buffer_free_container( buffer );
    }
#endif

    /* 
       Check if target file already exists as a symlink, 
       and remove it if override_symlink is true. 
    */
    if (override_symlink) {
      if (util_is_link( target_file ))
        remove( target_file );
    }
    
    /* Write the content out. */
    {
      FILE * stream = util_mkdir_fopen( target_file , "w");
      fprintf(stream , "%s" , char_buffer);
      fclose( stream );
    }
    free( char_buffer );
  }
  
  free( target_file );
}


/**
   Add an internal key_value pair. This substitution will be performed
   before the internal substitutions.
*/
void template_add_arg( template_type * template , const char * key , const char * value ) {
  subst_list_append_copy( template->arg_list , key , value , NULL /* No doc_string */);
}


void template_clear_args( template_type * template ) {
  subst_list_clear( template->arg_list );
}


int template_add_args_from_string( template_type * template , const char * arg_string) {
  return subst_list_add_from_string( template->arg_list , arg_string , true);
}


char * template_get_args_as_string( template_type * template ) {
  util_safe_free( template->arg_string );
  template->arg_string = subst_list_alloc_string_representation( template->arg_list );
  return template->arg_string;
}

/*****************************************************************/


