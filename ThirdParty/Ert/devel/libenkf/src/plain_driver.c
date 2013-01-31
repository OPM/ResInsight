/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plain_driver.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>
#include <ert/util/path_fmt.h>
#include <ert/util/buffer.h>

#include <ert/enkf/fs_driver.h>
#include <ert/enkf/plain_driver.h>
#include <ert/enkf/enkf_node.h>
#include <ert/enkf/fs_types.h>


/**
   The actual path to a stored node can be divided in three parts:


   /some/path/in/the/filesystem/CurrentDirectory/%03d/mem%03d/Analyzed/%s
   |<--------- 1 ------------->|<----- 2 ------>|<-------- 3 ---------->|

   1: This is root path of the enkf_fs filesystem. This is the path
      specified with the ENS_PATH configuration variable in the
      enkf_config system.

   2: The "directory" is a sub indexing under the root path. Typicall
      use of this is to differentiate between the enkf assimalition,
      various forward runs, smoother solutions and so on.

   3. The part with the %d variables in is the final storage
      hierarchy, where the first the replacement (%d,%d,%s) ->
      (report_step , iens, key) is done on run_time.

*/


struct plain_driver_struct {
  FS_DRIVER_FIELDS;
  int                __id;
  path_fmt_type * node_path;
  path_fmt_type * vector_path;
  /* ---------------------------*/
  char          * mount_point;
  char          * node_fmt;
  char          * vector_fmt;
};



static void plain_driver_assert_cast(plain_driver_type * plain_driver) {
  if (plain_driver->__id != PLAIN_DRIVER_ID) 
    util_abort("%s: internal error - cast failed - aborting \n",__func__);
}


static plain_driver_type * plain_driver_safe_cast( void * __driver) {
  plain_driver_type * driver = (plain_driver_type *) __driver;
  plain_driver_assert_cast(driver);
  return driver;
}


static void plain_driver_load_node(void * _driver , const char * node_key, int report_step , int iens ,  buffer_type * buffer) {
  plain_driver_type * driver = plain_driver_safe_cast( _driver );
  {
    char * filename      = path_fmt_alloc_file(driver->node_path , false , report_step , iens , node_key);

    buffer_fread_realloc( buffer , filename );
    free(filename);
  }
}


static void plain_driver_load_vector(void * _driver , const char * node_key, int iens ,  buffer_type * buffer) {
  plain_driver_type * driver = plain_driver_safe_cast( _driver );
  {
    char * filename      = path_fmt_alloc_file(driver->vector_path , false , iens , node_key);

    buffer_fread_realloc( buffer , filename );
    free(filename);
  }
}




static void plain_driver_save_node(void * _driver , const char * node_key , int report_step , int iens ,  buffer_type * buffer) {
  plain_driver_type * driver = (plain_driver_type *) _driver;
  plain_driver_assert_cast(driver);
  {
    char * filename = path_fmt_alloc_file(driver->node_path , true , report_step , iens , node_key);
    buffer_store( buffer , filename );
    free(filename);
  }
}


static void plain_driver_save_vector(void * _driver , const char * node_key , int iens ,  buffer_type * buffer) {
  plain_driver_type * driver = (plain_driver_type *) _driver;
  plain_driver_assert_cast(driver);
  {
    char * filename = path_fmt_alloc_file(driver->vector_path , true , iens , node_key);
    buffer_store( buffer , filename );
    free(filename);
  }
}



void plain_driver_unlink_node(void * _driver , const char * node_key , int report_step , int iens ) {
  plain_driver_type * driver = (plain_driver_type *) _driver;
  plain_driver_assert_cast(driver);
  {
    char * filename = path_fmt_alloc_file(driver->node_path , true , report_step , iens , node_key );
    util_unlink_existing(filename);
    free(filename);
  }
}

void plain_driver_unlink_vector(void * _driver , const char * node_key , int iens ) {
  plain_driver_type * driver = (plain_driver_type *) _driver;
  plain_driver_assert_cast(driver);
  {
    char * filename = path_fmt_alloc_file(driver->vector_path , true , iens , node_key );
    util_unlink_existing(filename);
    free(filename);
  }
}


/**
   Observe that the semantics is fundamentally different between
   plain_driver_paramater_has_node, and plain_driver_load_node:

   * When (trying to) load a node the function will try previous report steps
     all the way back to the first report step.

   * The has_node function will _not_ go back to earlier report steps, but
     instead return false if the report_step we ask for is not present.
*/

bool plain_driver_has_node(void * _driver , const char * node_key , int report_step , int iens ) {
  plain_driver_type * driver = (plain_driver_type *) _driver;
  plain_driver_assert_cast(driver);
  {
    bool has_node;
    char * filename = path_fmt_alloc_file(driver->node_path , true , report_step , iens , node_key);
    if (util_file_exists(filename))
      has_node = true;
    else
      has_node = false;
    free(filename);
    return has_node;
  }
}


bool plain_driver_has_vector(void * _driver , const char * node_key , int iens ) {
  plain_driver_type * driver = (plain_driver_type *) _driver;
  plain_driver_assert_cast(driver);
  {
    bool has_node;
    char * filename = path_fmt_alloc_file(driver->vector_path , true , iens , node_key);
    if (util_file_exists(filename))
      has_node = true;
    else
      has_node = false;
    free(filename);
    return has_node;
  }
}




void plain_driver_free(void *_driver) {
  plain_driver_type * driver = (plain_driver_type *) _driver;
  plain_driver_assert_cast(driver);

  path_fmt_free(driver->node_path);
  path_fmt_free(driver->vector_path);
  
  free( driver->vector_fmt );
  free( driver->node_fmt );
  util_safe_free( driver->mount_point );
  free(driver);
}




/**
  The driver takes a copy of the path object, i.e. it can be deleted
  in the calling scope after calling plain_driver_alloc().

  This is where the various function pointers are initialized.
*/

void * plain_driver_alloc(const char * mount_point , const char * node_fmt, const char * vector_fmt) {
  plain_driver_type * driver = util_malloc(sizeof * driver );
  {
    fs_driver_type * fs_driver = (fs_driver_type *) driver;
    fs_driver_init(fs_driver);
  }
  
  driver->load_node           = plain_driver_load_node;
  driver->save_node           = plain_driver_save_node;
  driver->unlink_node         = plain_driver_unlink_node;
  driver->has_node            = plain_driver_has_node;

  driver->load_vector         = plain_driver_load_vector;
  driver->save_vector         = plain_driver_save_vector;
  driver->unlink_vector       = plain_driver_unlink_vector;
  driver->has_vector          = plain_driver_has_vector;

  driver->fsync_driver        = NULL;
  driver->free_driver         = plain_driver_free;
  driver->mount_point         = util_alloc_string_copy( mount_point );
  driver->node_fmt            = util_alloc_sprintf( "%s%c%s" , mount_point , UTIL_PATH_SEP_CHAR , node_fmt );
  driver->vector_fmt          = util_alloc_sprintf( "%s%c%s" , mount_point , UTIL_PATH_SEP_CHAR , vector_fmt );
  
  driver->node_path           = path_fmt_alloc_directory_fmt( driver->node_fmt );
  driver->vector_path         = path_fmt_alloc_directory_fmt( driver->vector_fmt );
  driver->__id                = PLAIN_DRIVER_ID;
  return driver;
}


void plain_driver_create_fs( FILE * stream , fs_driver_enum driver_type , const char * node_fmt , const char * vector_fmt) {
  util_fwrite_int(driver_type , stream );
  util_fwrite_string(node_fmt , stream);
  util_fwrite_string(vector_fmt , stream);
}


/**
   The two integers from the mount info have already been read at the enkf_fs level.
*/
void * plain_driver_open(FILE * fstab_stream , const char * mount_point) {
  char * node_fmt = util_fread_alloc_string( fstab_stream );
  char * vector_fmt = util_fread_alloc_string( fstab_stream );
  plain_driver_type * driver = plain_driver_alloc( mount_point , node_fmt , vector_fmt );
  free(node_fmt);
  free(vector_fmt);
  return driver;
}


