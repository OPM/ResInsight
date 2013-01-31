/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'fs_driver.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __FS_DRIVER_H__
#define __FS_DRIVER_H__ 
#ifdef __cplusplus
extern "C" {
#endif
#include <ert/util/buffer.h>
#include <ert/util/stringlist.h>

#include <ert/enkf/enkf_node.h>
#include <ert/enkf/fs_types.h>

#define FS_MAGIC_ID          123998L
#define FSTAB_FILE          "ert_fstab"
#define CURRENT_FS_VERSION   105

/**
   Version history:

   0  : 

   
   File system version            | First svn version     |  Last svn version
   --------------------------------------------------------------------------
   100                            |                       |  1799
   101                            |   1810                |  1886
   102                            |   1887/1902           |  1996 
   103                            |   1997                |
                                  |   2047: sqlite added  |  2125
   104                            |   2127                |
                                  |   2140: block_fs_index added
                                  |   2190: started to distribute ert binary internally
   105                            |   3918                |
   --------------------------------------------------------------------------


   Version: 100
   ------------
   First version with subdirectories for different cases.


   
   Version: 101
   ------------
   Have changed the format for storing static keywords. Instead of a
   directory with numbered files in them, the keywords get an integer
   appended:

     INTEHEAD/0   ==>  INTEHEAD_0

   The actual changing of keyword is done in the function
   __realloc_static_kw() in enkf_state.c.



   Version: 102 
   ------------ 
   Have removed the type spesific plain drivers, now it is only
   plain_driver and plain_driver index. The special functionality for
   parameters/static/dynamic is now implemented at the enkf_fs level.
   The upgrade from 101 only involves the mount info and the
   implementation, the actual stored files are not touched by this
   upgrade.



   Version: 103
   ------------
   Have changed the reading/writing to go through the buffer
   type. This should simplify introducing other drivers than the
   simple plain file based driver.  

   The upgrade to version is 103 is quite extensive - all node types
   have specific _103_ functions. The xxx_fread() and xxx_fwrite()
   functions were removed in svn:2046.

   A very experimental version of the sqlite driver was added in
   svn:2047. When (if ??) this stabilizes it should probably lead to
   an upgrade to version 104.

   At the very last checkin of this version the fs prototypes also
   changed to use (const enkf_config_node_type *) instances instead of
   (const char * ) for the main key.


   
   Version: 104 
   ------------ 
   In this version the treatment of read_xx and write_xx drivers has
   changed. There are no longer separate read_xx and write_xx drivers,
   instead the drivers have internal state which differentiates
   between read and write.

   The block_fs driver is added - and reasonably stabilized in this
   version.


   Observe that all the upgrade functions were removed at svn:3305.


   Version 104B
   ------------
   Vector storage; each case is a seperate enkf_fs instance. Current
   is stored with a symlink. Read about 104 -> 104B fuckup in the
   function upgrade104B() in fs_driver.c


   Version: 105
   ------------
   Using time_map to store time information; common to all members in
   ensemble.
   
*/




  typedef struct fs_driver_struct         fs_driver_type;
  
  typedef void (save_kwlist_ftype)  (void * , int , int , buffer_type * buffer);  /* Functions used to load/store restart_kw_list instances. */
  typedef void (load_kwlist_ftype)  (void * , int , int , buffer_type * buffer);          
  
  typedef void (load_node_ftype)    (void * driver, const char * , int , int , buffer_type * );
  typedef void (save_node_ftype)    (void * driver, const char * , int , int , buffer_type * );
  typedef void (unlink_node_ftype)  (void * driver, const char * , int , int );
  typedef bool (has_node_ftype)     (void * driver, const char * , int , int );
  
  typedef void (load_vector_ftype)    (void * driver, const char * , int , buffer_type * );
  typedef void (save_vector_ftype)    (void * driver, const char * , int , buffer_type * );
  typedef void (unlink_vector_ftype)  (void * driver, const char * , int );
  typedef bool (has_vector_ftype)     (void * driver, const char * , int );
  
  typedef void (fsync_driver_ftype) (void * driver);
  typedef void (free_driver_ftype)  (void * driver);


/**
   The fs_driver_type contains a number of function pointers
   and a type_id used for run-time cast checking.
   
   The fs_driver_type is never actually used, but the point is that
   all drivers must implement the fs driver "interface". In practice
   this is done by including the macro FS_DRIVER_FIELDS *at the start*
   of the definition of another driver, i.e. the simplest actually
   working driver, the plain_driver is implemented like this:

   struct plain_driver_struct {
      FS_DRIVER_FIELDS
      int plain_driver_id;
      path_fmt_type * path;
   }


*/

   

#define FS_DRIVER_FIELDS                   \
load_node_ftype           * load_node;     \
save_node_ftype           * save_node;     \
has_node_ftype            * has_node;      \
unlink_node_ftype         * unlink_node;   \
load_vector_ftype         * load_vector;   \
save_vector_ftype         * save_vector;   \
has_vector_ftype          * has_vector;    \
unlink_vector_ftype       * unlink_vector; \
free_driver_ftype         * free_driver;   \
fsync_driver_ftype        * fsync_driver;  \
int                         type_id




struct fs_driver_struct {
  FS_DRIVER_FIELDS;
};



  /*****************************************************************/

  void                       fs_driver_init(fs_driver_type * );
  void                       fs_driver_assert_cast(const fs_driver_type * );
  fs_driver_type           * fs_driver_safe_cast(void * );
  
  void                       fs_driver_init_fstab( FILE * stream, fs_driver_impl driver_id );
  FILE                     * fs_driver_open_fstab( const char * path , bool create);
  fs_driver_impl             fs_driver_fread_type( FILE * stream ); 
  void                       fs_driver_assert_magic( FILE * stream );
  void                       fs_driver_assert_version( FILE * stream , const char * mount_point);


#ifdef __cplusplus
}
#endif
#endif
