/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'fs_types.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __FS_TYPES_H__
#define __FS_TYPES_H__



/*
  The various driver implementations - this goes on disk all over the
  place, and the numbers should be considered SET IN STONE. When a new
  driver is added the switch statement in the enkf_fs_mount() function
  must be updated.
*/

// Version from ~ svn:3720
//typedef enum {
//  INVALID_DRIVER_ID          = 0,
//  PLAIN_DRIVER_INDEX_ID      = 1001,
//  PLAIN_DRIVER_STATIC_ID     = 1002,  /* Depreceated */
//  PLAIN_DRIVER_DYNAMIC_ID    = 1003,  /* Depreceated */
//  PLAIN_DRIVER_PARAMETER_ID  = 1004,  /* Depreceated */
//  PLAIN_DRIVER_ID            = 1005,
//  BLOCK_FS_DRIVER_ID         = 3001,
//  BLOCK_FS_DRIVER_INDEX_ID   = 3002 } fs_driver_impl;


typedef enum {
  INVALID_DRIVER_ID          = 0,
  PLAIN_DRIVER_ID            = 1005,
  BLOCK_FS_DRIVER_ID         = 3001} fs_driver_impl;
  





/*
  The categories of drivers. To reduce the risk of programming
  error (or at least to detect it ...), there should be no overlap
  between these ID's and the ID's of the actual implementations
  above. The same comment about permanent storage applies to these
  numbers as well.
*/

typedef enum {
  DRIVER_PARAMETER        = 1,
  DRIVER_STATIC           = 2,
  DRIVER_INDEX            = 4,  // DRIVER_DYNAMIC = 3; removed at svn ~ 3720.
  DRIVER_DYNAMIC_FORECAST = 5,
  DRIVER_DYNAMIC_ANALYZED = 6} fs_driver_enum;



fs_driver_impl    fs_types_lookup_string_name(const char * driver_name);
const char      * fs_types_get_driver_name(fs_driver_enum driver_type);

#endif
