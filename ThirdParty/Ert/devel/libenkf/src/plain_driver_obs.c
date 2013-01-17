/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plain_driver_obs.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <obs_node.h>
#include <basic_driver.h>
#include <plain_driver_obs.h>
#include <fs_types.h>
#include <path_fmt.h>
#include <util.h>



struct plain_driver_obs_struct {
  BASIC_OBS_DRIVER_FIELDS;
  int             __id;
  path_fmt_type * path;   /* With one embedded %d format character which is replaced with report_step on runtime. */
};







static plain_driver_obs_type * plain_driver_obs_safe_cast(void * _driver) {
  plain_driver_obs_type * driver = (plain_driver_obs_type *) _driver;

  if (driver->__id != PLAIN_DRIVER_OBS_ID) 
    util_abort("%s: internal error - cast failed - aborting \n",__func__);
  
  return driver;
}




static char * plain_driver_obs_alloc_filename(const plain_driver_obs_type * driver , int report_step , const char * key, bool auto_mkdir) {
  return path_fmt_alloc_file(driver->path , auto_mkdir , report_step , key);
}



void plain_driver_obs_load_node(void * _driver , int report_step , obs_node_type * node) {
  plain_driver_obs_type * driver = plain_driver_obs_safe_cast(_driver);
  {
    char * filename = plain_driver_obs_alloc_filename(driver , report_step , obs_node_get_key(node) , false);
    FILE * stream = util_fopen(filename , "r");

    obs_node_fread(node , stream , report_step);

    fclose(stream);
    free(filename);
  }
}


void plain_driver_obs_unlink_node(void * _driver , int report_step , obs_node_type * node) {
  plain_driver_obs_type * driver = plain_driver_obs_safe_cast(_driver);
  {
    char * filename = plain_driver_obs_alloc_filename(driver , report_step  , obs_node_get_key(node) , false);
    util_unlink_existing(filename);
    free(filename);
  }
}


void plain_driver_obs_save_node(void * _driver , int report_step , obs_node_type * node) {
  plain_driver_obs_type * driver = plain_driver_obs_safe_cast(_driver);
  {
    char * filename = plain_driver_obs_alloc_filename(driver , report_step , obs_node_get_key(node) , true);
    FILE * stream = util_fopen(filename , "w");
    bool   data_written = obs_node_fwrite(node , stream , report_step);
    fclose(stream);
    if (!data_written)
      util_unlink_existing( filename ); /* remove empty files. */
    free(filename);
  }
}


/**
   Return true if we have a on-disk representation of the node.
*/

bool plain_driver_obs_has_node(void * _driver , int report_step , const char * key) {
  plain_driver_obs_type * driver = plain_driver_obs_safe_cast(_driver);
  {
    bool has_node;
    char * filename = plain_driver_obs_alloc_filename(driver , report_step , key , false);
    if (util_file_exists(filename))
      has_node = true;
    else
      has_node = false;
    free(filename);
    return has_node;
  }
}




void plain_driver_obs_free(void *_driver) {
  plain_driver_obs_type * driver = plain_driver_obs_safe_cast(_driver);
  path_fmt_free(driver->path);
  free(driver);
}



void plain_driver_obs_README(const char * root_path) {
  char * README_file = util_alloc_full_path(root_path , "README.txt");
  util_make_path(root_path);
  {
    FILE * stream      = util_fopen(README_file , "w");
    fprintf(stream,"This is the root directory of the EnKF ensemble filesystem. All files contain one enkf_node \n");
    fprintf(stream,"instance. The files are binary, and compressed with zlib (util_fwrite_compressed).\n");
    fclose(stream);
  }
  free(README_file);
}


/*
  The driver takes a copy of the path object, i.e. it can be deleted
  in the calling scope after calling plain_driver_obs_alloc().
*/
void * plain_driver_obs_alloc(const char * root_path , const char * obs_path ) {
  plain_driver_obs_type * driver = util_malloc(sizeof * driver );
  driver->load        = plain_driver_obs_load_node;
  driver->save        = plain_driver_obs_save_node;
  driver->has_node    = plain_driver_obs_has_node;
  driver->free_driver = plain_driver_obs_free;
  driver->unlink_node = plain_driver_obs_unlink_node;
  {
    char *path;

    if (root_path != NULL) 
      path = util_alloc_full_path(root_path , obs_path);
    else
      path = util_alloc_string_copy(obs_path);
    
    driver->path = path_fmt_alloc_directory_fmt( path );
    free(path);
  }
  driver->__id = PLAIN_DRIVER_OBS_ID;
  {
    basic_obs_driver_type * basic_driver = (basic_obs_driver_type *) driver;
    basic_obs_driver_init(basic_driver);
    return basic_driver;
  }
}


void plain_driver_obs_fwrite_mount_info(FILE * stream , const char * obs_fmt ) {
  util_fwrite_int(OBS_DRIVER , stream);
  util_fwrite_int(PLAIN_DRIVER_OBS_ID , stream);
  util_fwrite_string(obs_fmt , stream);

}

/**
   The two integers from the mount info have already been read at the enkf_fs level.
*/
plain_driver_obs_type * plain_driver_obs_fread_alloc(const char * root_path , FILE * stream) {
  char * obs_fmt = util_fread_alloc_string( stream );
  plain_driver_obs_type * driver = plain_driver_obs_alloc(root_path , obs_fmt);
  free(obs_fmt);
  return driver;
}

