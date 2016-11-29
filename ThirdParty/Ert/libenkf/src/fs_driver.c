/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'fs_driver.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>

#include <ert/enkf/fs_types.h>
#include <ert/enkf/fs_driver.h>

/* 
   The underlying base types (abstract - with no accompanying
   implementation); these two type ID's are not exported outside this
   file. They are not stored to disk, and only used in an attempt
   yo verify run-time casts.
*/
#define FS_DRIVER_ID           10


/*****************************************************************/
/* This fs driver implemenatition is common to both dynamic and
   parameter info. */

void fs_driver_init(fs_driver_type * driver) {
  driver->type_id = FS_DRIVER_ID;
  
  driver->load_node   = NULL;
  driver->save_node   = NULL;
  driver->has_node    = NULL;
  driver->unlink_node = NULL;

  driver->load_vector   = NULL;
  driver->save_vector   = NULL;
  driver->has_vector    = NULL;
  driver->unlink_vector = NULL;
  
  driver->free_driver   = NULL;
  driver->fsync_driver  = NULL;
}

void fs_driver_assert_cast(const fs_driver_type * driver) {
  if (driver->type_id != FS_DRIVER_ID) 
    util_abort("%s: internal error - incorrect cast() - aborting \n" , __func__);
}


fs_driver_type * fs_driver_safe_cast(void * __driver) {
  fs_driver_type * driver = (fs_driver_type *) __driver;
  if (driver->type_id != FS_DRIVER_ID)
    util_abort("%s: runtime cast failed. \n",__func__);
  return driver;
}

/*****************************************************************/


void fs_driver_init_fstab( FILE * stream, fs_driver_impl driver_id) {
  util_fwrite_long( FS_MAGIC_ID , stream );
  util_fwrite_int ( CURRENT_FS_VERSION , stream );
  util_fwrite_int ( driver_id , stream );
}



/**
   Will open fstab stream and return it. The semantics with respect to
   existing/not existnig fstab file depends on the value of the
   @create parameter:

   @create = True: If the fstab file exists the function will return
   NULL, otherwise it will return a stream opened for writing to the
   fstab file.

   @create = False: If the fstab file exists the the function will
   return a stream opened for reading of the fstab file, otherwise
   it will return NULL.

*/

char * fs_driver_alloc_fstab_file( const char * path ) {
  return util_alloc_filename( path , "ert_fstab" , NULL);
}


FILE * fs_driver_open_fstab( const char * path , bool create) {
  FILE * stream = NULL;
  char * fstab_file = fs_driver_alloc_fstab_file( path );
  if (create)
    util_make_path( path );

  if (util_file_exists( fstab_file ) != create) {
    if (create)
      stream = util_fopen( fstab_file , "w");
    else
      stream = util_fopen( fstab_file , "r");
  }
  free( fstab_file );
  return stream;
}


void fs_driver_assert_magic( FILE * stream ) {
  long fs_magic = util_fread_long( stream );
  if (fs_magic != FS_MAGIC_ID)
    util_abort("%s: WTF - fstab magic marker incorrect \n",__func__);
}



void fs_driver_assert_version( FILE * stream , const char * mount_point) {
  int file_version = util_fread_int( stream );

  if (file_version < MIN_SUPPORTED_FS_VERSION )
    util_exit("%s: The file system you are trying to access is created with a very old version of ert - sorry.\n",__func__);

  if (file_version > CURRENT_FS_VERSION)
    util_exit("%s: The file system you are trying to access has been created with a newer version of ert - sorry.\n",__func__);

  if (file_version < CURRENT_FS_VERSION) {
    if ((file_version == 105) && (CURRENT_FS_VERSION == 106))
      fprintf(stderr,"%s: The file system you are accessing has been written with an older version of ert - STATIC information ignored. \n",__func__);
    else
      util_exit("%s: The file system you are trying to access has been created with an old version of ert - sorry.\n",__func__);
  }

  
}


fs_driver_impl fs_driver_fread_type( FILE * stream ) {
  fs_driver_impl impl = util_fread_int( stream );
  return impl;
}


int fs_driver_fread_version( FILE * stream ) {
  long fs_magic = util_fread_long( stream );
  if (fs_magic != FS_MAGIC_ID)
    return -1;
  else {
    int file_version = util_fread_int( stream );
    return file_version;
  }
}


/*****************************************************************/


